// SPDX-License-Identifier: GPL-2.0-or-later
#include "DigimonDatabase.h"
#include "DigimonPolicy.h"
#include "Chat.h"
#include "Config.h"
#include "Creature.h"
#include "CryptoRandom.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "Formulas.h"
#include "GameTime.h"
#include "Item.h"
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QuestDef.h"
#include "Random.h"
#include "RBAC.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"
#include "ThreatManager.h"
#include "Transaction.h"
#include "Util.h"
#include "World.h"
#include "WorldPacket.h"
#include <memory>
#include <charconv>
#include <cctype>
#include <map>
#include <mutex>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Digimon
{
constexpr uint32 DeviceMenu = 900100;
enum Sender : uint32
{
    Main = 1,
    ChooseStarter = 2,
    Buy = 3,
    Equip = 4,
    Unequip = 5,
    Use = 6,
    Activate = 7,
    CollectionPage = 8
};
enum Action : uint32
{
    Invoke = 1,
    Withdraw,
    Attack,
    Follow,
    Recover,
    Collection,
    Techniques,
    Capture,
    Back,
    Fight
};

struct Settings
{
    bool Enabled = true;
    uint32 Welcome = 900100;
    uint32 Trainer = 900101;
    uint32 Device = 900110;
    uint64 InitialBits = 100;
    double TamerXP = 0.70;
    double CompanionXP = 1.0;
    uint32 BitsPerKill = 5;
    uint32 CaptureChance = 35;
    uint32 CaptureHealth = 25;
    uint32 CaptureCooldown = 10;
    bool Experiments = false;
} Config;

struct Species
{
    uint32 Entry;
    std::string Name;
    bool Starter;
    uint32 CaptureEntry;
    Stage DefaultStage;
    bool Enabled;
};

struct Companion
{
    uint32 Id;
    uint32 SpeciesId;
    Stage Evolution;
    uint8 Level;
    uint64 Experience;
    float Health;
    std::unordered_set<uint32> Learned;
    std::map<uint8, uint32> Equipped;
};

struct Technique
{
    std::string Name;
    uint32 Spell;
    Stage Required;
    uint64 Price;
    bool Enabled;
    bool Special;
};

struct Tamer
{
    uint64 Bits = 0;
    uint32 Capacity = 30;
    uint32 Active = 0;
    bool Summoned = false;
    bool Addon = false;
    uint64 CaptureReady = 0;
    uint32 NextOption = 1;
    ObjectGuid Actor;
    std::vector<Companion> Companions;
    std::unordered_set<uint32> SentTechniqueNames;
};

std::recursive_mutex Mutex;
std::unique_ptr<MySQLConnectionInfo> DatabaseInfo;
std::unique_ptr<Connection> Database;
std::unordered_map<uint32, Species> Catalog;
std::unordered_map<uint32, Technique> TechniqueCatalog;
std::unordered_map<std::string, uint32> Aliases;
std::unordered_map<uint32, std::unordered_set<uint32>> SpeciesTechniques;
struct Evolution
{
    uint32 Target;
    uint8 Level;
};
std::unordered_map<uint32, Evolution> Evolutions;
std::unordered_map<uint32, std::vector<ObjectGuid>> PreviewActors;
constexpr uint32 PreviewEntry = 900121;
std::unordered_map<uint32, Tamer> Tamers;
struct WildState
{
    std::string Token;
    std::unordered_map<uint32, ObjectGuid> WeakenedBy;
    bool Claimed = false;
    uint64 ReadyAt = 0;
};
std::unordered_map<ObjectGuid, WildState> Wild;
std::unordered_set<uint32> ProtectedQuestEntries;
struct Unlock
{
    uint8 Prepared;
    uint8 Simultaneous;
    uint8 PotionTier;
};
std::map<uint8, Unlock> Unlocks;
std::unordered_map<uint32, std::vector<ObjectGuid>> LabActors;

uint32 OwnerId(Player const* player)
{
    return player->GetGUID().GetCounter();
}

std::string NewToken()
{
    return ByteArrayToHexStr(Acore::Crypto::GetRandomBytes<16>());
}

template <class... Args> PreparedStatement<Connection>* Make(Statement index, Args const&... args)
{
    auto* statement = Database->MakeStatement(index);
    uint8 parameter = 0;
    (statement->SetData(parameter++, args), ...);
    return statement;
}

template <class... Args> PreparedQueryResult Query(Statement index, Args const&... args)
{
    std::unique_ptr<PreparedStatement<Connection>> statement(Make(index, args...));
    PreparedQueryResult result(Database->Query(statement.get()));
    // Direct synchronous connections return a valid result object for zero rows.
    // Match DatabaseWorkerPool::Query semantics before callers use Fetch().
    return result && result->GetRowCount() ? result : PreparedQueryResult(nullptr);
}

template <class... Args> bool Execute(Statement index, Args const&... args)
{
    std::unique_ptr<PreparedStatement<Connection>> statement(Make(index, args...));
    return Database->Execute(statement.get());
}

void SendUI(Player* player, std::string const& message);

bool TechniqueAllowed(uint32 species, uint32 technique)
{
    auto const found = SpeciesTechniques.find(species);
    return found != SpeciesTechniques.end() && found->second.count(technique);
}

std::string CatalogKey(std::string_view text)
{
    std::string result;
    for (unsigned char value : text)
        if (!std::isspace(value))
            result += static_cast<char>(std::tolower(value));
    return result;
}

uint32 ResolveSpecies(std::string_view text)
{
    uint32 id = 0;
    auto const parsed = std::from_chars(text.data(), text.data() + text.size(), id);
    if (parsed.ec == std::errc() && parsed.ptr == text.data() + text.size() && Catalog.count(id))
        return id;
    std::string const key = CatalogKey(text);
    if (auto alias = Aliases.find(key); alias != Aliases.end())
        return alias->second;
    for (auto const& [speciesId, species] : Catalog)
        if (CatalogKey(species.Name) == key)
            return speciesId;
    return 0;
}

std::pair<uint32, Stage> ResolveEvolution(Companion const& companion, uint8 level)
{
    uint32 species = companion.SpeciesId;
    Stage stage = companion.Evolution;
    for (uint8 step = 0; step < 4; ++step)
    {
        auto const rule = Evolutions.find(species);
        if (rule == Evolutions.end())
            break;
        auto const target = Catalog.find(rule->second.Target);
        if (target == Catalog.end() || !target->second.Enabled ||
            !EvolutionReady(level, rule->second.Level, stage, target->second.DefaultStage))
            break;
        species = target->first;
        stage = target->second.DefaultStage;
    }
    return {species, stage};
}

void Notify(Player* player, std::string const& message)
{
    ChatHandler(player->GetSession()).SendSysMessage(message);
    if (auto state = Tamers.find(OwnerId(player)); state != Tamers.end() && state->second.Addon)
        SendUI(player, "NOTICE|" + message);
}

void AddOption(Player* player, uint8 icon, std::string const& text, uint32 sender, uint32 action)
{
    // Never recycle an option ID within a session, including after a DB reload.
    // A delayed/replayed click cannot select a different action in a new menu.
    Tamer& state = Tamers.at(OwnerId(player));
    if (state.NextOption >= uint32(std::numeric_limits<int32>::max()))
        return;
    uint32 const key = state.NextOption++;
    player->PlayerTalkClass->GetGossipMenu().AddMenuItem(static_cast<int32>(key), icon, text, sender, action, "", 0);
}

bool Reload(Player* player)
{
    uint32 const owner = OwnerId(player);
    if (!Execute(InsertTamer, owner, Config.InitialBits))
        return false;
    auto profile = Query(SelectTamer, owner);
    if (!profile)
        return false;
    ObjectGuid const actor = Tamers[owner].Actor;
    uint32 const nextOption = Tamers[owner].NextOption;
    bool const addon = Tamers[owner].Addon;
    Tamer state;
    state.Actor = actor;
    state.Addon = addon;
    state.NextOption = nextOption;
    Field* fields = profile->Fetch();
    state.Bits = fields[0].Get<uint64>();
    state.Capacity = fields[1].Get<uint16>();
    state.Summoned = fields[2].Get<uint8>() != 0;
    state.CaptureReady = fields[3].Get<uint64>();
    if (auto active = Query(SelectActive, owner))
        state.Active = active->Fetch()[0].Get<uint32>();
    if (auto companions = Query(SelectCollection, owner))
        do
        {
            fields = companions->Fetch();
            state.Companions.push_back({fields[0].Get<uint32>(),
                                        fields[1].Get<uint32>(),
                                        static_cast<Stage>(fields[2].Get<uint8>()),
                                        fields[3].Get<uint8>(),
                                        fields[4].Get<uint64>(),
                                        fields[5].Get<float>(),
                                        {},
                                        {}});
        } while (companions->NextRow());
    if (auto learned = Query(SelectLearned, owner))
        do
        {
            fields = learned->Fetch();
            for (auto& companion : state.Companions)
                if (companion.Id == fields[0].Get<uint32>())
                    companion.Learned.insert(fields[1].Get<uint32>());
        } while (learned->NextRow());
    if (auto equipped = Query(SelectEquipped, owner))
        do
        {
            fields = equipped->Fetch();
            for (auto& companion : state.Companions)
                if (companion.Id == fields[0].Get<uint32>())
                    companion.Equipped.emplace(fields[1].Get<uint8>(), fields[2].Get<uint32>());
        } while (equipped->NextRow());
    Tamers[owner] = std::move(state);
    return true;
}

Companion* Active(Tamer& state)
{
    for (auto& companion : state.Companions)
        if (companion.Id == state.Active)
            return &companion;
    return nullptr;
}

Creature* Actor(Player* player, Tamer& state)
{
    return state.Actor.IsEmpty() ? nullptr : player->GetMap()->GetCreature(state.Actor);
}

void SendUI(Player* player, std::string const& message)
{
    WorldPacket packet;
    // Addon transport must keep the normal message opcode, even for GM test accounts.
    ChatHandler::BuildChatPacket(packet, CHAT_MSG_WHISPER, LANG_ADDON, player->GetGUID(), player->GetGUID(),
                                 "WDMMO\tV1|" + message, 0);
    player->GetSession()->SendPacket(&packet);
}

void SendCombatState(Player* player, Tamer& state)
{
    if (!state.Addon)
        return;
    Companion* companion = Active(state);
    Creature* actor = Actor(player, state);
    std::string message = "S|" + std::to_string(state.Active) + "|" +
                          std::to_string(companion ? companion->Level : 0) + "|" +
                          std::to_string(actor ? actor->GetHealth() : 0) + "|" +
                          std::to_string(actor ? actor->GetMaxHealth() : 0) + "|" +
                          std::to_string(actor ? actor->GetPower(POWER_FOCUS) : 0) + "|" +
                          std::to_string(actor ? actor->GetMaxPower(POWER_FOCUS) : 0);
    for (uint8 slot = 0; slot < 5; ++slot)
    {
        uint32 spellId = 0;
        uint32 cooldown = 0;
        int32 cost = 0;
        if (companion)
            if (auto equipped = companion->Equipped.find(slot); equipped != companion->Equipped.end())
                if (auto technique = TechniqueCatalog.find(equipped->second);
                    technique != TechniqueCatalog.end() && technique->second.Enabled &&
                    companion->Learned.count(equipped->second) &&
                    TechniqueAllowed(companion->SpeciesId, equipped->second) &&
                    companion->Evolution >= technique->second.Required &&
                    CanEquip(companion->Evolution, slot, technique->second.Special))
                {
                    spellId = technique->second.Spell;
                    cooldown = actor ? actor->GetSpellCooldown(spellId) : 0;
                    if (SpellInfo const* spell = sSpellMgr->GetSpellInfo(spellId))
                        cost = actor ? spell->CalcPowerCost(actor, spell->GetSchoolMask()) : spell->ManaCost;
                    if (state.SentTechniqueNames.insert(equipped->second).second)
                        SendUI(player, "T|" + std::to_string(equipped->second) + "|" + technique->second.Name);
                }
        message += "|" + std::to_string(spellId) + "," + std::to_string(cost) + "," + std::to_string(cooldown);
        message += "," + std::to_string(companion && companion->Equipped.count(slot) ?
                                            companion->Equipped.at(slot) : 0);
    }
    SendUI(player, message);
}

bool Managed(Unit const* unit)
{
    if (!unit || !unit->IsCreature())
        return false;
    auto const owner = unit->GetOwnerGUID();
    if (!owner.IsPlayer())
        return false;
    auto const found = Tamers.find(owner.GetCounter());
    return found != Tamers.end() && found->second.Actor == unit->GetGUID();
}

void PersistHealth(Player* player, Tamer& state)
{
    if (Creature* actor = Actor(player, state))
        if (Companion* companion = Active(state))
        {
            float const health = actor->IsAlive() ? actor->GetHealthPct() : 0.0f;
            if (Execute(SaveHealth, health, OwnerId(player), companion->Id))
                companion->Health = health;
        }
}

bool Busy(Player* player, Tamer& state)
{
    Creature* actor = Actor(player, state);
    return player->IsInCombat() || (actor && actor->IsInCombat());
}

void Despawn(Player* player, Tamer& state)
{
    PersistHealth(player, state);
    if (Creature* actor = Actor(player, state))
        actor->DespawnOrUnsummon();
    state.Actor.Clear();
}

void ApplyStats(Creature* actor, Companion const& companion)
{
    actor->SetLevel(companion.Level);
    uint32 const health = 60u + 25u * companion.Level;
    actor->SetCreateHealth(health);
    actor->SetMaxHealth(health);
    actor->SetHealth(std::max(1u, HealthFromPercent(health, companion.Health)));
    actor->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 2.0f + companion.Level * 1.5f);
    actor->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 4.0f + companion.Level * 2.0f);
    actor->UpdateDamagePhysical(BASE_ATTACK);
    actor->SetAttackTime(BASE_ATTACK, 1500);
    actor->setPowerType(POWER_FOCUS);
    actor->SetMaxPower(POWER_FOCUS, 100);
    actor->SetPower(POWER_FOCUS, 100);
}

bool ApplyForm(Player* player, Creature* actor, Companion const& companion)
{
    Species const& species = Catalog.at(companion.SpeciesId);
    uint32 const focus = actor->GetPower(POWER_FOCUS);
    if (actor->GetEntry() != species.Entry && !actor->UpdateEntry(species.Entry, nullptr, false, false))
        return false;
    actor->SetOwnerGUID(player->GetGUID());
    actor->SetCreatorGUID(player->GetGUID());
    actor->SetUnitFlag(UNIT_FLAG_PLAYER_CONTROLLED);
    actor->SetFaction(player->GetFaction());
    actor->SetName(species.Name);
    actor->SetReactState(REACT_PASSIVE);
    actor->SetNoCallAssistance(true);
    ApplyStats(actor, companion);
    actor->SetPower(POWER_FOCUS, focus);
    return true;
}

void RewardCompanion(Creature* actor, Unit* victim)
{
    std::lock_guard<std::recursive_mutex> guard(Mutex);
    if (!Config.Enabled || !Managed(actor) || !victim || !victim->IsCreature())
        return;
    Creature* defeated = victim->ToCreature();
    Player* player = ObjectAccessor::FindPlayer(actor->GetOwnerGUID());
    if (!player || player->IsGameMaster() || defeated->IsSummon() || defeated->GetOwnerGUID() ||
        defeated->IsCritter() || defeated->GetCreatureTemplate()->HasFlagsExtra(CREATURE_FLAG_EXTRA_NO_XP) ||
        defeated->GetCreatureTemplate()->ModExperience <= 0 || defeated->GetLootRecipient() != player)
        return;
    Tamer& state = Tamers.at(OwnerId(player));
    Companion* companion = Active(state);
    if (!companion || defeated->GetLevel() <= Acore::XP::GetGrayLevel(companion->Level))
        return;
    // Calculate from the companion's level, outside human XP hooks and rested XP.
    uint32 const base = Acore::XP::BaseGain(companion->Level, defeated->GetLevel(),
                                            GetContentLevelsForMapAndZone(defeated->GetMapId(), defeated->GetZoneId()));
    double const modifier =
        Config.CompanionXP * defeated->GetCreatureTemplate()->ModExperience * (defeated->isElite() ? 2.0 : 1.0);
    uint32 const amount = ScaleXP(base, modifier);
    Progress const progress = GainExperience({companion->Level, companion->Experience}, amount);
    bool const levelled = progress.Level > companion->Level;
    auto const form = levelled ? ResolveEvolution(*companion, progress.Level) :
                                std::make_pair(companion->SpeciesId, companion->Evolution);
    bool const evolved = form.first != companion->SpeciesId;
    auto transaction = std::make_shared<Transaction<Connection>>();
    transaction->Append(
        Make(SaveProgress, progress.Level, progress.Experience, actor->GetHealthPct(), form.first, uint8(form.second),
             OwnerId(player), companion->Id));
    if (evolved)
        transaction->Append(Make(PruneEquipment, OwnerId(player), companion->Id));
    transaction->Append(Make(RewardBits, Config.BitsPerKill, OwnerId(player), Config.BitsPerKill));
    if (Database->ExecuteTransaction(transaction) != 0 || !Reload(player))
    {
        LOG_ERROR("module.digimon", "Cannot persist companion reward for owner {}", OwnerId(player));
        Notify(player, "No se pudo guardar la recompensa del compañero. Consulta al administrador.");
        return;
    }
    if (levelled)
        if (Companion* updated = Active(Tamers.at(OwnerId(player))))
        {
            if (!ApplyForm(player, actor, *updated))
                LOG_ERROR("module.digimon", "Cannot apply evolved actor for owner {}", OwnerId(player));
            Notify(player, "Tu compañero ha alcanzado el nivel " + std::to_string(updated->Level) + ".");
            if (evolved)
                Notify(player, "¡Digievolución! Tu compañero ahora es " + Catalog.at(updated->SpeciesId).Name + ".");
            SendCombatState(player, Tamers.at(OwnerId(player)));
        }
}

bool ReadRate(std::string const& key, std::string const& fallback, double& rate)
{
    std::string const text = sConfigMgr->GetOption<std::string>(key, fallback);
    auto const parsed = std::from_chars(text.data(), text.data() + text.size(), rate);
    return parsed.ec == std::errc() && parsed.ptr == text.data() + text.size() && std::isfinite(rate) && rate >= 0 &&
           rate <= 100;
}

bool Summon(Player* player, Tamer& state)
{
    Companion* companion = Active(state);
    if (!companion || companion->Health <= 0 || !player->IsAlive() || !player->IsInWorld() || player->IsInFlight() ||
        player->GetVehicle() || player->InBattleground() || Busy(player, state))
        return false;
    if (Actor(player, state))
        return true;
    auto const species = Catalog.find(companion->SpeciesId);
    if (species == Catalog.end() || !species->second.Enabled)
        return false;
    Position const position = player->GetNearPosition(2.0f, 0.8f);
    TempSummon* actor = player->SummonCreature(species->second.Entry, position);
    if (!actor)
        return false;
    actor->SetOwnerGUID(player->GetGUID());
    actor->SetCreatorGUID(player->GetGUID());
    // Ownership alone does not set this flag for a plain TempSummon. Without it,
    // the core treats neutral-enemy attacks as NPC-versus-NPC and rejects them.
    actor->SetUnitFlag(UNIT_FLAG_PLAYER_CONTROLLED);
    actor->SetFaction(player->GetFaction());
    actor->SetName(species->second.Name);
    actor->SetReactState(REACT_PASSIVE);
    actor->InitCharmInfo(); // Provides the core's global cooldown manager for manual techniques.
    actor->SetNoCallAssistance(true);
    ApplyStats(actor, *companion);
    state.Actor = actor->GetGUID();
    actor->GetMotionMaster()->MoveFollow(player, 2.0f, 0.8f);
    if (!Execute(UpdateSummoned, uint8(1), OwnerId(player)))
    {
        actor->DespawnOrUnsummon();
        state.Actor.Clear();
        return false;
    }
    state.Summoned = true;
    return true;
}

void GiveDevice(Player* player)
{
    if (!player->HasItemCount(Config.Device, 1, true) && !player->AddItem(Config.Device, 1))
        Notify(player, "Libera un hueco para el Dispositivo Sagrado. Puedes usar .digimon mientras tanto.");
}

bool AtTrainer(Player* player, ObjectGuid source)
{
    if (!source.IsCreatureOrVehicle())
        return false;
    Creature* trainer = player->GetMap()->GetCreature(source);
    return trainer && trainer->GetEntry() == Config.Trainer && trainer->IsAlive() &&
           player->IsWithinDistInMap(trainer, INTERACTION_DISTANCE) && player->IsWithinLOSInMap(trainer);
}

bool IsEquipped(Companion const& companion, uint32 id)
{
    auto const technique = TechniqueCatalog.find(id);
    if (technique == TechniqueCatalog.end() || !technique->second.Enabled ||
        companion.Evolution < technique->second.Required || !companion.Learned.count(id) ||
        !TechniqueAllowed(companion.SpeciesId, id))
        return false;
    for (auto const& [slot, equipped] : companion.Equipped)
        if (equipped == id && CanEquip(companion.Evolution, slot, technique->second.Special))
            return true;
    return false;
}

void TechniquesMenu(Player* player, ObjectGuid source, Companion const& companion)
{
    ClearGossipMenuFor(player);
    player->PlayerTalkClass->GetGossipMenu().SetMenuId(DeviceMenu);
    for (uint32 id : companion.Learned)
        if (auto technique = TechniqueCatalog.find(id);
            technique != TechniqueCatalog.end() && technique->second.Enabled &&
            TechniqueAllowed(companion.SpeciesId, id))
            AddOption(player, GOSSIP_ICON_CHAT,
                      (IsEquipped(companion, id) ? "Retirar técnica: " : "Equipar técnica: ") + technique->second.Name,
                      IsEquipped(companion, id) ? Unequip : Equip, id);
    AddOption(player, GOSSIP_ICON_CHAT, "Volver al dispositivo", Main, Back);
    SendGossipMenuFor(player, 1, source);
}

void LearnTechnique(Player* player, ObjectGuid source, Tamer& state, uint32 id)
{
    Companion* companion = Active(state);
    auto const found = TechniqueCatalog.find(id);
    if (!AtTrainer(player, source) || !companion || Busy(player, state) || found == TechniqueCatalog.end())
        return;
    Technique const& technique = found->second;
    if (!technique.Enabled || companion->Evolution < technique.Required || companion->Learned.count(id) ||
        state.Bits < technique.Price || !TechniqueAllowed(companion->SpeciesId, id))
    {
        Notify(player, "No cumples los requisitos, ya conoces la técnica o faltan DigiBits.");
        return;
    }
    auto transaction = std::make_shared<Transaction<Connection>>();
    std::string const token = NewToken();
    transaction->Append(Make(DebitTechnique, id, token, OwnerId(player), companion->Id));
    transaction->Append(Make(LearnAfterDebit, id, OwnerId(player), companion->Id, token));
    if (Database->ExecuteTransaction(transaction) != 0 || !Reload(player))
    {
        Notify(player, "No se pudo guardar el entrenamiento. Consulta al administrador.");
        return;
    }
    companion = Active(Tamers.at(OwnerId(player)));
    Notify(player, companion && companion->Learned.count(id)
                       ? "Técnica aprendida. Equípala en el dispositivo para utilizarla."
                       : "La compra no se aplicó. Revisa los requisitos y el saldo.");
}

void SetTechnique(Player* player, Tamer& state, uint32 id, bool remove)
{
    Companion* companion = Active(state);
    auto const found = TechniqueCatalog.find(id);
    if (!companion || Busy(player, state) || found == TechniqueCatalog.end() || !companion->Learned.count(id))
        return;
    if (remove)
    {
        if (Execute(UnequipTechnique, OwnerId(player), companion->Id, id))
            Reload(player);
        return;
    }
    Technique const& technique = found->second;
    if (!technique.Enabled || companion->Evolution < technique.Required || IsEquipped(*companion, id) ||
        !TechniqueAllowed(companion->SpeciesId, id))
        return;
    for (uint8 slot = 0; slot <= 4; ++slot)
        if (CanEquip(companion->Evolution, slot, technique.Special) && !companion->Equipped.count(slot))
        {
            if (Execute(EquipTechnique, slot, OwnerId(player), companion->Id, id))
                Reload(player);
            return;
        }
    Notify(player, "No queda un espacio compatible. Retira una técnica equipada.");
}

void UseTechnique(Player* player, Tamer& state, uint32 id)
{
    Companion* companion = Active(state);
    Creature* actor = Actor(player, state);
    if (!player->IsAlive() || !companion || !actor || !actor->IsAlive() || !IsEquipped(*companion, id) ||
        !player->IsWithinDistInMap(actor, 40.0f))
    {
        Notify(player, "Necesitas un compañero vivo y cercano con esa técnica equipada.");
        return;
    }
    auto const found = TechniqueCatalog.find(id);
    if (found == TechniqueCatalog.end() || !found->second.Enabled || !companion->Learned.count(id) ||
        companion->Evolution < found->second.Required)
        return;
    Technique const& technique = found->second;
    SpellInfo const* spell = sSpellMgr->GetSpellInfo(technique.Spell);
    if (!spell)
        return;
    Unit* target = spell->IsPositive() ? actor : player->GetSelectedUnit();
    if (!target || !target->IsAlive() ||
        (!spell->IsPositive() && (!target->IsCreature() || !actor->IsValidAttackTarget(target) ||
                                 !player->IsValidAttackTarget(target) ||
                                 !player->IsWithinDistInMap(target, 40.0f) || !player->IsWithinLOSInMap(target))))
    {
        Notify(player, "Selecciona una criatura enemiga cercana y a la vista para utilizar esta técnica.");
        return;
    }
    if (!spell->IsPositive())
    {
        actor->AI()->AttackStart(target);
        actor->SetFacingToObject(target);
    }
    SpellCastResult const result = actor->HasSpellCooldown(technique.Spell)
                                       ? SPELL_FAILED_NOT_READY
                                       : actor->CastSpell(target, technique.Spell, false);
    switch (result)
    {
    case SPELL_CAST_OK:
        actor->AddSpellCooldown(technique.Spell, 0, 0);
        SendUI(player, "CAST|" + std::to_string(technique.Spell));
        break;
    case SPELL_FAILED_NO_POWER:
        Notify(player, "El compañero necesita más energía; se recupera automáticamente.");
        break;
    case SPELL_FAILED_OUT_OF_RANGE:
        Notify(player, "El compañero se acerca al objetivo. Usa la técnica cuando esté a su alcance.");
        break;
    case SPELL_FAILED_NOT_READY:
        Notify(player, "La técnica todavía está recuperándose.");
        break;
    case SPELL_FAILED_LINE_OF_SIGHT:
        Notify(player, "El compañero no tiene al objetivo a la vista.");
        break;
    default:
        Notify(player, "El compañero no puede usar la técnica ahora (código " + std::to_string(result) + ").");
        break;
    }
    SendCombatState(player, state);
}

void CaptureTarget(Player* player, Tamer& state)
{
    Creature* actor = Actor(player, state);
    Unit* selected = player->GetSelectedUnit();
    Creature* target = selected ? selected->ToCreature() : nullptr;
    auto wild = target ? Wild.find(target->GetGUID()) : Wild.end();
    if (!actor || !actor->IsAlive() || !target || !target->IsAlive() || wild == Wild.end() || wild->second.Claimed ||
        target->IsSummon() || target->GetOwnerGUID() || target->GetCharmerGUID() ||
        target->GetCreatureTemplate()->rank != CREATURE_ELITE_NORMAL ||
        target->HasFlagsExtra(CREATURE_FLAG_EXTRA_DUNGEON_BOSS) || ProtectedQuestEntries.count(target->GetEntry()) ||
        !player->IsWithinDistInMap(target, 15.0f) || !player->IsWithinLOSInMap(target) ||
        !player->IsWithinDistInMap(actor, 40.0f) || target->GetHealthPct() > Config.CaptureHealth ||
        target->GetLootRecipient() != player)
    {
        Notify(player, "Selecciona un salvaje autorizado, debilitado por tu compañero, cercano y a la vista.");
        return;
    }
    uint32 const owner = OwnerId(player);
    auto const damage = wild->second.WeakenedBy.find(owner);
    if (damage == wild->second.WeakenedBy.end() || damage->second != state.Actor)
    {
        Notify(player, "Tu compañero activo debe debilitar a este salvaje antes de capturarlo.");
        return;
    }
    uint32 speciesId = 0;
    for (auto const& [id, species] : Catalog)
        if (species.CaptureEntry == target->GetEntry())
            speciesId = id;
    if (!speciesId || state.Companions.size() >= state.Capacity)
    {
        Notify(player, "No es capturable o tu colección está llena.");
        return;
    }
    uint64 const now = GameTime::GetGameTime().count();
    if (now < state.CaptureReady)
    {
        Notify(player,
               "El dispositivo está recuperándose. Espera " + std::to_string(state.CaptureReady - now) + " segundos.");
        return;
    }
    std::string const captureKey = wild->second.Token;
    std::string const attempt = NewToken();
    bool const caught = roll_chance_i(Config.CaptureChance);
    auto transaction = std::make_shared<Transaction<Connection>>();
    transaction->Append(Make(BeginCaptureAttempt, now + Config.CaptureCooldown, attempt, owner, now));
    if (caught)
    {
        transaction->Append(Make(InsertCaptured, target->GetLevel(), target->GetHealthPct(), speciesId, captureKey,
                                 owner, attempt, target->GetEntry()));
        transaction->Append(Make(ClaimCapturedWild, now + std::max(30u, target->GetRespawnDelay()), captureKey, owner));
    }
    if (Database->ExecuteTransaction(transaction) != 0 || !Reload(player))
    {
        Notify(player, "No se pudo guardar el intento. El salvaje sigue en el mundo.");
        return;
    }
    auto captured = Query(SelectCaptured, captureKey);
    if (!captured || captured->Fetch()[1].Get<uint32>() != owner)
    {
        Notify(player, caught ? "No se aplicó la captura. Revisa el espacio de la colección."
                              : "El salvaje se ha resistido. Vuelve a intentarlo cuando el dispositivo se recupere.");
        return;
    }
    wild->second.Claimed = true;
    wild->second.ReadyAt = now + std::max(30u, target->GetRespawnDelay());
    if (actor->GetVictim() == target)
    {
        actor->AttackStop();
        actor->GetThreatMgr().ClearAllThreat();
        actor->GetMotionMaster()->MoveFollow(player, 2.0f, 0.8f);
    }
    // The durable collection row exists before the wild creature disappears.
    target->DespawnOrUnsummon();
    Notify(player, "Captura guardada en tu colección. Conserva su nivel y su salud actual.");
}

void ActivateCompanion(Player* player, Tamer& state, uint32 id)
{
    if (id == state.Active || Busy(player, state) || player->IsInFlight() || player->GetVehicle())
        return;
    bool owned = false;
    for (auto const& companion : state.Companions)
        owned = owned || companion.Id == id;
    if (!owned)
        return;
    PersistHealth(player, state);
    auto transaction = std::make_shared<Transaction<Connection>>();
    transaction->Append(Make(ChangeActive, OwnerId(player), id, id));
    transaction->Append(Make(SetTeamLead, OwnerId(player)));
    transaction->Append(Make(UpdateSummoned, uint8(0), OwnerId(player)));
    if (Database->ExecuteTransaction(transaction) != 0)
        return;
    Despawn(player, state);
    if (Reload(player))
        Summon(player, Tamers.at(OwnerId(player)));
}

void CollectionMenu(Player* player, ObjectGuid source, Tamer const& state, uint32 offset = 0)
{
    Notify(player, "Colección: " + std::to_string(state.Companions.size()) + "/" + std::to_string(state.Capacity) +
                       ". Equipo operativo: 1. Invocados simultáneos: 1. DigiBits: " + std::to_string(state.Bits) +
                       ".");
    auto unlock = Unlocks.upper_bound(player->GetLevel());
    if (unlock != Unlocks.begin())
    {
        --unlock;
        Notify(player, "Plan de nivel: " + std::to_string(unlock->second.Prepared) + " espacios de equipo, " +
                           std::to_string(unlock->second.Simultaneous) + " simultáneos y DigiPociones de grado " +
                           std::to_string(unlock->second.PotionTier) +
                           ". Estas ampliaciones siguen en preparación; esta Alpha utiliza un compañero.");
    }
    ClearGossipMenuFor(player);
    player->PlayerTalkClass->GetGossipMenu().SetMenuId(DeviceMenu);
    std::size_t const begin = offset < state.Companions.size() ? offset : 0;
    std::size_t const end = std::min(begin + 20, state.Companions.size());
    for (std::size_t index = begin; index < end; ++index)
    {
        Companion const& companion = state.Companions[index];
        if (auto species = Catalog.find(companion.SpeciesId); species != Catalog.end())
            AddOption(player, GOSSIP_ICON_CHAT,
                      (companion.Id == state.Active ? "[Activo] " : "Activar ") + species->second.Name + " #" +
                          std::to_string(companion.Id) + " — nivel " + std::to_string(companion.Level) + ", salud " +
                          std::to_string(uint32(companion.Health)) + "%",
                      Activate, companion.Id);
    }
    if (begin >= 20)
        AddOption(player, GOSSIP_ICON_CHAT, "Compañeros anteriores", CollectionPage, static_cast<uint32>(begin - 20));
    if (end < state.Companions.size())
        AddOption(player, GOSSIP_ICON_CHAT, "Más compañeros", CollectionPage, static_cast<uint32>(end));
    AddOption(player, GOSSIP_ICON_CHAT, "Volver al dispositivo", Main, Back);
    SendGossipMenuFor(player, 1, source);
}

void Menu(Player* player, ObjectGuid source, bool welcome)
{
    std::lock_guard<std::recursive_mutex> guard(Mutex);
    if (!Config.Enabled || !Database || (!Tamers.count(OwnerId(player)) && !Reload(player)))
        return;
    Tamer& state = Tamers.at(OwnerId(player));
    ClearGossipMenuFor(player);
    player->PlayerTalkClass->GetGossipMenu().SetMenuId(DeviceMenu);
    if (!state.Active && welcome)
        for (auto const& [id, species] : Catalog)
            if (species.Starter && species.Enabled)
                AddOption(player, GOSSIP_ICON_CHAT, "Elegir " + species.Name, ChooseStarter, id);
    if (state.Active)
    {
        AddOption(player, GOSSIP_ICON_BATTLE, "Luchar — mostrar barra de combate", Main, Fight);
        AddOption(player, GOSSIP_ICON_CHAT, "Invocar compañero", Main, Invoke);
        AddOption(player, GOSSIP_ICON_CHAT, "Retirar compañero", Main, Withdraw);
        AddOption(player, GOSSIP_ICON_CHAT, "Atacar mi objetivo", Main, Attack);
        AddOption(player, GOSSIP_ICON_CHAT, "Dejar de atacar y seguirme", Main, Follow);
        AddOption(player, GOSSIP_ICON_CHAT, "Descansar y recuperar al compañero (fuera de combate)", Main, Recover);
        AddOption(player, GOSSIP_ICON_CHAT, "Gestionar técnicas aprendidas y equipadas", Main, Techniques);
        AddOption(player, GOSSIP_ICON_CHAT, "Capturar el salvaje seleccionado", Main, Capture);
        if (Companion* companion = Active(state))
            for (auto const& [id, technique] : TechniqueCatalog)
            {
                if (IsEquipped(*companion, id))
                    AddOption(player, GOSSIP_ICON_CHAT, "Usar: " + technique.Name, Use, id);
                if (AtTrainer(player, source) && technique.Enabled && TechniqueAllowed(companion->SpeciesId, id) &&
                    companion->Evolution >= technique.Required &&
                    !companion->Learned.count(id))
                    AddOption(player, GOSSIP_ICON_TRAINER,
                              "Aprender " + technique.Name + " (" + std::to_string(technique.Price) + " DigiBits)", Buy,
                              id);
            }
    }
    AddOption(player, GOSSIP_ICON_CHAT, "Consultar colección y espacios", Main, Collection);
    SendGossipMenuFor(player, 1, source);
}

void Select(Player* player, ObjectGuid source, bool welcome, uint32 sender, uint32 action, bool reopen = true)
{
    std::lock_guard<std::recursive_mutex> guard(Mutex);
    CloseGossipMenuFor(player);
    if (!Config.Enabled || !Database || !player->IsAlive() || !Tamers.count(OwnerId(player)))
        return;
    uint32 const owner = OwnerId(player);
    Tamer& state = Tamers.at(owner);
    if (sender == Buy)
        LearnTechnique(player, source, state, action);
    else if (sender == Equip || sender == Unequip)
        SetTechnique(player, state, action, sender == Unequip);
    else if (sender == Use)
        UseTechnique(player, state, action);
    else if (sender == Activate)
        ActivateCompanion(player, state, action);
    else if (sender == CollectionPage)
    {
        CollectionMenu(player, source, state, action);
        return;
    }
    else if (sender == ChooseStarter)
    {
        if (!welcome || Busy(player, state) || state.Active || Query(SelectStarter, owner) || !Catalog.count(action) ||
            !Catalog.at(action).Starter || !Catalog.at(action).Enabled)
            return;
        auto transaction = std::make_shared<Transaction<Connection>>();
        transaction->Append(Make(InsertStarter, owner, owner, action));
        transaction->Append(Make(InsertInitialActive, owner));
        transaction->Append(Make(SetTeamLead, owner));
        if (Database->ExecuteTransaction(transaction) != 0 || !Reload(player))
        {
            Notify(player, "No se pudo guardar la selección. No se ha creado ningún compañero temporal.");
            return;
        }
        GiveDevice(player);
        if (!Summon(player, Tamers.at(owner)))
            Notify(player, "Tu elección está guardada. Invoca al compañero con el dispositivo cuando puedas.");
    }
    else if (sender == Main)
    {
        switch (action)
        {
        case Fight:
            SendCombatState(player, state);
            SendUI(player, "FIGHT");
            Notify(player, "Modo Luchar: el complemento WorldDigimon muestra las órdenes en la barra principal.");
            return;
        case Invoke:
            if (!Summon(player, state))
                Notify(player, "No se puede invocar ahora: revisa salud, combate y ubicación.");
            break;
        case Withdraw:
            if (!Busy(player, state))
            {
                Despawn(player, state);
                if (Execute(UpdateSummoned, uint8(0), owner))
                    state.Summoned = false;
            }
            break;
        case Attack: {
            Creature* actor = Actor(player, state);
            Unit* target = player->GetSelectedUnit();
            if (!actor || !actor->IsAlive() || !target || !target->IsCreature() || target == actor ||
                !target->IsAlive())
            {
                Notify(player, "Invoca un compañero vivo y selecciona una criatura enemiga.");
                break;
            }
            actor->SetFaction(player->GetFaction());
            bool const hostile = player->IsValidAttackTarget(target) && actor->IsValidAttackTarget(target);
            bool const close = player->IsWithinDistInMap(target, 40.0f);
            bool const visible = player->IsWithinLOSInMap(target);
            LOG_DEBUG("module.digimon", "Attack request owner {} actor {} target {}: hostile {}, close {}, visible {}",
                      owner, actor->GetGUID().ToString(), target->GetGUID().ToString(), hostile, close, visible);
            if (!hostile || !close || !visible)
            {
                Notify(player, "El objetivo debe ser enemigo, estar a menos de 40 metros y a la vista.");
                break;
            }
            actor->AI()->AttackStart(target);
            if (actor->GetVictim() != target)
                Notify(player, "El compañero no pudo iniciar el ataque. Espera a que termine de regresar.");
            break;
        }
        case Follow:
            if (Creature* actor = Actor(player, state))
            {
                actor->AttackStop();
                actor->GetThreatMgr().ClearAllThreat();
                actor->GetMotionMaster()->MoveFollow(player, 2.0f, 0.8f);
            }
            break;
        case Recover:
            if (!Busy(player, state) && state.Active)
            {
                Despawn(player, state);
                if (Execute(SaveHealth, 100.0f, owner, state.Active))
                {
                    Reload(player);
                    Summon(player, Tamers.at(owner));
                }
            }
            break;
        case Collection:
            CollectionMenu(player, source, state);
            return;
        case Capture:
            CaptureTarget(player, state);
            break;
        case Back:
            break;
        case Techniques:
            if (Companion* companion = Active(state))
            {
                TechniquesMenu(player, source, *companion);
                return;
            }
            break;
        default:
            return;
        }
    }
    SendCombatState(player, Tamers.at(owner));
    if (reopen)
        Menu(player, source, welcome);
}

class DigimonDatabaseScript final : public DatabaseScript
{
public:
    DigimonDatabaseScript() : DatabaseScript("DigimonDatabaseScript") {}

    bool OnModuleDatabasesLoading() override
    {
        Config.Enabled = sConfigMgr->GetOption<bool>("DigimonTamer.Enable", true);
        Config.Welcome = sConfigMgr->GetOption<uint32>("DigimonTamer.WelcomeEntry", 900100);
        Config.Device = sConfigMgr->GetOption<uint32>("DigimonTamer.DeviceEntry", 900110);
        Config.Trainer = sConfigMgr->GetOption<uint32>("DigimonTamer.TrainerEntry", 900101);
        Config.InitialBits = sConfigMgr->GetOption<uint32>("DigimonTamer.InitialBits", 100);
        if (!Config.Enabled)
            return true;
        if (!ReadRate("DigimonTamer.TamerXPRate", "0.70", Config.TamerXP) ||
            !ReadRate("DigimonTamer.CompanionXPRate", "1.0", Config.CompanionXP))
        {
            LOG_ERROR("module.digimon", "XP rates must be finite decimal numbers between 0 and 100.");
            return false;
        }
        Config.BitsPerKill = sConfigMgr->GetOption<uint32>("DigimonTamer.BitsPerKill", 5);
        Config.CaptureChance = sConfigMgr->GetOption<uint32>("DigimonTamer.CaptureChance", 35);
        Config.CaptureHealth = sConfigMgr->GetOption<uint32>("DigimonTamer.CaptureHealthPercent", 25);
        Config.CaptureCooldown = sConfigMgr->GetOption<uint32>("DigimonTamer.CaptureCooldownSeconds", 10);
        Config.Experiments = sConfigMgr->GetOption<bool>("DigimonTamer.ExperimentsEnable", false);
        if (Config.BitsPerKill > 1000000 || Config.CaptureChance > 100 || !Config.CaptureHealth ||
            Config.CaptureHealth >= 100 || !Config.CaptureCooldown || Config.CaptureCooldown > 3600 ||
            sConfigMgr->GetOption<uint32>("DigimonTamer.MaxSimultaneous", 1) != 1)
        {
            LOG_ERROR("module.digimon",
                      "Invalid capture configuration, wallet reward or operational summon cap (must remain one).");
            return false;
        }
        DatabaseInfo =
            std::make_unique<MySQLConnectionInfo>(sConfigMgr->GetOption<std::string>("CharacterDatabaseInfo", ""));
        Database = std::make_unique<Connection>(*DatabaseInfo);
        if (Database->Open() || !Database->PrepareStatements())
        {
            LOG_ERROR("module.digimon", "Cannot open Digimon tables. Apply the module characters migration first.");
            return false;
        }
        if (auto rows = Query(SelectSpecies))
            do
            {
                Field* fields = rows->Fetch();
                Catalog.emplace(fields[0].Get<uint32>(), Species{fields[1].Get<uint32>(), fields[2].Get<std::string>(),
                                                                 fields[3].Get<uint8>() != 0, fields[4].Get<uint32>(),
                                                                 static_cast<Stage>(fields[5].Get<uint8>()),
                                                                 fields[6].Get<uint8>() != 0});
            } while (rows->NextRow());
        if (auto rows = Query(SelectTechniques))
            do
            {
                Field* fields = rows->Fetch();
                TechniqueCatalog.emplace(fields[0].Get<uint32>(),
                                         Technique{fields[1].Get<std::string>(), fields[2].Get<uint32>(),
                                                   static_cast<Stage>(fields[3].Get<uint8>()), fields[4].Get<uint64>(),
                                                   fields[5].Get<uint8>() != 0, fields[6].Get<uint8>() != 0});
            } while (rows->NextRow());
        if (auto rows = Query(SelectUnlocks))
            do
            {
                Field* fields = rows->Fetch();
                Unlocks.emplace(fields[0].Get<uint8>(),
                                Unlock{fields[1].Get<uint8>(), fields[2].Get<uint8>(), fields[3].Get<uint8>()});
            } while (rows->NextRow());
        if (auto rows = Query(SelectAliases))
            do
            {
                Field* fields = rows->Fetch();
                Aliases.emplace(CatalogKey(fields[0].Get<std::string>()), fields[1].Get<uint32>());
            } while (rows->NextRow());
        if (auto rows = Query(SelectSpeciesTechniques))
            do
            {
                Field* fields = rows->Fetch();
                SpeciesTechniques[fields[0].Get<uint32>()].insert(fields[1].Get<uint32>());
            } while (rows->NextRow());
        if (auto rows = Query(SelectEvolutions))
            do
            {
                Field* fields = rows->Fetch();
                Evolutions.emplace(fields[0].Get<uint32>(), Evolution{fields[1].Get<uint32>(), fields[2].Get<uint8>()});
            } while (rows->NextRow());
        return !Catalog.empty();
    }

    void OnModuleDatabasesKeepAlive() override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        if (Database)
            Database->Ping();
    }

    void OnModuleDatabasesClosing() override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        Tamers.clear();
        Catalog.clear();
        TechniqueCatalog.clear();
        Aliases.clear();
        SpeciesTechniques.clear();
        Evolutions.clear();
        PreviewActors.clear();
        Wild.clear();
        ProtectedQuestEntries.clear();
        Unlocks.clear();
        LabActors.clear();
        Database.reset();
        DatabaseInfo.reset();
    }
};

class DigimonWorldScript final : public WorldScript
{
public:
    DigimonWorldScript() : WorldScript("DigimonWorldScript") {}
    void OnStartup() override
    {
        if (!Config.Enabled)
            return;
        for (auto const& [id, species] : Catalog)
            if (species.Enabled && (!sObjectMgr->GetCreatureTemplate(species.Entry) ||
                sObjectMgr->GetCreatureTemplate(species.Entry)->ScriptID !=
                    sObjectMgr->GetScriptId("DigimonCompanionAI")))
            {
                LOG_ERROR("module.digimon", "Species {} references missing creature {}", id, species.Entry);
                World::StopNow(ERROR_EXIT_CODE);
                return;
            }
        for (auto const& [id, technique] : TechniqueCatalog)
        {
            if (technique.Name.empty() || technique.Name.size() > 160 ||
                technique.Name.find_first_of("|\t\r\n") != std::string::npos)
            {
                LOG_ERROR("module.digimon", "Invalid technique name {}", id);
                World::StopNow(ERROR_EXIT_CODE);
                return;
            }
            if (technique.Enabled)
            {
                SpellInfo const* spell = sSpellMgr->GetSpellInfo(technique.Spell);
                if (!spell)
                {
                    LOG_ERROR("module.digimon", "Enabled technique {} references missing spell {}", id,
                              technique.Spell);
                    World::StopNow(ERROR_EXIT_CODE);
                    return;
                }
                LOG_INFO("module.digimon", "Validated technique {}: spell {} ({})", id, technique.Spell,
                         spell->SpellName[0]);
            }
        }
        for (auto const& [source, rule] : Evolutions)
            if (!Catalog.count(source) || !Catalog.count(rule.Target) || !Catalog.at(source).Enabled ||
                !Catalog.at(rule.Target).Enabled || !EvolutionReady(rule.Level, rule.Level,
                    Catalog.at(source).DefaultStage, Catalog.at(rule.Target).DefaultStage))
            {
                LOG_ERROR("module.digimon", "Invalid enabled evolution rule for species {}", source);
                World::StopNow(ERROR_EXIT_CODE);
                return;
            }
        for (auto const& [questId, quest] : sObjectMgr->GetQuestTemplates())
            for (int32 entry : quest->RequiredNpcOrGo)
                if (entry > 0)
                    ProtectedQuestEntries.insert(static_cast<uint32>(entry));
        LOG_INFO("module.digimon", "Alpha 0.3 loaded: {} species; capture chance {}%; protected quest entries {}.",
                 Catalog.size(), Config.CaptureChance, ProtectedQuestEntries.size());
        if (Config.Experiments)
            LOG_WARN("module.digimon",
                     "Administrator-only multi-follower experiment enabled; operational companion cap remains one.");
    }
};

class DigimonPlayerScript final : public PlayerScript
{
public:
    DigimonPlayerScript() : PlayerScript("DigimonPlayerScript") {}
    void OnPlayerLogin(Player* player) override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        if (!Config.Enabled || !Database || !Reload(player))
            return;
        Tamer& state = Tamers.at(OwnerId(player));
        if (state.Active)
        {
            GiveDevice(player);
            if (state.Summoned)
                Summon(player, state);
        }
        Notify(player, "Bienvenido, Tamer. Tu compañero combate por ti. Usa el Dispositivo Sagrado o .digimon.");
    }
    void OnPlayerLogout(Player* player) override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        PreviewActors.erase(OwnerId(player));
        auto state = Tamers.find(OwnerId(player));
        if (state != Tamers.end())
        {
            Despawn(player, state->second);
            Tamers.erase(state);
        }
    }
    void OnPlayerSave(Player* player) override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        auto state = Tamers.find(OwnerId(player));
        if (state != Tamers.end())
            PersistHealth(player, state->second);
    }
    void OnPlayerMapChanged(Player* player) override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        PreviewActors.erase(OwnerId(player));
        auto state = Tamers.find(OwnerId(player));
        if (state != Tamers.end())
        {
            state->second.Actor.Clear();
            if (state->second.Summoned)
                Summon(player, state->second);
        }
    }
    bool OnPlayerBeforeTeleport(Player* player, uint32, float, float, float, float, uint32, Unit*) override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        auto state = Tamers.find(OwnerId(player));
        if (player->IsInWorld() && state != Tamers.end())
            PersistHealth(player, state->second);
        return true;
    }
    void OnPlayerGiveXP(Player*, uint32& amount, Unit*, uint8) override
    {
        if (Config.Enabled)
            amount = ScaleXP(amount, Config.TamerXP);
    }
    void OnPlayerGossipSelect(Player* player, uint32 menu, uint32 sender, uint32 action) override
    {
        if (menu == DeviceMenu)
            Select(player, player->GetGUID(), false, sender, action);
    }
};

class DigimonDamageScript final : public UnitScript
{
public:
    DigimonDamageScript()
        : UnitScript("DigimonDamageScript", true,
                     {UNITHOOK_ON_DAMAGE, UNITHOOK_MODIFY_MELEE_DAMAGE, UNITHOOK_MODIFY_SPELL_DAMAGE_TAKEN,
                      UNITHOOK_MODIFY_PERIODIC_DAMAGE_AURAS_TICK})
    {
    }
    void FilterDamage(Unit* attacker, Unit* victim, uint32& damage)
    {
        if (!Config.Enabled || !attacker || attacker == victim)
            return;
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        bool const playerOrigin = attacker->IsPlayer() || attacker->GetOwnerGUID().IsPlayer() ||
                                  attacker->GetCharmerGUID().IsPlayer() ||
                                  attacker->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (playerOrigin && !Managed(attacker))
            damage = 0;
    }
    void OnDamage(Unit* attacker, Unit* victim, uint32& damage) override
    {
        FilterDamage(attacker, victim, damage);
        if (!Config.Enabled || !damage || !attacker || !victim || !victim->IsCreature())
            return;
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        if (Managed(attacker))
            if (Player* owner = attacker->GetCharmerOrOwnerPlayerOrPlayerItself())
                if (!owner->IsGameMaster())
                {
                    // This core revision requires a player participation marker,
                    // even for kills made entirely by owned summons. Preserve the
                    // normal damage threshold: only mark actual companion damage.
                    victim->ToCreature()->LowerPlayerDamageReq(0, true, owner->GetLevel());
                    if (auto wild = Wild.find(victim->GetGUID()); wild != Wild.end() && !wild->second.Claimed)
                        wild->second.WeakenedBy[OwnerId(owner)] = attacker->GetGUID();
                }
    }
    void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage) override
    {
        // Apply before the combat log packet as well as at final damage delivery.
        FilterDamage(attacker, target, damage);
    }
    void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage, SpellInfo const*) override
    {
        FilterDamage(attacker, target, damage);
    }
    void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const*) override
    {
        if (damage <= 0)
            return;
        uint32 amount = static_cast<uint32>(damage);
        FilterDamage(attacker, target, amount);
        damage = static_cast<int32>(amount);
    }
};

class DigimonSpellScript final : public AllSpellScript
{
public:
    DigimonSpellScript() : AllSpellScript("DigimonSpellScript", {ALLSPELLHOOK_CAN_PREPARE}) {}
    bool CanPrepare(Spell* spell, SpellCastTargets const*, AuraEffect const*) override
    {
        if (!Config.Enabled || !spell->GetCaster()->IsPlayer())
            return true;
        SpellInfo const* info = spell->GetSpellInfo();
        if (!info->IsPositive())
        {
            spell->SendCastResult(SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW);
            return false;
        }
        for (auto const& effect : info->Effects)
            if (effect.Effect == SPELL_EFFECT_INSTAKILL || effect.Effect == SPELL_EFFECT_SCHOOL_DAMAGE ||
                effect.Effect == SPELL_EFFECT_SUMMON || effect.Effect == SPELL_EFFECT_SUMMON_PET)
            {
                spell->SendCastResult(SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW);
                return false;
            }
        return true;
    }
};

class DigimonWelcome final : public CreatureScript
{
public:
    DigimonWelcome() : CreatureScript("DigimonWelcome") {}
    bool OnGossipHello(Player* player, Creature* creature) override
    {
        Menu(player, creature->GetGUID(), creature->GetEntry() == Config.Welcome);
        return true;
    }
    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        if (player->IsWithinDistInMap(creature, INTERACTION_DISTANCE))
            Select(player, creature->GetGUID(), creature->GetEntry() == Config.Welcome, sender, action);
        return true;
    }
};

class DigimonDevice final : public ItemScript
{
public:
    DigimonDevice() : ItemScript("DigimonDevice") {}
    bool OnUse(Player* player, Item* item, SpellCastTargets const&) override
    {
        if (item->GetEntry() != Config.Device)
            return false;
        // An intercepted item use must unlock the client item cursor/grey state.
        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        if (auto state = Tamers.find(OwnerId(player)); state != Tamers.end() && state->second.Addon)
        {
            SendCombatState(player, state->second);
            SendUI(player, "MENU");
        }
        else
            Menu(player, item->GetGUID(), false);
        return true;
    }
    void OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action) override
    {
        if (item->GetEntry() == Config.Device && item->GetOwnerGUID() == player->GetGUID())
            Select(player, item->GetGUID(), false, sender, action);
    }
};

class DigimonCommand final : public CommandScript
{
public:
    DigimonCommand() : CommandScript("DigimonCommand") {}
    Acore::ChatCommands::ChatCommandTable GetCommands() const override
    {
        using namespace Acore::ChatCommands;
        return {{"digimon", OpenDevice, rbac::RBAC_PERM_COMMAND_HELP, Console::No},
                {"spawndigi", SpawnPreview, rbac::RBAC_PERM_COMMAND_NPC_ADD, Console::No},
                {"digilevel", SetCompanionLevel, rbac::RBAC_PERM_COMMAND_NPC_ADD, Console::No},
                {"listadigi", ListCatalog, rbac::RBAC_PERM_COMMAND_NPC_ADD, Console::No},
                {"digimonlab", MultiFollowerExperiment, rbac::RBAC_PERM_COMMAND_NPC_ADD, Console::No}};
    }
    static Player* Administrator(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();
        return Config.Enabled && Database && player && player->IsInWorld() &&
                       player->GetSession()->GetSecurity() >= SEC_ADMINISTRATOR ? player : nullptr;
    }
    static bool ListCatalog(ChatHandler* handler)
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        if (!Administrator(handler))
            return false;
        for (auto const& [id, species] : Catalog)
            handler->SendSysMessage(std::to_string(id) + " · " + species.Name +
                                    (species.Enabled ? " · disponible" : " · pendiente de instalar/habilitar"));
        for (auto const& [name, id] : Aliases)
            handler->SendSysMessage("Alias: " + name + " → " + std::to_string(id));
        return true;
    }
    static bool SpawnPreview(ChatHandler* handler, char const* args)
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        Player* player = Administrator(handler);
        if (!player || !player->IsAlive() || player->IsInCombat() || player->InBattleground() ||
            player->GetVehicle() || player->IsInFlight())
            return false;
        auto& previews = PreviewActors[OwnerId(player)];
        std::string name = args ? args : "";
        if (CatalogKey(name) == "borrar")
        {
            for (ObjectGuid guid : previews)
                if (Creature* actor = player->GetMap()->GetCreature(guid))
                    actor->DespawnOrUnsummon();
            previews.clear();
            handler->SendSysMessage("Invocaciones de prueba retiradas.");
            return true;
        }
        std::erase_if(previews, [player](ObjectGuid guid) { return !player->GetMap()->GetCreature(guid); });
        if (previews.size() >= 5)
        {
            handler->SendSysMessage("Máximo cinco pruebas. Usa .spawndigi borrar.");
            return true;
        }
        uint32 level = 1;
        if (auto const space = name.find_last_of(' '); space != std::string::npos)
        {
            uint32 parsedLevel = 0;
            auto const result = std::from_chars(name.data() + space + 1, name.data() + name.size(), parsedLevel);
            if (result.ec == std::errc() && result.ptr == name.data() + name.size())
            {
                level = parsedLevel;
                name.resize(space);
            }
        }
        uint32 const id = ResolveSpecies(name);
        if (!id || level < 1 || level > 80)
        {
            handler->SendSysMessage("Uso: .spawndigi Nombre o ID [nivel 1–80]. Consulta .listadigi.");
            return true;
        }
        Species const& species = Catalog.at(id);
        if (!species.Enabled)
        {
            handler->SendSysMessage("Ese Digimon está pendiente de instalar/habilitar. No se usará otro modelo.");
            return true;
        }
        CreatureModel const* model = sObjectMgr->ChooseDisplayId(sObjectMgr->GetCreatureTemplate(species.Entry));
        if (!model)
            return false;
        Position const position = player->GetNearPosition(4.0f, 0.0f);
        if (TempSummon* actor = player->SummonCreature(PreviewEntry, position, TEMPSUMMON_TIMED_DESPAWN, 180000))
        {
            actor->SetCreatorGUID(player->GetGUID());
            actor->SetDisplayId(model->CreatureDisplayID, model->DisplayScale);
            actor->SetNativeDisplayId(model->CreatureDisplayID);
            actor->SetName(species.Name + " (prueba)");
            actor->SetReactState(REACT_PASSIVE);
            actor->SetNoCallAssistance(true);
            ApplyStats(actor, Companion{0, id, species.DefaultStage, uint8(level), 0, 100.0f, {}, {}});
            previews.push_back(actor->GetGUID());
            handler->SendSysMessage("Prueba temporal creada durante 180 segundos; sin botín, XP ni captura.");
            LOG_INFO("module.digimon", "Admin account {} previewed species {} at level {}",
                     player->GetSession()->GetAccountId(), id, level);
            return true;
        }
        return false;
    }
    static bool SetCompanionLevel(ChatHandler* handler, char const* args)
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        Player* player = Administrator(handler);
        if (!player || (!Tamers.count(OwnerId(player)) && !Reload(player)))
            return false;
        std::string_view const text = args ? args : "";
        uint32 level = 0;
        auto const parsed = std::from_chars(text.data(), text.data() + text.size(), level);
        if (parsed.ec != std::errc() || parsed.ptr != text.data() + text.size() || level < 1 || level > 80)
        {
            handler->SendSysMessage("Uso: .digilevel 1–80. Ajusta tu compañero actual; XP parcial a cero.");
            return true;
        }
        Tamer& state = Tamers.at(OwnerId(player));
        Companion* companion = Active(state);
        Creature* actor = Actor(player, state);
        if (!player->IsAlive() || Busy(player, state) || !companion || companion->Health <= 0 ||
            (actor && !actor->IsAlive()))
        {
            handler->SendSysMessage("Necesitas un compañero vivo y estar fuera de combate.");
            return true;
        }
        auto const form = level > companion->Level ? ResolveEvolution(*companion, uint8(level)) :
                                                   std::make_pair(companion->SpeciesId, companion->Evolution);
        float const health = actor ? actor->GetHealthPct() : companion->Health;
        auto transaction = std::make_shared<Transaction<Connection>>();
        transaction->Append(Make(SaveProgress, uint8(level), uint64(0), health, form.first, uint8(form.second),
                                 OwnerId(player), companion->Id));
        transaction->Append(Make(PruneEquipment, OwnerId(player), companion->Id));
        transaction->Append(Make(AuditLevel, player->GetSession()->GetAccountId(), OwnerId(player), companion->Id,
                                 companion->SpeciesId, form.first, companion->Level, uint8(level)));
        if (Database->ExecuteTransaction(transaction) != 0 || !Reload(player))
        {
            handler->SendSysMessage("No se pudo guardar el nivel. Consulta el registro del servidor.");
            return true;
        }
        Tamer& updated = Tamers.at(OwnerId(player));
        if (Companion* current = Active(updated); actor && current && !ApplyForm(player, actor, *current))
            LOG_ERROR("module.digimon", "Cannot apply admin level to actor for owner {}", OwnerId(player));
        SendCombatState(player, updated);
        handler->SendSysMessage("Compañero: " + Catalog.at(form.first).Name + " · nivel " + std::to_string(level) +
                                ". Guardado; evolución aplicada si corresponde. No hay devolución automática.");
        return true;
    }
    static bool OpenDevice(ChatHandler* handler, char const* args)
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        Player* player = handler->GetSession()->GetPlayer();
        if (!Config.Enabled || !Database || !player->IsInWorld() ||
            (!Tamers.count(OwnerId(player)) && !Reload(player)))
            return false;
        std::string_view const command = args ? args : "";
        Tamer& state = Tamers.at(OwnerId(player));
        if (command == "sync" || command == "ui")
        {
            state.Addon = true;
            state.SentTechniqueNames.clear();
            SendCombatState(player, state);
            if (command == "ui")
                SendUI(player, "MENU");
        }
        else if (command.substr(0, 5) == "cast ")
        {
            uint32 slot = 0;
            auto const parsed = std::from_chars(command.data() + 5, command.data() + command.size(), slot);
            if (parsed.ec != std::errc() || parsed.ptr != command.data() + command.size() || slot < 1 || slot > 5)
                return false;
            Companion* companion = Active(state);
            if (!companion || !companion->Equipped.count(uint8(slot - 1)))
                Notify(player, "Ese espacio no tiene una técnica equipada.");
            else
                UseTechnique(player, state, companion->Equipped.at(uint8(slot - 1)));
        }
        else
        {
            std::map<std::string_view, Action> const actions = {
                {"attack", Attack}, {"follow", Follow}, {"summon", Invoke}, {"withdraw", Withdraw},
                {"recover", Recover}, {"capture", Capture}, {"fight", Fight}, {"collection", Collection},
                {"techniques", Techniques}};
            if (auto action = actions.find(command); action != actions.end())
                Select(player, player->GetGUID(), false, Main, action->second, false);
            else if (command.empty())
                Menu(player, player->GetGUID(), false);
            else
                return false;
        }
        return true;
    }
    static bool MultiFollowerExperiment(ChatHandler* handler)
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        Player* player = handler->GetSession()->GetPlayer();
        if (!Config.Enabled || !Config.Experiments || !player->IsAlive() || player->IsInCombat() ||
            player->GetSession()->GetSecurity() < SEC_ADMINISTRATOR)
            return false;
        auto& followers = LabActors[OwnerId(player)];
        for (ObjectGuid guid : followers)
            if (Creature* actor = player->GetMap()->GetCreature(guid); actor && actor->IsInWorld())
                return false;
        followers.clear();
        // Separate, harmless followers: no collection rows, progression, pet bar
        // or combat privileges. Do not confuse this proof with three native pets.
        for (uint32 index = 0; index < 3; ++index)
        {
            Position const position = player->GetNearPosition(3.0f, 1.0f + index * 2.0f);
            if (TempSummon* actor = player->SummonCreature(900120, position, TEMPSUMMON_TIMED_DESPAWN, 30000))
            {
                actor->SetOwnerGUID(player->GetGUID());
                actor->SetFaction(player->GetFaction());
                actor->SetReactState(REACT_PASSIVE);
                actor->SetNoCallAssistance(true);
                followers.push_back(actor->GetGUID());
                actor->GetMotionMaster()->MoveFollow(player, 3.0f, 1.0f + index * 2.0f);
            }
        }
        handler->SendSysMessage(
            "Ensayo: tres seguidores temporales durante 30 segundos. El compañero operativo no cambia.");
        return true;
    }
};

struct DigimonLabFollowerAI : ScriptedAI
{
    explicit DigimonLabFollowerAI(Creature* creature) : ScriptedAI(creature) {}
    void AttackStart(Unit*) override {}
    void UpdateAI(uint32) override
    {
        Player* owner = ObjectAccessor::FindPlayer(me->GetOwnerGUID());
        if (!Config.Experiments || !owner || !owner->IsInWorld() || !owner->IsInMap(me))
            me->DespawnOrUnsummon();
    }
};

struct DigimonPreviewAI : ScriptedAI
{
    explicit DigimonPreviewAI(Creature* creature) : ScriptedAI(creature) {}
    void AttackStart(Unit*) override {}
    void UpdateAI(uint32) override
    {
        Player* creator = ObjectAccessor::FindPlayer(me->GetCreatorGUID());
        if (!creator || !creator->IsInWorld() || !creator->IsInMap(me))
            me->DespawnOrUnsummon();
    }
};

struct DigimonCompanionAI : ScriptedAI
{
    TaskScheduler CombatScheduler;

    explicit DigimonCompanionAI(Creature* creature) : ScriptedAI(creature)
    {
        CombatScheduler.Schedule(std::chrono::seconds(4), [this](TaskContext context)
        {
            std::lock_guard<std::recursive_mutex> guard(Mutex);
            if (me->IsAlive() && Managed(me))
                me->ModifyPower(POWER_FOCUS, int32(24.0f * sWorld->getRate(RATE_POWER_FOCUS)));
            context.Repeat();
        });
        CombatScheduler.Schedule(std::chrono::seconds(1), [this](TaskContext context)
        {
            std::lock_guard<std::recursive_mutex> guard(Mutex);
            if (Player* owner = ObjectAccessor::FindPlayer(me->GetOwnerGUID());
                owner && owner->IsInMap(me) && Managed(me))
                SendCombatState(owner, Tamers.at(OwnerId(owner)));
            context.Repeat();
        });
    }
    void KilledUnit(Unit* victim) override
    {
        RewardCompanion(me, victim);
    }
    void EnterEvadeMode(EvadeReason reason) override
    {
        if (!_EnterEvadeMode(reason))
            return;
        me->ClearUnitState(UNIT_STATE_EVADE);
        if (Player* owner = ObjectAccessor::FindPlayer(me->GetOwnerGUID()))
            me->GetMotionMaster()->MoveFollow(owner, 2.0f, 0.8f);
    }
    void JustDied(Unit*) override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        if (Player* owner = ObjectAccessor::FindPlayer(me->GetOwnerGUID()))
        {
            auto state = Tamers.find(OwnerId(owner));
            if (state != Tamers.end() && state->second.Actor == me->GetGUID())
                PersistHealth(owner, state->second);
        }
    }
    void UpdateAI(uint32 diff) override
    {
        Player* owner = ObjectAccessor::FindPlayer(me->GetOwnerGUID());
        {
            std::lock_guard<std::recursive_mutex> guard(Mutex);
            if (!owner || !owner->IsInWorld() || !owner->IsInMap(me) || !Managed(me))
            {
                me->DespawnOrUnsummon();
                return;
            }
        }
        CombatScheduler.Update(diff);
        if (Unit* target = me->GetVictim())
        {
            if (target->IsAlive() && me->IsValidAttackTarget(target))
                DoMeleeAttackIfReady();
            else
                EnterEvadeMode(EVADE_REASON_NO_HOSTILES);
        }
        else if (!me->IsInCombat() && !owner->IsInCombat() && !me->IsWithinDistInMap(owner, 60.0f))
        {
            Position const position = owner->GetNearPosition(2.0f, 0.8f);
            me->NearTeleportTo(position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                               position.GetOrientation());
            me->GetMotionMaster()->MoveFollow(owner, 2.0f, 0.8f);
        }
    }
};

struct DigimonWildAI : ScriptedAI
{
    explicit DigimonWildAI(Creature* creature) : ScriptedAI(creature)
    {
        NewLife();
    }
    ~DigimonWildAI() override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        Wild.erase(me->GetGUID());
    }
    void NewLife(bool respawn = false)
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        if (!Database || !me->GetSpawnId())
            return; // Summoned copies are deliberately not capturable.
        uint64 const now = GameTime::GetGameTime().count();
        if (!Execute(EnsureWildLife, me->GetSpawnId(), me->GetMapId(), me->GetInstanceId(), NewToken()))
            return;
        auto life = Query(SelectWildLife, me->GetSpawnId(), me->GetMapId(), me->GetInstanceId());
        if (!life)
            return;
        if (respawn || (life->Fetch()[1].Get<uint8>() && life->Fetch()[2].Get<uint64>() <= now))
        {
            if (!Execute(ResetWildLife, NewToken(), me->GetSpawnId(), me->GetMapId(), me->GetInstanceId(), now))
                return;
            life = Query(SelectWildLife, me->GetSpawnId(), me->GetMapId(), me->GetInstanceId());
            if (!life)
                return;
        }
        Field* fields = life->Fetch();
        Wild[me->GetGUID()] = {fields[0].Get<std::string>(), {}, fields[1].Get<uint8>() != 0, fields[2].Get<uint64>()};
    }
    void JustRespawned() override
    {
        NewLife(true);
    }
    void Reset() override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        if (auto wild = Wild.find(me->GetGUID()); wild != Wild.end())
            wild->second.WeakenedBy.clear();
    }
    void UpdateAI(uint32) override
    {
        {
            std::lock_guard<std::recursive_mutex> guard(Mutex);
            if (auto life = Wild.find(me->GetGUID()); life != Wild.end() && life->second.Claimed && me->IsAlive())
            {
                uint64 const now = GameTime::GetGameTime().count();
                // Recover a committed capture even if the previous world process
                // died before removing the creature. No DB query runs per tick.
                me->DespawnOrUnsummon(
                    0ms, Seconds(std::max<uint64>(1, life->second.ReadyAt > now ? life->second.ReadyAt - now : 1)));
                return;
            }
        }
        if (UpdateVictim())
            DoMeleeAttackIfReady();
    }
};
} // namespace Digimon

void AddDigimonTamerScripts()
{
    using namespace Digimon;
    new DigimonDatabaseScript();
    new DigimonWorldScript();
    new DigimonPlayerScript();
    new DigimonDamageScript();
    new DigimonSpellScript();
    new DigimonWelcome();
    new DigimonDevice();
    new DigimonCommand();
    RegisterCreatureAI(DigimonPreviewAI);
    RegisterCreatureAI(DigimonCompanionAI);
    RegisterCreatureAI(DigimonWildAI);
    RegisterCreatureAI(DigimonLabFollowerAI);
}
