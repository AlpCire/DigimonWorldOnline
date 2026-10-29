// SPDX-License-Identifier: GPL-2.0-or-later
#include "DigimonDatabase.h"
#include "DigimonPolicy.h"
#include "DigimonTalentTree.h"
#include "DigimonSpecialization.h"
#include "Chat.h"
#include "CharmInfo.h"
#include "Config.h"
#include "Creature.h"
#include "CryptoRandom.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "Formulas.h"
#include "GameTime.h"
#include "Item.h"
#include "Bag.h"
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
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"
#include "ThreatManager.h"
#include "Transaction.h"
#include "Util.h"
#include "World.h"
#include "WorldPacket.h"
#include <array>
#include <memory>
#include <functional>
#include <charconv>
#include <chrono>
#include <bit>
#include <cctype>
#include <map>
#include <mutex>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Digimon
{
#include "DigimonCombatProfile.h"

constexpr uint32 DeviceMenu = 900100;
constexpr uint32 LevelUpVisualKit = 6375; // Native Level Up visual, SpellVisualKit.dbc.
enum Sender : uint32
{
    Main = 1,
    ChooseStarter = 2,
    Buy = 3,
    Equip = 4,
    Unequip = 5,
    Use = 6,
    Activate = 7,
    CollectionPage = 8,
    ChooseEvolution = 9,
    MissionStart = 10,
    MissionClaim = 11
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
    Fight,
    EvolutionMenuAction,
    MissionMenuAction,
    StarterMenuAction
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
    uint8 Bond = 0;
    uint8 Happiness = 50;
    uint8 DamageBuffPct = 0;
    uint64 DamageBuffExpires = 0;
    uint8 ProtectionBuffPct = 0;
    uint64 ProtectionBuffExpires = 0;
    std::unordered_set<uint32> Learned;
    std::map<uint8, uint32> Equipped;
    std::unordered_set<uint32> Forms;
    uint8 PartySlot = 255;
    std::string Role = "DPS";
    SpecializationProfile Profile = SpecializationProfile::Basic;
    std::array<uint8, 28> Talents{};
};

struct Consumable
{
    std::string Kind;
    uint8 HappinessGain = 0;
    uint8 Magnitude = 0;
    uint16 Duration = 0;
    uint32 Spell = 0;
    bool Enabled = false;
};

struct Technique
{
    std::string Name;
    std::string LocalizedName;
    uint32 Spell;
    Stage Required;
    uint64 Price;
    uint8 RequiredLevel;
    uint32 CooldownMs;
    bool ControlLimited;
    bool Enabled;
    bool Special;
};

struct Tamer
{
    uint64 Bits = 0;
    uint32 Capacity = 30;
    uint16 StorageBoxes = 2;
    uint32 Active = 0;
    bool Summoned = false;
    bool Addon = false;
    std::string Behaviour = "defensive";
    uint64 CaptureReady = 0;
    uint32 NextOption = 1;
    ObjectGuid Actor;
    struct CaptureChallenge
    {
        ObjectGuid Target;
        std::string Nonce;
        std::string Attempt;
        uint8 ClickMask = 0;
        std::chrono::steady_clock::time_point Started;
        std::chrono::steady_clock::time_point LastClick;
    } Capture;
    std::vector<Companion> Companions;
    std::unordered_set<uint32> SentTechniqueNames;
};

std::recursive_mutex Mutex;
std::unique_ptr<MySQLConnectionInfo> DatabaseInfo;
std::unique_ptr<Connection> Database;
std::unordered_map<uint32, Species> Catalog;
std::unordered_map<uint32, Technique> TechniqueCatalog;
std::unordered_map<uint32, Consumable> ConsumableCatalog;
std::unordered_map<uint64, uint64> TechniqueCooldowns;
std::string const& TechniqueDisplayName(Technique const& technique)
{
    return technique.LocalizedName.empty() ? technique.Name : technique.LocalizedName;
}
std::unordered_map<std::string, uint32> Aliases;
std::unordered_map<uint32, std::unordered_set<uint32>> SpeciesTechniques;
struct Evolution
{
    uint32 Target;
    uint8 Level;
};
std::unordered_map<uint32, std::vector<Evolution>> Evolutions;
struct RouteRequirement
{
    std::string Affinity;
    uint16 Points;
    uint16 Victories;
    uint8 Catalysts;
    std::string Mission;
    uint32 CatalystItem = 0;
    uint32 MissionCreatureEntry = 0;
};
std::map<std::pair<uint32, uint32>, RouteRequirement> Requirements;
constexpr uint32 ImpmonQuestBase = 900501;
constexpr uint32 PagumonQuestBase = 900801;
constexpr uint32 PagumonQuestLast = 900804;
constexpr uint32 PagumonSpecies = 104;
constexpr uint32 ImpmonSpecies = 103;

bool IsPagumonQuest(uint32 questId)
{
    return questId >= PagumonQuestBase && questId <= PagumonQuestLast;
}

uint32 NativeRouteQuest(uint32 targetSpecies)
{
    if (targetSpecies == ImpmonSpecies)
        return PagumonQuestBase;
    return targetSpecies >= 105 && targetSpecies <= 117 ? ImpmonQuestBase + targetSpecies - 105 : 0;
}
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
void SendV2(Player* player, std::string const& message);
void CleanSpentConsumables(Player* player);

bool TechniqueAllowed(uint32 species, uint32 technique)
{
    if (IsTalentTechnique(technique))
        return true;
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
    for (uint8 step = 0; step < 16; ++step)
    {
        auto const rules = Evolutions.find(species);
        if (rules == Evolutions.end())
            break;
        uint32 selectedSpecies = 0;
        Stage selectedStage = stage;
        uint32 eligible = 0;
        for (Evolution const& rule : rules->second)
        {
            auto const target = Catalog.find(rule.Target);
            if (!Requirements.count({species, rule.Target}) && target != Catalog.end() && target->second.Enabled &&
                EvolutionReady(level, rule.Level, stage, target->second.DefaultStage))
            {
                ++eligible;
                selectedSpecies = target->first;
                selectedStage = target->second.DefaultStage;
            }
        }
        if (!HasUniqueEvolutionChoice(eligible) || !selectedSpecies)
            break;
        species = selectedSpecies;
        stage = selectedStage;
    }
    return {species, stage};
}

std::pair<uint32, Stage> ResolveDevolution(Companion const& companion, uint8 level)
{
    uint32 species = companion.SpeciesId;
    Stage stage = companion.Evolution;
    for (uint8 step = 0; step < 16; ++step)
    {
        uint32 previous = 0;
        for (auto const& [source, rules] : Evolutions)
        {
            auto const candidate = Catalog.find(source);
            if (candidate == Catalog.end() || !candidate->second.Enabled)
                continue;
            for (Evolution const& rule : rules)
            {
                if (rule.Target != species || !DevolutionReady(level, rule.Level, stage, candidate->second.DefaultStage))
                    continue;
                // Converging paths have no unique previous species; never pick one arbitrarily.
                if (previous && previous != source)
                    return {species, stage};
                previous = source;
            }
        }
        if (!previous)
            break;
        species = previous;
        stage = Catalog.at(species).DefaultStage;
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
    std::string const behaviour = Tamers[owner].Behaviour;
    Tamer state;
    state.Actor = actor;
    state.Addon = addon;
    state.Behaviour = behaviour;
    state.NextOption = nextOption;
    Field* fields = profile->Fetch();
    state.Bits = fields[0].Get<uint64>();
    state.StorageBoxes = fields[4].Get<uint16>();
    state.Capacity = 6 + uint32(state.StorageBoxes) * 15;
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
                                        fields[6].Get<uint8>(),
                                        fields[7].Get<uint8>(),
                                        fields[8].Get<uint8>(),
                                        fields[9].Get<uint64>(),
                                        fields[10].Get<uint8>(),
                                        fields[11].Get<uint64>(),
                                        {},
                                        {},
                                        {},
                                        fields[12].Get<uint8>(),
                                        fields[13].Get<std::string>(),
                                        ProfileFromCode(fields[14].Get<std::string>())});
        } while (companions->NextRow());
    if (auto points = Query(SelectTalentPoints, owner))
        do
        {
            fields = points->Fetch();
            uint8 const talent = fields[1].Get<uint8>();
            for (Companion& companion : state.Companions)
                if (TalentNode const* node = FindTalentNode(talent);
                    node && companion.Id == fields[0].Get<uint32>() &&
                    companion.Talents[talent] < node->MaxRank)
                    ++companion.Talents[talent];
        } while (points->NextRow());
    // Backfill techniques for companions that were granted or level-adjusted by
    // administration tools instead of the normal XP reward path.
    for (Companion const& companion : state.Companions)
        Execute(LearnEligibleTechniques, companion.Id);
    Execute(EnsureForms, owner);
    if (auto forms = Query(SelectDiscoveredForms, owner))
        do
        {
            fields = forms->Fetch();
            for (auto& companion : state.Companions)
                if (companion.Id == fields[0].Get<uint32>())
                    companion.Forms.insert(fields[1].Get<uint32>());
        } while (forms->NextRow());
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

void PlayLevelUpVisual(Creature* actor)
{
    // The core's level-up spell targets the player, so use the native visual
    // packet on the companion's GUID instead.
    if (actor)
        actor->SendPlaySpellVisual(LevelUpVisualKit);
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
                    TalentTechniqueUnlocked(companion->Talents, equipped->second) &&
                    companion->Evolution >= technique->second.Required && companion->Level >= technique->second.RequiredLevel &&
                    CanEquip(companion->Evolution, slot, technique->second.Special))
                {
                    spellId = technique->second.Spell;
                    cooldown = actor ? actor->GetSpellCooldown(spellId) : 0;
                    uint64 const key = (uint64(companion->Id) << 32) | equipped->second;
                    if (auto const ready = TechniqueCooldowns.find(key); ready != TechniqueCooldowns.end())
                    {
                        uint64 const now = GameTime::GetGameTime().count();
                        cooldown = ready->second > now ? uint32((ready->second - now) * IN_MILLISECONDS) : 0;
                    }
                    if (SpellInfo const* spell = sSpellMgr->GetSpellInfo(spellId))
                        cost = CombatProfiles.count(equipped->second) ? CombatProfiles.at(equipped->second).Cost :
                            actor ? spell->CalcPowerCost(actor, spell->GetSchoolMask()) : spell->ManaCost;
                    if (state.SentTechniqueNames.insert(equipped->second).second)
                        SendUI(player, "T|" + std::to_string(equipped->second) + "|" + TechniqueDisplayName(technique->second));
                }
        message += "|" + std::to_string(spellId) + "," + std::to_string(cost) + "," + std::to_string(cooldown);
        message += "," + std::to_string(companion && companion->Equipped.count(slot) ?
                                            companion->Equipped.at(slot) : 0);
    }
    SendUI(player, message);
    SendUI(player, "MODE|" + state.Behaviour);
    if (companion)
    {
        uint64 const required = companion->Level < 100 ? 100u + 75u * companion->Level : 0;
        SendUI(player, "XP|" + std::to_string(companion->Experience) + "|" + std::to_string(required));
        SendUI(player, "BOND|" + std::to_string(companion->Bond));
        uint64 const now = GameTime::GetGameTime().count();
        uint32 const damageRemaining = companion->DamageBuffExpires > now ?
            static_cast<uint32>(companion->DamageBuffExpires - now) : 0;
        uint32 const protectionRemaining = companion->ProtectionBuffExpires > now ?
            static_cast<uint32>(companion->ProtectionBuffExpires - now) : 0;
        SendUI(player, "CARE|" + std::to_string(companion->Happiness) + "|" +
                      std::to_string(damageRemaining ? companion->DamageBuffPct : 0) + "|" +
                      std::to_string(damageRemaining) + "|" +
                      std::to_string(protectionRemaining ? companion->ProtectionBuffPct : 0) + "|" +
                      std::to_string(protectionRemaining));
    }
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

void RefreshTamerSpeed(Player* player, Creature* actor, Companion const& companion)
{
    if (!player || !actor || !actor->IsAlive() || !player->IsInMap(actor))
        return;
    uint8 const bonus = TamerMovementBonus(companion.Talents[4]);
    if (!bonus)
    {
        player->RemoveAurasDueToSpell(23451, actor->GetGUID());
        return;
    }
    // Keep the same aura while the companion is present. Recasting it every
    // eight seconds resets its client timer and may restore the DBC's 100% value.
    if (AuraEffect* effect = player->GetAuraEffect(23451, EFFECT_0, actor->GetGUID()))
    {
        if (effect->GetAmount() != bonus)
            effect->ChangeAmount(bonus);
        return;
    }
    CustomSpellValues values;
    values.AddSpellMod(SPELLVALUE_BASE_POINT0, bonus);
    actor->CastCustomSpell(23451, values, player, TRIGGERED_FULL_MASK);
    // Spell 23451 has a 100% default effect in the client DBC. Force the
    // server-side aura amount after application so one rank is exactly 2%.
    if (AuraEffect* effect = player->GetAuraEffect(23451, EFFECT_0, actor->GetGUID()))
        effect->ChangeAmount(bonus);
}

void Despawn(Player* player, Tamer& state)
{
    PersistHealth(player, state);
    if (Creature* actor = Actor(player, state))
    {
        player->RemoveAurasDueToSpell(23451, actor->GetGUID());
        actor->DespawnOrUnsummon();
    }
    state.Actor.Clear();
}

void ApplyStats(Creature* actor, Companion const& companion)
{
    actor->SetLevel(companion.Level);
    // Companion durability and melee power grow at every level, including after
    // evolution; techniques also use their per-level combat profile scaling.
    uint32 const baseHealth = 60u + 30u * companion.Level;
    uint32 const health = baseHealth * (100u + (companion.Role == "TANK" ? 40u :
                                               companion.Role == "HEALER" ? 10u : 0u) +
                                          2u * companion.Talents[2]) / 100u;
    actor->SetCreateHealth(health);
    actor->SetMaxHealth(health);
    actor->SetHealth(std::max(1u, HealthFromPercent(health, companion.Health)));
    actor->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 2.0f + companion.Level * 1.8f);
    actor->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 4.0f + companion.Level * 2.4f);
    actor->UpdateDamagePhysical(BASE_ATTACK);
    actor->SetAttackTime(BASE_ATTACK, 1500u * (100u - 2u * companion.Talents[5]) / 100u);
    actor->setPowerType(POWER_FOCUS);
    uint32 const focus = 100u + 5u * (companion.Talents[15] + companion.Talents[20]);
    actor->SetMaxPower(POWER_FOCUS, focus);
    actor->SetPower(POWER_FOCUS, focus);
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
    RefreshTamerSpeed(player, actor, companion);
    return true;
}

bool CreditTraining(Companion const& companion, Creature const& defeated,
                    std::shared_ptr<Transaction<Connection>> const& transaction, uint32& creditedTarget);
void SendRoutes(Player* player, Tamer& state);
void SendTalents(Player* player, Tamer& state);
void SendDetails(Player* player, Tamer& state);
void NotifyMissionProgress(Player* player, Tamer& state, uint32 target);
void MissionMenu(Player* player, ObjectGuid source, Tamer& state);

void RewardCompanion(Creature* actor, Unit* victim)
{
    std::lock_guard<std::recursive_mutex> guard(Mutex);
    if (!Config.Enabled || !Managed(actor) || !victim || !victim->IsCreature())
        return;
    Creature* defeated = victim->ToCreature();
    Player* player = ObjectAccessor::FindPlayer(actor->GetOwnerGUID());
    if (!player || player->IsGameMaster() || defeated->IsSummon() || defeated->GetOwnerGUID() ||
        defeated->IsCritter() || defeated->GetLootRecipient() != player)
        return;
    Tamer& state = Tamers.at(OwnerId(player));
    Companion* companion = Active(state);
    if (!companion)
        return;
    auto transaction = std::make_shared<Transaction<Connection>>();
    uint32 missionTarget = 0;
    bool const missionCredited = CreditTraining(*companion, *defeated, transaction, missionTarget);
    bool const canEarnXP = !defeated->GetCreatureTemplate()->HasFlagsExtra(CREATURE_FLAG_EXTRA_NO_XP) &&
        defeated->GetCreatureTemplate()->ModExperience > 0 &&
        defeated->GetLevel() > Acore::XP::GetGrayLevel(companion->Level);
    if (!canEarnXP)
    {
        if (!missionCredited)
            return;
        transaction->Append(Make(CreditBond, companion->Id, uint8(1)));
        if (Database->ExecuteTransaction(transaction) != 0 || !Reload(player))
        {
            LOG_ERROR("module.digimon", "Cannot persist custom mission credit for owner {}", OwnerId(player));
            Notify(player, "No se pudo guardar el progreso del encargo. Consulta al administrador.");
            return;
        }
        NotifyMissionProgress(player, Tamers.at(OwnerId(player)), missionTarget);
        return;
    }
    // Calculate from the companion's level, outside human XP hooks and rested XP.
    uint32 const base = Acore::XP::BaseGain(companion->Level, defeated->GetLevel(),
                                            GetContentLevelsForMapAndZone(defeated->GetMapId(), defeated->GetZoneId()));
    double const modifier =
        Config.CompanionXP * defeated->GetCreatureTemplate()->ModExperience * (defeated->isElite() ? 2.0 : 1.0);
    uint32 const amount = ScaleXP(base, modifier);
    transaction->Append(Make(CreditBond, companion->Id, uint8(1)));
    Progress const progress = GainExperience({companion->Level, companion->Experience}, amount);
    bool const levelled = progress.Level > companion->Level;
    auto const form = levelled ? ResolveEvolution(*companion, progress.Level) :
                                std::make_pair(companion->SpeciesId, companion->Evolution);
    bool const evolved = form.first != companion->SpeciesId;
    transaction->Append(
        Make(SaveProgress, form.first, progress.Level, progress.Experience, actor->GetHealthPct(), uint8(form.second),
             OwnerId(player), companion->Id));
    transaction->Append(Make(LearnEligibleTechniques, companion->Id));
    if (evolved)
    {
        if (!companion->Forms.count(form.first))
            transaction->Append(Make(CloneFormEquipment, companion->SpeciesId, OwnerId(player), companion->Id));
        transaction->Append(Make(UnlockForm, companion->Id, form.first));
        transaction->Append(Make(RecordEvolutionHistory, companion->Id, companion->SpeciesId, form.first,
                                 progress.Level, "EVOLVE"));
        transaction->Append(Make(PruneEquipment, OwnerId(player), companion->Id));
    }
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
            PlayLevelUpVisual(actor);
            Notify(player, "Tu compañero ha alcanzado el nivel " + std::to_string(updated->Level) + ".");
            if (evolved)
                Notify(player, "¡Digievolución! Tu compañero ahora es " + Catalog.at(updated->SpeciesId).Name + ".");
            SendTalents(player, Tamers.at(OwnerId(player)));
            SendDetails(player, Tamers.at(OwnerId(player)));
            SendCombatState(player, Tamers.at(OwnerId(player)));
        }
    if (missionCredited)
        NotifyMissionProgress(player, Tamers.at(OwnerId(player)), missionTarget);
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
    RefreshTamerSpeed(player, actor, *companion);
    return true;
}

void GiveDevice(Player* player)
{
    if (!player->HasItemCount(Config.Device, 1, true) && !player->AddItem(Config.Device, 1))
        Notify(player, "Libera un hueco para el Dispositivo Sagrado. Puedes usar .digimon mientras tanto.");
}

void SendStarterOptions(Player* player)
{
    if (!player->HasItemCount(Config.Device, 1, true))
        return;
    SendUI(player, "STARTERS_BEGIN");
    for (auto const& [id, species] : Catalog)
    {
        if (!species.Starter || !species.Enabled)
            continue;
        CreatureTemplate const* creature = sObjectMgr->GetCreatureTemplate(species.Entry);
        CreatureModel const* model = creature ? sObjectMgr->ChooseDisplayId(creature) : nullptr;
        if (!model)
            continue;
        std::string nextName = "Sin ruta registrada";
        uint32 nextLevel = 0;
        if (auto routes = Evolutions.find(id); routes != Evolutions.end())
            for (Evolution const& route : routes->second)
                if (Catalog.count(route.Target) && Catalog.at(route.Target).Enabled)
                {
                    nextName = Catalog.at(route.Target).Name;
                    nextLevel = route.Level;
                    break;
                }
        SendUI(player, "STARTER|" + std::to_string(id) + "|" + species.Name + "|" +
                       std::to_string(model->CreatureDisplayID) + "|" + nextName + "|" +
                       std::to_string(nextLevel));
    }
    SendUI(player, "STARTERS_END");
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
        companion.Evolution < technique->second.Required || companion.Level < technique->second.RequiredLevel ||
        !companion.Learned.count(id) ||
        !TechniqueAllowed(companion.SpeciesId, id) ||
        !TalentTechniqueUnlocked(companion.Talents, id))
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
            companion.Evolution >= technique->second.Required && companion.Level >= technique->second.RequiredLevel &&
            TechniqueAllowed(companion.SpeciesId, id) &&
            TalentTechniqueUnlocked(companion.Talents, id))
            AddOption(player, GOSSIP_ICON_CHAT,
                      (IsEquipped(companion, id) ? "Retirar técnica: " : "Equipar técnica: ") + TechniqueDisplayName(technique->second),
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
    if (!technique.Enabled || companion->Evolution < technique.Required || companion->Level < technique.RequiredLevel || companion->Learned.count(id) ||
        state.Bits < technique.Price || !TechniqueAllowed(companion->SpeciesId, id) ||
        !TalentTechniqueUnlocked(companion->Talents, id))
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
    if (!technique.Enabled || companion->Evolution < technique.Required || companion->Level < technique.RequiredLevel || IsEquipped(*companion, id) ||
        !TechniqueAllowed(companion->SpeciesId, id) ||
        !TalentTechniqueUnlocked(companion->Talents, id))
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

#include "DigimonCombat.h"

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
        companion->Evolution < found->second.Required || companion->Level < found->second.RequiredLevel)
        return;
    if (auto profile = CombatProfiles.find(id); profile != CombatProfiles.end())
    {
        UseProfile(player, state, *companion, actor, id, profile->second);
        return;
    }
    Technique const& technique = found->second;
    SpellInfo const* spell = sSpellMgr->GetSpellInfo(technique.Spell);
    if (!spell)
        return;
    Unit* target = spell->IsPositive() ? actor : player->GetSelectedUnit();
    if (id == 2002)
    {
        Unit* selected = player->GetSelectedUnit();
        if (selected && Managed(selected))
            if (Player* recipient = ObjectAccessor::FindPlayer(selected->GetOwnerGUID());
                recipient && (recipient == player || player->IsInSameGroupWith(recipient)))
                target = selected;
        if (selected && selected->IsPlayer())
            if (Player* recipient = selected->ToPlayer();
                recipient == player || player->IsInSameGroupWith(recipient))
                if (auto member = Tamers.find(OwnerId(recipient)); member != Tamers.end())
                    if (Creature* alliedActor = Actor(recipient, member->second))
                        target = alliedActor;
    }
    uint64 const cooldownKey = (uint64(companion->Id) << 32) | id;
    uint64 const now = GameTime::GetGameTime().count();
    if (auto const cooldown = TechniqueCooldowns.find(cooldownKey); cooldown != TechniqueCooldowns.end() &&
        cooldown->second > now)
    {
        Notify(player, "La técnica todavía está recuperándose.");
        return;
    }
    if (!target || !target->IsAlive() ||
        (!spell->IsPositive() && (!target->IsCreature() || !actor->IsValidAttackTarget(target) ||
                                 !player->IsValidAttackTarget(target) ||
                                 !player->IsWithinDistInMap(target, 40.0f) || !player->IsWithinLOSInMap(target))))
    {
        Notify(player, "Selecciona una criatura enemiga cercana y a la vista para utilizar esta técnica.");
        return;
    }
    if (id == 2002)
    {
        if (!Managed(target) || !actor->IsWithinDistInMap(target, 30.0f) ||
            !actor->IsWithinLOSInMap(target) || actor->GetPower(POWER_FOCUS) < 30)
        {
            Notify(player, "El Digimon aliado debe estar cerca y el sanador necesita 30 de energía.");
            return;
        }
        int32 const base = 30 + 8 * companion->Level;
        int32 const amount = base;
        CustomSpellValues values;
        values.AddSpellMod(SPELLVALUE_BASE_POINT0, amount);
        if (actor->CastCustomSpell(technique.Spell, values, target, TRIGGERED_FULL_MASK) == SPELL_CAST_OK)
        {
            actor->ModifyPower(POWER_FOCUS, -30);
            uint32 const adjusted = technique.CooldownMs * (100u - 3u * companion->Talents[23]) / 100u;
            TechniqueCooldowns[cooldownKey] = now + (adjusted + 999) / 1000;
            SendUI(player, "CAST|" + std::to_string(technique.Spell));
        }
        SendCombatState(player, state);
        return;
    }
    if (IsTalentTechnique(id))
    {
        if (id == 2004 || id == 2005)
        {
            Unit* selected = player->GetSelectedUnit();
            if (selected && Managed(selected))
                if (Player* recipient = ObjectAccessor::FindPlayer(selected->GetOwnerGUID());
                    recipient && (recipient == player || player->IsInSameGroupWith(recipient)))
                    target = selected;
            if (selected && selected->IsPlayer())
                if (Player* recipient = selected->ToPlayer();
                    recipient == player || player->IsInSameGroupWith(recipient))
                    if (auto member = Tamers.find(OwnerId(recipient)); member != Tamers.end())
                        if (Creature* alliedActor = Actor(recipient, member->second))
                            target = alliedActor;
            if (!Managed(target))
                target = actor;
        }
        else
            target = player->GetSelectedUnit();
        uint32 const cost = id == 2001 ? 20u : id == 2000 ? 25u :
                            id == 2003 ? 40u : id == 2004 ? 30u : 45u;
        bool const friendly = id == 2004 || id == 2005;
        bool const valid = friendly ? target && Managed(target) && target->IsAlive() &&
            actor->IsWithinDistInMap(target, 30.0f) && actor->IsWithinLOSInMap(target) :
            ValidCombatTarget(player, actor, target);
        if (!valid || actor->GetPower(POWER_FOCUS) < cost)
        {
            Notify(player, "Objetivo no válido, fuera de alcance o falta energía.");
            return;
        }
        int32 const base = id == 2000 ? 30 + 9 * companion->Level :
                           id == 2003 ? 24 + 7 * companion->Level :
                           id == 2004 ? 50 + 10 * companion->Level :
                           id == 2005 ? 65 + 15 * companion->Level : 0;
        CustomSpellValues values;
        if (base)
            values.AddSpellMod(SPELLVALUE_BASE_POINT0, base);
        if (!friendly)
        {
            actor->AI()->AttackStart(target);
            actor->SetFacingToObject(target);
        }
        if (actor->CastCustomSpell(technique.Spell, values, target, TRIGGERED_FULL_MASK) == SPELL_CAST_OK)
        {
            actor->ModifyPower(POWER_FOCUS, -int32(cost));
            uint32 cooldown = technique.CooldownMs;
            if (id == 2005)
                cooldown = cooldown * (100u - 3u * companion->Talents[23]) / 100u;
            TechniqueCooldowns[cooldownKey] = now + (cooldown + 999) / 1000;
            SendUI(player, "CAST|" + std::to_string(technique.Spell));
        }
        SendCombatState(player, state);
        return;
    }
    if (technique.ControlLimited &&
        (target->IsPlayer() || (target->IsCreature() &&
                                target->ToCreature()->HasFlagsExtra(CREATURE_FLAG_EXTRA_DUNGEON_BOSS))))
    {
        Notify(player, "El control de esta técnica no afecta a jugadores ni a jefes.");
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
        if (technique.CooldownMs)
            TechniqueCooldowns[cooldownKey] = now + (technique.CooldownMs + 999) / 1000;
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
        if (species.Enabled && species.CaptureEntry == target->GetEntry())
            speciesId = id;
    auto const partyCount = std::count_if(state.Companions.begin(), state.Companions.end(),
                                          [](Companion const& c) { return c.PartySlot < 6; });
    uint32 stored = 0;
    if (auto result = Query(CountStorage, owner))
        stored = result->Fetch()[0].Get<uint32>();
    if (!speciesId || (partyCount >= 6 && stored >= uint32(state.StorageBoxes) * 15))
    {
        Notify(player, "No es capturable o tu Digiequipo y Digialmacén están llenos.");
        return;
    }
    uint64 const now = GameTime::GetGameTime().count();
    if (now < state.CaptureReady)
    {
        Notify(player,
               "El dispositivo está recuperándose. Espera " + std::to_string(state.CaptureReady - now) + " segundos.");
        return;
    }
    std::string const attempt = NewToken();
    auto transaction = std::make_shared<Transaction<Connection>>();
    transaction->Append(Make(BeginCaptureAttempt, now + Config.CaptureCooldown, attempt, owner, now));
    if (Database->ExecuteTransaction(transaction) != 0 || !Reload(player))
    {
        Notify(player, "No se pudo iniciar la captura. El salvaje sigue en el mundo.");
        return;
    }
    Tamer& refreshed = Tamers.at(owner);
    refreshed.Capture.Target = target->GetGUID();
    refreshed.Capture.Nonce = NewToken();
    refreshed.Capture.Attempt = attempt;
    refreshed.Capture.ClickMask = 0;
    refreshed.Capture.Started = std::chrono::steady_clock::now();
    refreshed.Capture.LastClick = refreshed.Capture.Started;
    refreshed.Addon = true;
    SendV2(player, "CAPTURE_BEGIN|" + refreshed.Capture.Nonce + "|8|" + std::to_string(speciesId));
    Notify(player, "¡Pulsa los cinco nodos del Dispositivo antes de que termine el tiempo!");
}

bool CaptureTargetStillValid(Player* player, Tamer& state, Creature* target)
{
    if (!target)
        return false;
    auto wild = Wild.find(target->GetGUID());
    uint32 const owner = OwnerId(player);
    if (wild == Wild.end() || wild->second.Claimed || !target->IsAlive() || target->IsSummon() || target->GetOwnerGUID() ||
        target->GetCharmerGUID() || target->GetCreatureTemplate()->rank != CREATURE_ELITE_NORMAL ||
        target->HasFlagsExtra(CREATURE_FLAG_EXTRA_DUNGEON_BOSS) || ProtectedQuestEntries.count(target->GetEntry()) ||
        !player->IsWithinDistInMap(target, 15.0f) || !player->IsWithinLOSInMap(target) ||
        target->GetHealthPct() > Config.CaptureHealth || target->GetLootRecipient() != player)
        return false;
    auto damage = wild->second.WeakenedBy.find(owner);
    return damage != wild->second.WeakenedBy.end() && damage->second == state.Actor;
}

void CaptureStep(Player* player, Tamer& state, std::string_view nonce, uint32 node)
{
    auto fail = [&]()
    {
        state.Capture = {};
        SendV2(player, "CAPTURE_END|FAIL");
        Notify(player, "La sincronización falló. El Digimon salvaje sigue en el mundo.");
    };
    auto& challenge = state.Capture;
    auto const nowSteady = std::chrono::steady_clock::now();
    if (challenge.Nonce.empty() || nonce != challenge.Nonce || node < 1 || node > 5 ||
        nowSteady - challenge.Started > std::chrono::seconds(8) ||
        (challenge.ClickMask & (1u << (node - 1))) ||
        nowSteady - challenge.LastClick < std::chrono::milliseconds(65))
    {
        fail();
        return;
    }
    Creature* target = player->GetMap()->GetCreature(challenge.Target);
    if (!CaptureTargetStillValid(player, state, target))
    {
        fail();
        return;
    }
    challenge.ClickMask |= uint8(1u << (node - 1));
    challenge.LastClick = nowSteady;
    uint8 const completed = std::popcount(challenge.ClickMask);
    SendV2(player, "CAPTURE_PROGRESS|" + std::to_string(completed) + "|5");
    if (completed < 5)
        return;

    auto wild = Wild.find(target->GetGUID());
    if (wild == Wild.end())
    {
        fail();
        return;
    }
    uint32 const owner = OwnerId(player);
    uint64 const now = GameTime::GetGameTime().count();
    std::string const captureKey = wild->second.Token;
    std::string const attempt = challenge.Attempt;
    uint32 speciesId = 0;
    for (auto const& [id, species] : Catalog)
        if (species.Enabled && species.CaptureEntry == target->GetEntry())
            speciesId = id;
    if (!speciesId)
    {
        fail();
        return;
    }
    auto transaction = std::make_shared<Transaction<Connection>>();
    transaction->Append(Make(InsertCaptured, target->GetLevel(), target->GetHealthPct(), speciesId,
                             captureKey, owner, attempt, target->GetEntry()));
    transaction->Append(Make(AssignCapturedParty, captureKey));
    transaction->Append(Make(ClaimCapturedWild, now + std::max(30u, target->GetRespawnDelay()), captureKey, owner));
    if (Database->ExecuteTransaction(transaction) != 0 || !Reload(player))
    {
        fail();
        return;
    }
    auto captured = Query(SelectCaptured, captureKey);
    if (!captured || captured->Fetch()[1].Get<uint32>() != owner)
    {
        fail();
        return;
    }
    wild->second.Claimed = true;
    wild->second.ReadyAt = now + std::max(30u, target->GetRespawnDelay());
    Tamers.at(owner).Capture = {};
    Creature* actor = Actor(player, Tamers.at(owner));
    if (actor && actor->GetVictim() == target)
    {
        actor->AttackStop();
        actor->GetThreatMgr().ClearAllThreat();
        actor->GetMotionMaster()->MoveFollow(player, 2.0f, 0.8f);
    }
    // The durable collection row exists before the wild creature disappears.
    target->DespawnOrUnsummon();
    SendV2(player, "CAPTURE_END|SUCCESS");
    Notify(player, "¡Captura completada! Se guardó en tu Digiequipo o en el Digialmacén.");
}

void ActivateCompanion(Player* player, Tamer& state, uint32 id)
{
    if (id == state.Active || Busy(player, state) || player->IsInFlight() || player->GetVehicle())
        return;
    bool owned = false;
    for (auto const& companion : state.Companions)
        if (companion.Id == id)
        {
            owned = companion.PartySlot < 6;
            break;
        }
    if (!owned)
    {
        Notify(player, "Ese Digimon está en el Digialmacén. Pásalo al Digiequipo antes de invocarlo.");
        return;
    }
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
    uint32 const party = std::count_if(state.Companions.begin(), state.Companions.end(),
                                       [](Companion const& c) { return c.PartySlot < 6; });
    uint32 stored = 0;
    if (auto result = Query(CountStorage, OwnerId(player)))
        stored = result->Fetch()[0].Get<uint32>();
    Notify(player, "Digiequipo: " + std::to_string(party) + "/6. Digialmacén: " + std::to_string(stored) + "/" +
                       std::to_string(uint32(state.StorageBoxes) * 15) + ". Compañero invocado: 1. DigiBits: " + std::to_string(state.Bits) +
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
    std::vector<Companion const*> partyMembers;
    for (Companion const& c : state.Companions)
        if (c.PartySlot < 6)
            partyMembers.push_back(&c);
    std::size_t const begin = offset < partyMembers.size() ? offset : 0;
    std::size_t const end = std::min(begin + 20, partyMembers.size());
    for (std::size_t index = begin; index < end; ++index)
    {
        Companion const& companion = *partyMembers[index];
        if (auto species = Catalog.find(companion.SpeciesId); species != Catalog.end())
            AddOption(player, GOSSIP_ICON_CHAT,
                      (companion.Id == state.Active ? "[Activo] " : "Activar ") + species->second.Name + " #" +
                          std::to_string(companion.Id) + " - nivel " + std::to_string(companion.Level) + ", salud " +
                          std::to_string(uint32(companion.Health)) + "%",
                      Activate, companion.Id);
    }
    if (begin >= 20)
        AddOption(player, GOSSIP_ICON_CHAT, "Compañeros anteriores", CollectionPage, static_cast<uint32>(begin - 20));
    if (end < partyMembers.size())
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
    if (!state.Active && state.Companions.empty() && player->HasItemCount(Config.Device, 1, true))
        AddOption(player, GOSSIP_ICON_CHAT, "Elegir mi primer Digimon", Main, StarterMenuAction);
    else if (!state.Active && state.Companions.empty() && welcome && !player->HasItemCount(Config.Device, 1, true))
        AddOption(player, GOSSIP_ICON_CHAT, "La elección se desbloquea al completar la historia de llegada", Main, Back);
    if (state.Active)
    {
        AddOption(player, GOSSIP_ICON_BATTLE, "Luchar - mostrar barra de combate", Main, Fight);
        AddOption(player, GOSSIP_ICON_CHAT, "Invocar compañero", Main, Invoke);
        AddOption(player, GOSSIP_ICON_CHAT, "Retirar compañero", Main, Withdraw);
        AddOption(player, GOSSIP_ICON_CHAT, "Atacar mi objetivo", Main, Attack);
        AddOption(player, GOSSIP_ICON_CHAT, "Dejar de atacar y seguirme", Main, Follow);
        AddOption(player, GOSSIP_ICON_CHAT, "Descansar y recuperar al compañero (fuera de combate)", Main, Recover);
        AddOption(player, GOSSIP_ICON_CHAT, "Gestionar técnicas aprendidas y equipadas", Main, Techniques);
        AddOption(player, GOSSIP_ICON_CHAT, "Capturar el salvaje seleccionado", Main, Capture);
        AddOption(player, GOSSIP_ICON_CHAT, "Consultar y elegir digievolución", Main, EvolutionMenuAction);
        if (AtTrainer(player, source))
            if (Companion* active = Active(state))
                AddOption(player, GOSSIP_ICON_TRAINER,
                          "Encargos de evolución para " + Catalog.at(active->SpeciesId).Name,
                          Main, MissionMenuAction);
        if (Companion* companion = Active(state))
            for (auto const& [id, technique] : TechniqueCatalog)
            {
                if (IsEquipped(*companion, id))
                    AddOption(player, GOSSIP_ICON_CHAT, "Usar: " + TechniqueDisplayName(technique), Use, id);
                if (AtTrainer(player, source) && technique.Enabled && TechniqueAllowed(companion->SpeciesId, id) &&
                    companion->Evolution >= technique.Required && companion->Level >= technique.RequiredLevel &&
                    !companion->Learned.count(id))
                    AddOption(player, GOSSIP_ICON_TRAINER,
                              "Aprender " + TechniqueDisplayName(technique) + " (" + std::to_string(technique.Price) + " DigiBits)", Buy,
                              id);
            }
    }
    AddOption(player, GOSSIP_ICON_CHAT, "Consultar colección y espacios", Main, Collection);
    // Keep AzerothCore's native quest offers and turn-ins alongside the device menu.
    player->PrepareQuestMenu(source);
    SendGossipMenuFor(player, 1, source);
}

void SendEvolutionOptions(Player* player, Tamer const& state)
{
    SendUI(player, "EVO_BEGIN");
    auto const active = std::find_if(state.Companions.begin(), state.Companions.end(),
                                     [&state](Companion const& companion) { return companion.Id == state.Active; });
    if (active != state.Companions.end())
    {
        auto const routes = Evolutions.find(active->SpeciesId);
        if (routes != Evolutions.end())
            for (Evolution const& route : routes->second)
                if (Catalog.count(route.Target) && Catalog.at(route.Target).Enabled && !active->Forms.count(route.Target) &&
                    EvolutionReady(active->Level, route.Level, active->Evolution, Catalog.at(route.Target).DefaultStage))
                    SendUI(player, "EVO|" + std::to_string(route.Target) + "|" + std::to_string(route.Level) + "|" +
                                   Catalog.at(route.Target).Name + "|EVOLVE");
        for (uint32 species : active->Forms)
            if (species != active->SpeciesId && Catalog.count(species) && Catalog.at(species).Enabled)
                SendUI(player, "EVO|" + std::to_string(species) + "|" + std::to_string(active->Level) + "|" +
                               Catalog.at(species).Name + "|REUSE");
    }
    SendUI(player, "EVO_END");
}

#include "DigimonProgression.h"

void MissionMenu(Player* player, ObjectGuid source, Tamer& state)
{
    Companion* companion = Active(state);
    if (!AtTrainer(player, source) || !companion)
    {
        Notify(player, "Habla con el Guía del Mundo Digital y lleva activo el Digimon correspondiente.");
        return;
    }
    ClearGossipMenuFor(player);
    player->PlayerTalkClass->GetGossipMenu().SetMenuId(DeviceMenu);
    auto routes = Evolutions.find(companion->SpeciesId);
    uint32 count = 0;
    if (routes != Evolutions.end())
        for (Evolution const& route : routes->second)
        {
            if (!Catalog.count(route.Target) || !Catalog.at(route.Target).Enabled || companion->Forms.count(route.Target))
                continue;
            auto requirement = Requirements.find({companion->SpeciesId, route.Target});
            if (requirement == Requirements.end())
                continue;
            RouteRequirement const& req = requirement->second;
            TrialProgress const progress = ReadTrial(*companion, route.Target, req);
            uint32 const questId = NativeRouteQuest(route.Target);
            Quest const* quest = questId ? sObjectMgr->GetQuestTemplate(questId) : nullptr;
            QuestStatus const questStatus = quest ? player->GetQuestStatus(questId) : QUEST_STATUS_NONE;
            if (companion->SpeciesId == PagumonSpecies && route.Target == ImpmonSpecies)
            {
                bool chainStarted = false;
                for (uint32 id = PagumonQuestBase; id <= PagumonQuestLast; ++id)
                    chainStarted = chainStarted || player->GetQuestStatus(id) != QUEST_STATUS_NONE;
                if (progress.Completed)
                    AddOption(player, GOSSIP_ICON_CHAT, "Ruta registrada: Pagumon puede digievolucionar a Impmon al nivel 10", Main, Back);
                else if (questStatus == QUEST_STATUS_COMPLETE)
                    AddOption(player, GOSSIP_ICON_CHAT, "Entrega la misión actual al Digimon que te la encargó", Main, Back);
                else if (chainStarted)
                    AddOption(player, GOSSIP_ICON_CHAT, "Continúa la cadena de Pagumon con Patamon, Impmon y Wizardmon", Main, Back);
                else
                    AddOption(player, GOSSIP_ICON_TRAINER, "Comenzar cadena evolutiva: Una señal propia", MissionStart, route.Target);
                ++count;
                continue;
            }
            CreatureTemplate const* objectiveTemplate = req.MissionCreatureEntry ?
                sObjectMgr->GetCreatureTemplate(req.MissionCreatureEntry) : nullptr;
            std::string targetName = objectiveTemplate ? objectiveTemplate->Name : "objetivos";
            if (req.MissionCreatureEntry == 705)
                targetName = "Lobos jóvenes";
            std::string objective = req.MissionCreatureEntry ?
                ("Derrota " + std::to_string(req.Victories) + " " +
                 targetName + " (" +
                 std::to_string(progress.Victories) + "/" + std::to_string(req.Victories) + ")") :
                (req.Mission + " - victorias " + std::to_string(progress.Victories) + "/" +
                 std::to_string(req.Victories));
            if (questStatus == QUEST_STATUS_COMPLETE)
                AddOption(player, GOSSIP_ICON_TRAINER, "Entregar misión nativa: " + req.Mission + " - " +
                          Catalog.at(route.Target).Name, MissionClaim, route.Target);
            else if (questStatus == QUEST_STATUS_INCOMPLETE)
                AddOption(player, GOSSIP_ICON_CHAT, "En curso: " + objective + " - " +
                          Catalog.at(route.Target).Name, MissionStart, route.Target);
            else if (progress.Completed && req.Catalysts && progress.Catalysts)
                AddOption(player, GOSSIP_ICON_TRAINER, "Reclamar recompensa: " + req.Mission + " - " +
                          Catalog.at(route.Target).Name, MissionClaim, route.Target);
            else if (progress.Completed)
                AddOption(player, GOSSIP_ICON_CHAT, "Encargo entregado: " + req.Mission + " - " +
                          Catalog.at(route.Target).Name, Main, Back);
            else
                AddOption(player, GOSSIP_ICON_TRAINER, "Aceptar: " + objective +
                          " - " + Catalog.at(route.Target).Name, MissionStart, route.Target);
            ++count;
        }
    if (!count)
        AddOption(player, GOSSIP_ICON_CHAT, "No hay encargos para tu Digimon activo", Main, Back);
    AddOption(player, GOSSIP_ICON_CHAT, "Volver", Main, Back);
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
    else if (sender == MissionStart || sender == MissionClaim)
    {
        if (!AtTrainer(player, source))
            return;
        StartTraining(player, source, state, action);
        return;
    }
    else if (sender == Equip || sender == Unequip)
        SetTechnique(player, state, action, sender == Unequip);
    else if (sender == Use)
        UseTechnique(player, state, action);
    else if (sender == Activate)
        ActivateCompanion(player, state, action);
    else if (sender == ChooseEvolution)
    {
        Companion* companion = Active(state);
        if (!companion || Busy(player, state))
            return;
        uint32 const current = companion->SpeciesId;
        Stage const currentStage = companion->Evolution;
        uint32 const targetId = action;
        auto target = Catalog.find(targetId);
        if (target == Catalog.end() || !target->second.Enabled)
        {
            Notify(player, "Esa forma no está disponible en el servidor.");
            return;
        }
        bool valid = companion->Forms.count(targetId) != 0;
        char const* historyAction = valid ? "REUSE" : "EVOLVE";
        if (!valid && target->second.DefaultStage >= currentStage)
        {
            auto routes = Evolutions.find(current);
            if (routes != Evolutions.end())
                for (Evolution const& route : routes->second)
                    if (route.Target == targetId && companion->Level >= route.Level &&
                        EvolutionReady(companion->Level, route.Level, currentStage, target->second.DefaultStage))
                        valid = true;
        }
        else if (!valid && target->second.DefaultStage < currentStage)
        {
            auto routes = Evolutions.find(targetId);
            if (routes != Evolutions.end())
                for (Evolution const& route : routes->second)
                    if (route.Target == current && companion->Level < route.Level &&
                        DevolutionReady(companion->Level, route.Level, currentStage, target->second.DefaultStage))
                        valid = true;
            historyAction = "DEVOLVE";
        }
        if (valid && std::string_view(historyAction) == "EVOLVE")
            valid = TrialReady(player, *companion, targetId);
        if (!valid)
        {
            Notify(player, "La ruta no cumple nivel, etapa o requisitos actuales.");
            return;
        }
        Creature* actor = Actor(player, state);
        float const health = actor ? actor->GetHealthPct() : companion->Health;
        auto transaction = std::make_shared<Transaction<Connection>>();
        transaction->Append(Make(SaveProgress, targetId, companion->Level, companion->Experience, health,
                                 uint8(target->second.DefaultStage), owner, companion->Id));
        Item* catalyst = nullptr;
        if (std::string_view(historyAction) == "EVOLVE")
            if (auto req = Requirements.find({current, targetId}); req != Requirements.end() && req->second.Catalysts)
            {
                catalyst = FindCatalyst(player, req->second.CatalystItem);
                if (!catalyst)
                    return;
                transaction->Append(Make(SpendPhysicalCatalyst, catalyst->GetGUID().GetCounter(), owner, companion->Id, targetId));
            }
        transaction->Append(Make(UnlockForm, companion->Id, targetId));
        transaction->Append(Make(LearnEligibleTechniques, companion->Id));
        if (!companion->Forms.count(targetId))
            transaction->Append(Make(CloneFormEquipment, current, owner, companion->Id));
        transaction->Append(Make(RecordEvolutionHistory, companion->Id, current, targetId, companion->Level,
                                 historyAction));
        transaction->Append(Make(PruneEquipment, owner, companion->Id));
        if (Database->ExecuteTransaction(transaction) != 0 || !Reload(player))
        {
            Notify(player, "No se pudo guardar el cambio de forma.");
            return;
        }
        if (catalyst)
            player->DestroyItem(catalyst->GetBagSlot(), catalyst->GetSlot(), true);
        if (actor)
            if (Companion* updated = Active(Tamers.at(owner)); !updated || !ApplyForm(player, actor, *updated))
                LOG_ERROR("module.digimon", "Cannot apply selected form for owner {}", owner);
        std::string const notice = std::string_view(historyAction) == "EVOLVE" ? "¡Digievolución! " :
                                   std::string_view(historyAction) == "DEVOLVE" ? "¡De-digievolución! " :
                                   "Forma restaurada: ";
        Notify(player, notice + "Ahora es " + target->second.Name + ".");
        SendTalents(player, Tamers.at(owner));
        SendDetails(player, Tamers.at(owner));
        SendCombatState(player, Tamers.at(owner));
    }
    else if (sender == CollectionPage)
    {
        CollectionMenu(player, source, state, action);
        return;
    }
    else if (sender == ChooseStarter)
    {
        if (!welcome || Busy(player, state) || state.Active || !state.Companions.empty() || Query(SelectStarter, owner) || !Catalog.count(action) ||
            !player->HasItemCount(Config.Device, 1, true) || !Catalog.at(action).Starter || !Catalog.at(action).Enabled)
            return;
        auto transaction = std::make_shared<Transaction<Connection>>();
        transaction->Append(Make(InsertStarter, owner, owner, action));
        transaction->Append(Make(InsertInitialActive, owner));
        transaction->Append(Make(SetTeamLead, owner));
        transaction->Append(Make(SetPartyLead, owner));
        if (Database->ExecuteTransaction(transaction) != 0 || !Reload(player))
        {
            Notify(player, "No se pudo guardar la selección. No se ha creado ningún compañero temporal.");
            return;
        }
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
            if (state.Behaviour == "passive")
            {
                Notify(player, "El compañero está en modo Pasivo. Cambia la conducta desde el Digivice para atacar.");
                break;
            }
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
                Companion* beforeCare = Active(state);
                uint8 const bondBeforeCare = beforeCare ? beforeCare->Bond : 0;
                auto care = std::make_shared<Transaction<Connection>>();
                care->Append(Make(CreditCareBond, state.Active));
                if (Database->ExecuteTransaction(care) != 0)
                {
                    Notify(player, "No se pudo guardar el cuidado del compañero.");
                    break;
                }
                Despawn(player, state);
                if (Execute(SaveHealth, 100.0f, owner, state.Active))
                {
                    Reload(player);
                    Summon(player, Tamers.at(owner));
                    Companion* cared = Active(Tamers.at(owner));
                    Notify(player, cared && cared->Bond > bondBeforeCare ?
                        "Has cuidado de tu compañero: Empatía +1." :
                        "Compañero recuperado. El cuidado vuelve a dar Empatía tras 24 horas.");
                }
            }
            break;
        case Collection:
            CollectionMenu(player, source, state);
            return;
        case EvolutionMenuAction: {
            Companion* companion = Active(state);
            if (!companion)
                break;
            ClearGossipMenuFor(player);
            player->PlayerTalkClass->GetGossipMenu().SetMenuId(DeviceMenu);
            uint32 count = 0;
            auto routes = Evolutions.find(companion->SpeciesId);
            if (routes != Evolutions.end())
                for (Evolution const& route : routes->second)
                    if (Catalog.count(route.Target) && Catalog.at(route.Target).Enabled && !companion->Forms.count(route.Target) &&
                        EvolutionReady(companion->Level, route.Level, companion->Evolution,
                                       Catalog.at(route.Target).DefaultStage))
                    {
                        AddOption(player, GOSSIP_ICON_CHAT,
                                  "Digievolucionar a " + Catalog.at(route.Target).Name + " (nivel " +
                                      std::to_string(route.Level) + ")", ChooseEvolution, route.Target);
                        ++count;
                    }
            for (uint32 unlockedSpecies : companion->Forms)
                if (unlockedSpecies != companion->SpeciesId && Catalog.count(unlockedSpecies) &&
                    Catalog.at(unlockedSpecies).Enabled)
                {
                    AddOption(player, GOSSIP_ICON_CHAT,
                              "Volver a una forma descubierta: " + Catalog.at(unlockedSpecies).Name,
                              ChooseEvolution, unlockedSpecies);
                    ++count;
                }
            if (!count)
                AddOption(player, GOSSIP_ICON_CHAT, "No hay cambios de forma disponibles para tu nivel", Main, Back);
            AddOption(player, GOSSIP_ICON_CHAT, "Volver al dispositivo", Main, Back);
            SendGossipMenuFor(player, 1, source);
            return;
        }
        case MissionMenuAction:
            MissionMenu(player, source, state);
            return;
        case StarterMenuAction:
            if (!state.Active && state.Companions.empty() && player->HasItemCount(Config.Device, 1, true))
            {
                SendStarterOptions(player);
                return;
            }
            break;
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
        Config.CaptureHealth = sConfigMgr->GetOption<uint32>("DigimonTamer.CaptureHealthPercent", 25);
        Config.CaptureCooldown = sConfigMgr->GetOption<uint32>("DigimonTamer.CaptureCooldownSeconds", 10);
        Config.Experiments = sConfigMgr->GetOption<bool>("DigimonTamer.ExperimentsEnable", false);
        if (Config.BitsPerKill > 1000000 || !Config.CaptureHealth ||
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
                                         Technique{fields[1].Get<std::string>(), fields[2].Get<std::string>(), fields[3].Get<uint32>(),
                                                   static_cast<Stage>(fields[4].Get<uint8>()), fields[5].Get<uint64>(),
                                                   fields[8].Get<uint8>(), fields[9].Get<uint32>(),
                                                   fields[10].Get<uint8>() != 0, fields[6].Get<uint8>() != 0,
                                                   fields[7].Get<uint8>() != 0});
            } while (rows->NextRow());
        if (auto rows = Query(SelectConsumables))
            do
            {
                Field* fields = rows->Fetch();
                uint32 const entry = fields[0].Get<uint32>();
                Consumable item{fields[1].Get<std::string>(), fields[2].Get<uint8>(), fields[3].Get<uint8>(),
                                fields[4].Get<uint16>(), fields[5].Get<uint32>(), fields[6].Get<uint8>() != 0};
                bool const validKind = item.Kind == "FOOD" || item.Kind == "DAMAGE" ||
                    item.Kind == "PROTECTION" || item.Kind == "SPEED";
                bool const valid = entry && validKind && item.HappinessGain <= 100 && item.Magnitude <= 50 &&
                    item.Duration <= 3600 && (item.Kind == "FOOD" || item.Kind == "SPEED" || item.Duration > 0) &&
                    (item.Kind != "SPEED" || item.Spell != 0);
                if (!valid)
                {
                    LOG_ERROR("module.digimon", "Invalid consumable catalog row for item {}", entry);
                    return false;
                }
                ConsumableCatalog.emplace(entry, std::move(item));
            } while (rows->NextRow());
        if (ConsumableCatalog.empty())
        {
            LOG_ERROR("module.digimon", "Consumable catalog is empty; apply migration 021 and world item templates.");
            return false;
        }
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
        if (auto rows = Query(SelectCombatProfiles))
            do
            {
                Field* f = rows->Fetch();
                CombatProfiles.emplace(f[0].Get<uint32>(), CombatProfile{f[1].Get<std::string>(),
                    f[2].Get<uint32>(), f[3].Get<uint32>(), f[4].Get<uint32>(), f[5].Get<uint8>(), f[6].Get<uint32>(),
                    f[7].Get<float>(), f[8].Get<float>(), f[9].Get<float>(), f[10].Get<float>(), f[11].Get<float>(), f[12].Get<uint8>()});
            } while (rows->NextRow());
        if (auto rows = Query(SelectRequirements))
            do
            {
                Field* f = rows->Fetch();
                Requirements[{f[0].Get<uint32>(), f[1].Get<uint32>()}] =
                    {f[2].Get<std::string>(), f[3].Get<uint16>(), f[4].Get<uint16>(), f[5].Get<uint8>(),
                     f[6].Get<std::string>(), f[7].Get<uint32>(), f[8].Get<uint32>()};
            } while (rows->NextRow());
        if (auto rows = Query(SelectEvolutions))
            do
            {
                Field* fields = rows->Fetch();
                Evolutions[fields[0].Get<uint32>()].push_back(
                    Evolution{fields[1].Get<uint32>(), fields[2].Get<uint8>()});
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
        ConsumableCatalog.clear();
        Aliases.clear();
        SpeciesTechniques.clear();
        Evolutions.clear();
        Requirements.clear();
        CombatProfiles.clear();
        TechniqueGlobalCooldowns.clear();
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
        uint32 const consumableScript = sObjectMgr->GetScriptId("DigimonConsumable");
        for (auto const& [entry, item] : ConsumableCatalog)
            if (item.Enabled)
            {
                ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(entry);
                if (!itemTemplate || itemTemplate->ScriptId != consumableScript ||
                    (item.Kind == "SPEED" && !sSpellMgr->GetSpellInfo(item.Spell)))
                {
                    LOG_ERROR("module.digimon", "Consumable {} is missing its item script or spell", entry);
                    World::StopNow(ERROR_EXIT_CODE);
                    return;
                }
            }
        for (auto const& [id, profile] : CombatProfiles)
        {
            bool valid = TechniqueCatalog.count(id) && profile.Hits >= 1 && profile.Hits <= 8 &&
                profile.Cost <= 100 && profile.Range >= 0 && profile.Range <= 40 && profile.Radius <= 10 &&
                profile.Duration <= 30000 && std::isfinite(profile.Base) && std::isfinite(profile.PerLevel) &&
                std::isfinite(profile.Weapon) && profile.Base >= 0 && profile.PerLevel >= 0 && profile.Weapon >= 0;
            for (uint32 spell : {profile.Primary, profile.Secondary, profile.Utility})
                valid = valid && (!spell || sSpellMgr->GetSpellInfo(spell));
            if (!valid)
            {
                LOG_ERROR("module.digimon", "Invalid combat profile {}", id);
                World::StopNow(ERROR_EXIT_CODE);
                return;
            }
        }
        // Explicit directed routes may connect equal stages, but never form a cycle.
        std::unordered_map<uint32, uint8> colors;
        std::function<bool(uint32)> visit = [&](uint32 node)
        {
            if (colors[node] == 1)
                return false;
            if (colors[node] == 2)
                return true;
            colors[node] = 1;
            if (auto it = Evolutions.find(node); it != Evolutions.end())
                for (Evolution const& edge : it->second)
                    if (!visit(edge.Target))
                        return false;
            colors[node] = 2;
            return true;
        };
        for (auto const& [speciesId, species] : Catalog)
            if (!visit(speciesId))
            {
                LOG_ERROR("module.digimon", "Evolution graph contains a cycle at {}", speciesId);
                World::StopNow(ERROR_EXIT_CODE);
                return;
            }
        for (auto const& [source, rules] : Evolutions)
            for (Evolution const& rule : rules)
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
        LOG_INFO("module.digimon", "Alpha 0.3 loaded: {} species; interactive five-node captures; protected quest entries {}.",
                 Catalog.size(), ProtectedQuestEntries.size());
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
        CleanSpentCatalysts(player);
        CleanSpentConsumables(player);
        if (state.Active)
        {
            GiveDevice(player);
            if (state.Summoned)
                Summon(player, state);
        }
        else if (state.Companions.empty() && player->getRace() == RACE_HUMAN &&
                 !player->HasItemCount(Config.Device, 1, true))
            Notify(player, "Tu historia en el Mundo Digital comienza con el Guía de Villanorte. Completa su cadena para recibir el Dispositivo Sagrado y elegir compañero.");
        else if (state.Companions.empty() && player->HasItemCount(Config.Device, 1, true))
            Notify(player, "El Dispositivo Sagrado está listo. Habla con el Guía y elige tu primer Digimon.");
        else
            Notify(player, "Bienvenido, Tamer. Usa el Dispositivo Sagrado o .digimon para dirigir a tu compañero.");
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

Companion* ManagedCompanion(Unit* unit)
{
    if (!Managed(unit))
        return nullptr;
    ObjectGuid const ownerGuid = unit->GetOwnerGUID();
    auto tamer = Tamers.find(ownerGuid.GetCounter());
    return tamer != Tamers.end() ? Active(tamer->second) : nullptr;
}

void ApplyCareCombatEffects(Unit* attacker, Unit* victim, uint32& damage, bool techniqueDamage)
{
    if (!damage)
        return;
    std::lock_guard<std::recursive_mutex> guard(Mutex);
    uint64 const now = GameTime::GetGameTime().count();
    if (Companion* companion = ManagedCompanion(attacker))
    {
        bool const lowFocus = attacker->GetPower(POWER_FOCUS) * 2u <
                              attacker->GetMaxPower(POWER_FOCUS);
        uint32 bonus = FormTalentDamageBonus(companion->Profile, companion->Talents,
            techniqueDamage, victim->GetHealthPct() < 50.0f,
            attacker->GetHealthPct() > 75.0f, lowFocus, !lowFocus);
        if (companion->Role == "DPS")
            bonus += 15u;
        if (attacker->GetHealthPct() < 50.0f)
            bonus += 2u * companion->Talents[14];
        if (Creature const* creature = victim->ToCreature(); creature && creature->isElite())
            bonus += 10u * companion->Talents[10];
        if (companion->Talents[6] && roll_chance_i(companion->Talents[6]))
            bonus += 50u;
        damage = static_cast<uint32>(std::min<uint64>(std::numeric_limits<uint32>::max(),
            (uint64(damage) * (100u + bonus) + 99u) / 100u));
    }
    if (Companion* companion = ManagedCompanion(attacker);
        companion && companion->DamageBuffExpires > now && companion->DamageBuffPct)
        damage = static_cast<uint32>(std::min<uint64>(std::numeric_limits<uint32>::max(),
            (uint64(damage) * (100 + companion->DamageBuffPct) + 99) / 100));
    if (Companion* companion = ManagedCompanion(victim))
    {
        uint32 reduction = companion->Talents[12] +
            (companion->Role == "TANK" ? 10u : 0u);
        if (victim->GetHealthPct() < 35.0f)
            reduction += 10u * companion->Talents[18];
        damage = static_cast<uint32>(std::max<uint64>(1,
            uint64(damage) * (100u - reduction) / 100u));
    }
    if (Companion* companion = ManagedCompanion(victim);
        companion && companion->ProtectionBuffExpires > now && companion->ProtectionBuffPct)
        damage = static_cast<uint32>(std::max<uint64>(1,
            uint64(damage) * (100 - companion->ProtectionBuffPct) / 100));
}

class DigimonDamageScript final : public UnitScript
{
public:
    DigimonDamageScript()
        : UnitScript("DigimonDamageScript", true,
                     {UNITHOOK_ON_DAMAGE, UNITHOOK_ON_HEAL, UNITHOOK_MODIFY_HEAL_RECEIVED,
                      UNITHOOK_MODIFY_MELEE_DAMAGE, UNITHOOK_MODIFY_SPELL_DAMAGE_TAKEN,
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
                    if (Companion* companion = ManagedCompanion(attacker))
                    {
                        float const bonus = (companion->Role == "TANK" ? 1.5f : 0.0f) +
                            0.1f * companion->Talents[13];
                        if (bonus > 0.0f)
                            victim->GetThreatMgr().AddThreat(attacker, float(damage) * bonus,
                                                             nullptr, true, true);
                    }
                    if (auto wild = Wild.find(victim->GetGUID()); wild != Wild.end() && !wild->second.Claimed)
                        wild->second.WeakenedBy[OwnerId(owner)] = attacker->GetGUID();
                }
    }
    void ModifyHealReceived(Unit* first, Unit* second, uint32& heal, SpellInfo const* spell) override
    {
        if (!Config.Enabled || !first || !second || !heal)
            return;
        // This core passes caster first for direct heals and target first for periodic heals.
        Unit* caster = spell && spell->Id == 2061 ? first : second;
        Unit* target = spell && spell->Id == 2061 ? second : first;
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        uint32 bonus = 0;
        if (Companion* companion = ManagedCompanion(caster))
        {
            bonus += 2u * companion->Talents[3] +
                     FormTalentHealingBonus(companion->Profile, companion->Talents,
                                            target->GetHealthPct() < 50.0f) +
                     (companion->Role == "HEALER" ? 50u : 0u);
            if (target->GetHealthPct() < 50.0f)
                bonus += 2u * companion->Talents[22];
            if (caster->GetOwnerGUID() != target->GetOwnerGUID())
                bonus += 15u * companion->Talents[26];
        }
        if (Companion* recipient = ManagedCompanion(target))
            bonus += 2u * recipient->Talents[16];
        heal = static_cast<uint32>(std::min<uint64>(std::numeric_limits<uint32>::max(),
            uint64(heal) * (100u + bonus) / 100u));
    }
    void OnHeal(Unit* healer, Unit* receiver, uint32& gain) override
    {
        if (!Config.Enabled || !healer || !receiver || !gain || !Managed(receiver))
            return;
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        Companion* companion = ManagedCompanion(healer);
        if (!companion || !companion->Talents[24] || receiver->HasAura(17, healer->GetGUID()))
            return;
        CustomSpellValues values;
        values.AddSpellMod(SPELLVALUE_BASE_POINT0,
            int32(std::max<uint32>(1u, gain * 4u * companion->Talents[24] / 100u)));
        healer->CastCustomSpell(17, values, receiver, TRIGGERED_FULL_MASK);
    }
    void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage) override
    {
        // Apply before the combat log packet as well as at final damage delivery.
        FilterDamage(attacker, target, damage);
        if (damage)
            ApplyCareCombatEffects(attacker, target, damage, false);
    }
    void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage, SpellInfo const*) override
    {
        FilterDamage(attacker, target, damage);
        if (damage)
            ApplyCareCombatEffects(attacker, target, damage, true);
    }
    void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const*) override
    {
        if (damage <= 0)
            return;
        uint32 amount = static_cast<uint32>(damage);
        FilterDamage(attacker, target, amount);
        if (amount)
            ApplyCareCombatEffects(attacker, target, amount, true);
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

bool HasActivePagumon(Player* player)
{
    auto state = Tamers.find(OwnerId(player));
    Companion* companion = state == Tamers.end() ? nullptr : Active(state->second);
    return companion && companion->SpeciesId == PagumonSpecies;
}

bool HandlePagumonQuestAccept(Player* player, Quest const* quest)
{
    if (!quest || !IsPagumonQuest(quest->GetQuestId()))
        return false;
    uint32 const questId = quest->GetQuestId();
    auto state = Tamers.find(OwnerId(player));
    Companion* companion = state == Tamers.end() ? nullptr : Active(state->second);
    if (!companion || companion->SpeciesId != PagumonSpecies ||
        !Requirements.count({PagumonSpecies, ImpmonSpecies}) ||
        !Evolutions.count(PagumonSpecies) ||
        std::none_of(Evolutions.at(PagumonSpecies).begin(), Evolutions.at(PagumonSpecies).end(),
                     [](Evolution const& route) { return route.Target == ImpmonSpecies; }))
    {
        player->RemoveActiveQuest(questId, true);
        Notify(player, "Esta cadena solo puede iniciarse con Pagumon activo.");
        return true;
    }
    auto transaction = std::make_shared<Transaction<Connection>>();
    transaction->Append(Make(StartTrial, companion->Id, ImpmonSpecies));
    transaction->Append(Make(SaveNativeQuest, OwnerId(player), questId, companion->Id, ImpmonSpecies));
    if (Database->ExecuteTransaction(transaction) != 0)
    {
        player->RemoveActiveQuest(questId, true);
        Notify(player, "No se pudo vincular esta misión a tu Pagumon; vuelve a aceptarla.");
    }
    return true;
}

bool HandlePagumonQuestReward(Player* player, Quest const* quest)
{
    if (!quest || !IsPagumonQuest(quest->GetQuestId()))
        return false;
    uint32 const questId = quest->GetQuestId();
    auto binding = Query(SelectNativeQuest, OwnerId(player), questId);
    if (!binding)
    {
        Notify(player, "No se encontró el vínculo de Pagumon para esta misión; el registro de evolución no se ha actualizado.");
        return false;
    }
    Field* fields = binding->Fetch();
    uint32 const companionId = fields[0].Get<uint32>();
    uint32 const target = fields[1].Get<uint32>();
    auto state = Tamers.find(OwnerId(player));
    if (state == Tamers.end() || target != ImpmonSpecies)
        return false;
    auto companion = std::find_if(state->second.Companions.begin(), state->second.Companions.end(),
        [companionId](Companion const& value) { return value.Id == companionId; });
    auto requirement = Requirements.find({PagumonSpecies, ImpmonSpecies});
    if (companion == state->second.Companions.end() || companion->SpeciesId != PagumonSpecies ||
        requirement == Requirements.end())
    {
        Notify(player, "El Pagumon vinculado ya no está disponible; la evolución no se ha actualizado.");
        return false;
    }
    auto transaction = std::make_shared<Transaction<Connection>>();
    if (questId == PagumonQuestLast)
    {
        transaction->Append(Make(CompleteNativeTrial, requirement->second.Victories, companionId, ImpmonSpecies));
        transaction->Append(Make(CreditBond, companionId, uint8(5)));
    }
    transaction->Append(Make(DeleteNativeQuest, OwnerId(player), questId));
    if (Database->ExecuteTransaction(transaction) != 0)
        Notify(player, "La misión se completó, pero no se pudo guardar la ruta evolutiva. Avisa al administrador.");
    else if (questId == PagumonQuestLast)
    {
        Reload(player);
        Notify(player, "Ruta de Pagumon registrada: XP de Tamer, catalizador arcano y 5 de Empatía. Al nivel 10 podrás digievolucionar a Impmon.");
    }
    else
        Notify(player, "Progreso de Pagumon registrado. Continúa la cadena con el siguiente personaje.");
    return false;
}

void CreditHumanStoryTalk(Player* player, uint32 entry)
{
    std::lock_guard<std::recursive_mutex> guard(Mutex);
    uint32 questId = entry == 900160 ? 900601 : entry == 900161 ? 900603 :
                     entry == Config.Welcome ? 900605 : 0;
    if (questId && player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
        player->AreaExploredOrEventHappens(questId);
    uint32 const pagumonTalkQuest = entry == 900160 ? PagumonQuestBase :
                                    entry == Config.Welcome ? PagumonQuestLast : 0;
    if (pagumonTalkQuest && HasActivePagumon(player) &&
        player->GetQuestStatus(pagumonTalkQuest) == QUEST_STATUS_INCOMPLETE)
        player->AreaExploredOrEventHappens(pagumonTalkQuest);
    uint32 const arrivalQuest = entry == 900163 ? 900701 : entry == 900164 ? 900702 :
                                entry == 900165 ? 900703 : entry == Config.Welcome ? 900704 : 0;
    if (arrivalQuest && player->GetQuestStatus(arrivalQuest) == QUEST_STATUS_INCOMPLETE)
        player->AreaExploredOrEventHappens(arrivalQuest);
}

class DigimonQuestNPC final : public CreatureScript
{
public:
    DigimonQuestNPC() : CreatureScript("DigimonQuestNPC") {}
    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!player->IsWithinDistInMap(creature, INTERACTION_DISTANCE))
            return true;
        CreditHumanStoryTalk(player, creature->GetEntry());
        ClearGossipMenuFor(player);
        player->PrepareQuestMenu(creature->GetGUID());
        SendGossipMenuFor(player, 1, creature->GetGUID());
        return true;
    }
    bool OnQuestAccept(Player* player, Creature*, Quest const* quest) override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        return HandlePagumonQuestAccept(player, quest);
    }
    bool OnQuestReward(Player* player, Creature*, Quest const* quest, uint32) override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        return HandlePagumonQuestReward(player, quest);
    }
};

class DigimonWelcome final : public CreatureScript
{
public:
    DigimonWelcome() : CreatureScript("DigimonWelcome") {}
    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->GetEntry() == Config.Welcome)
            CreditHumanStoryTalk(player, creature->GetEntry());
        Menu(player, creature->GetGUID(), creature->GetEntry() == Config.Welcome);
        return true;
    }
    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        if (player->IsWithinDistInMap(creature, INTERACTION_DISTANCE))
            Select(player, creature->GetGUID(), creature->GetEntry() == Config.Welcome, sender, action);
        return true;
    }
    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        if (IsPagumonQuest(quest ? quest->GetQuestId() : 0) && creature->GetEntry() == Config.Welcome)
            return HandlePagumonQuestAccept(player, quest);
        if (!quest || quest->GetQuestId() < ImpmonQuestBase || quest->GetQuestId() > ImpmonQuestBase + 12 ||
            creature->GetEntry() != Config.Welcome || !Tamers.count(OwnerId(player)))
            return false;
        uint32 const target = 105 + quest->GetQuestId() - ImpmonQuestBase;
        Companion* companion = Active(Tamers.at(OwnerId(player)));
        if (!companion || companion->Forms.count(target) || !Requirements.count({companion->SpeciesId, target}) ||
            !Evolutions.count(companion->SpeciesId) ||
            std::none_of(Evolutions.at(companion->SpeciesId).begin(), Evolutions.at(companion->SpeciesId).end(),
                         [target](Evolution const& route) { return route.Target == target; }))
        {
            player->RemoveActiveQuest(quest->GetQuestId(), true);
            Notify(player, "Esta misión solo está disponible con el Digimon de la rama correspondiente activo.");
            return true;
        }
        auto transaction = std::make_shared<Transaction<Connection>>();
        transaction->Append(Make(StartTrial, companion->Id, target));
        transaction->Append(Make(SetTraining, companion->Id, target));
        transaction->Append(Make(SaveNativeQuest, OwnerId(player), quest->GetQuestId(), companion->Id, target));
        if (Database->ExecuteTransaction(transaction) != 0)
            Notify(player, "No se pudo registrar el vínculo entre la misión y tu Digimon.");
        return false;
    }
    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32) override
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        if (IsPagumonQuest(quest ? quest->GetQuestId() : 0) && creature->GetEntry() == Config.Welcome)
            return HandlePagumonQuestReward(player, quest);
        if (!quest || quest->GetQuestId() < ImpmonQuestBase || quest->GetQuestId() > ImpmonQuestBase + 12 ||
            creature->GetEntry() != Config.Welcome || !Tamers.count(OwnerId(player)))
            return false;
        auto binding = Query(SelectNativeQuest, OwnerId(player), quest->GetQuestId());
        if (!binding)
            return false;
        Field* fields = binding->Fetch();
        uint32 const companionId = fields[0].Get<uint32>();
        uint32 const target = fields[1].Get<uint32>();
        auto& companions = Tamers.at(OwnerId(player)).Companions;
        auto companion = std::find_if(companions.begin(), companions.end(),
            [companionId](Companion const& value) { return value.Id == companionId; });
        if (companion == companions.end() || target != 105 + quest->GetQuestId() - ImpmonQuestBase ||
            !Requirements.count({companion->SpeciesId, target}))
            return false;
        auto transaction = std::make_shared<Transaction<Connection>>();
        transaction->Append(Make(CompleteNativeTrial, Requirements.at({companion->SpeciesId, target}).Victories,
                                 companionId, target));
        transaction->Append(Make(CreditBond, companionId, uint8(5)));
        transaction->Append(Make(ClearTraining, companionId));
        transaction->Append(Make(DeleteNativeQuest, OwnerId(player), quest->GetQuestId()));
        if (Database->ExecuteTransaction(transaction) != 0)
            Notify(player, "La misión se entregó, pero no se pudo sincronizar el registro evolutivo; avisa al administrador.");
        else
        {
            Reload(player);
            Notify(player, "Misión completada: XP de Tamer, recompensa nativa y Empatía para tu compañero (máximo 100).");
        }
        return false;
    }
};

void CleanSpentConsumables(Player* player)
{
    if (!player || !Database)
        return;
    uint32 const owner = OwnerId(player);
    if (auto rows = Query(SelectConsumedItems, owner))
        do
        {
            Field* fields = rows->Fetch();
            uint32 const guid = fields[0].Get<uint32>();
            uint32 const entry = fields[1].Get<uint32>();
            if (Item* item = player->GetItemByGuid(ObjectGuid::Create<HighGuid::Item>(guid));
                item && item->GetEntry() == entry)
                player->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
            Execute(DeleteConsumedItem, guid, owner);
        } while (rows->NextRow());
}

class DigimonConsumable final : public ItemScript
{
public:
    DigimonConsumable() : ItemScript("DigimonConsumable") {}
    bool OnUse(Player* player, Item* item, SpellCastTargets const&) override
    {
        if (!player || !item)
            return false;
        auto definition = ConsumableCatalog.find(item->GetEntry());
        if (definition == ConsumableCatalog.end() || !definition->second.Enabled)
            return false;
        Consumable const& consumable = definition->second;
        // Swiftness is a native 3.3.5a item spell; let the core cast and consume it.
        if (consumable.Kind == "SPEED")
            return false;

        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        uint32 const owner = OwnerId(player);
        if (!Tamers.count(owner) && !Reload(player))
            return true;
        Tamer& state = Tamers.at(owner);
        Companion* companion = Active(state);
        if (!player->IsAlive() || player->IsInCombat() || !companion || Busy(player, state))
        {
            Notify(player, "Necesitas un compañero y estar fuera de combate para usar este alimento.");
            return true;
        }

        uint64 const now = GameTime::GetGameTime().count();
        uint8 const happiness = static_cast<uint8>(std::min<uint32>(100,
            uint32(companion->Happiness) + consumable.HappinessGain));
        uint8 damagePct = companion->DamageBuffPct;
        uint64 damageExpires = companion->DamageBuffExpires;
        uint8 protectionPct = companion->ProtectionBuffPct;
        uint64 protectionExpires = companion->ProtectionBuffExpires;
        if (consumable.Kind == "DAMAGE")
        {
            damagePct = consumable.Magnitude;
            damageExpires = now + consumable.Duration;
        }
        else if (consumable.Kind == "PROTECTION")
        {
            protectionPct = consumable.Magnitude;
            protectionExpires = now + consumable.Duration;
        }
        else if (consumable.Kind != "FOOD")
            return true;

        auto transaction = std::make_shared<Transaction<Connection>>();
        transaction->Append(Make(SaveCareAndEffects, happiness, damagePct, damageExpires, protectionPct,
                                 protectionExpires, owner, companion->Id));
        transaction->Append(Make(RecordConsumedItem, item->GetGUID().GetCounter(), owner, item->GetEntry()));
        if (Database->ExecuteTransaction(transaction) != 0)
        {
            Notify(player, "No se pudo guardar el cuidado; el objeto no se ha consumido. Inténtalo de nuevo.");
            return true;
        }

        companion->Happiness = happiness;
        companion->DamageBuffPct = damagePct;
        companion->DamageBuffExpires = damageExpires;
        companion->ProtectionBuffPct = protectionPct;
        companion->ProtectionBuffExpires = protectionExpires;
        player->DestroyItemCount(item->GetEntry(), 1, true);
        if (consumable.Kind == "FOOD")
            Notify(player, Catalog.at(companion->SpeciesId).Name + " disfruta la comida. Felicidad: " + std::to_string(happiness) + "/100.");
        else
            Notify(player, "Cuidado aplicado a " + Catalog.at(companion->SpeciesId).Name +
                ". Felicidad: " + std::to_string(happiness) + "/100. El efecto de esta categoría se renueva, no se acumula.");
        SendCombatState(player, state);
        if (state.Addon)
            SendDetails(player, state);
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
                {"digixp", GiveCompanionXP, rbac::RBAC_PERM_COMMAND_NPC_ADD, Console::No},
                {"listadigi", ListCatalog, rbac::RBAC_PERM_COMMAND_NPC_ADD, Console::No},
                {"digimonlabbar", NativeControlBarExperiment, rbac::RBAC_PERM_COMMAND_NPC_ADD, Console::No},
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
            handler->SendSysMessage(std::to_string(id) + " - " + species.Name +
                                    (species.Enabled ? " - disponible" : " - pendiente de instalar/habilitar"));
        for (auto const& [name, id] : Aliases)
            handler->SendSysMessage("Alias: " + name + " â†’ " + std::to_string(id));
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
        if (!id || level < 1 || level > 100)
        {
            handler->SendSysMessage("Uso: .spawndigi Nombre o ID [nivel 1-100]. Consulta .listadigi.");
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
            Companion preview{};
            preview.SpeciesId = id;
            preview.Evolution = species.DefaultStage;
            preview.Level = uint8(level);
            preview.Health = 100.0f;
            ApplyStats(actor, preview);
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
        if (parsed.ec != std::errc() || parsed.ptr != text.data() + text.size() || level < 1 || level > 100)
        {
            handler->SendSysMessage("Uso: .digilevel 1-100. Ajusta tu compañero actual; XP parcial a cero.");
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
        bool const levelled = level > companion->Level;
        auto const form = levelled ? ResolveEvolution(*companion, uint8(level)) :
                          level < companion->Level ? ResolveDevolution(*companion, uint8(level)) :
                                                     std::make_pair(companion->SpeciesId, companion->Evolution);
        float const health = actor ? actor->GetHealthPct() : companion->Health;
        auto transaction = std::make_shared<Transaction<Connection>>();
        transaction->Append(Make(SaveProgress, form.first, uint8(level), uint64(0), health, uint8(form.second),
                                 OwnerId(player), companion->Id));
        if (form.first != companion->SpeciesId)
        {
            if (!companion->Forms.count(form.first))
                transaction->Append(Make(CloneFormEquipment, companion->SpeciesId, OwnerId(player), companion->Id));
            transaction->Append(Make(UnlockForm, companion->Id, form.first));
            transaction->Append(Make(RecordEvolutionHistory, companion->Id, companion->SpeciesId, form.first,
                                     uint8(level), levelled ? "EVOLVE" : "DEVOLVE"));
        }
        if (level < companion->Level)
            transaction->Append(Make(PruneTalentPoints, companion->Id, uint32(level - 1)));
        transaction->Append(Make(LearnEligibleTechniques, companion->Id));
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
        if (actor && levelled)
            PlayLevelUpVisual(actor);
        SendTalents(player, updated);
        SendDetails(player, updated);
        SendCombatState(player, updated);
        handler->SendSysMessage("Compañero: " + Catalog.at(form.first).Name + " - nivel " + std::to_string(level) +
                                ". Guardado; evolución o involución aplicada si corresponde.");
        return true;
    }
    static bool GiveCompanionXP(ChatHandler* handler, char const* args)
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        Player* player = Administrator(handler);
        if (!player || (!Tamers.count(OwnerId(player)) && !Reload(player)))
            return false;
        std::string_view const text = args ? args : "";
        uint32 amount = 0;
        auto const parsed = std::from_chars(text.data(), text.data() + text.size(), amount);
        if (parsed.ec != std::errc() || parsed.ptr != text.data() + text.size() || !amount || amount > 1000000)
        {
            handler->SendSysMessage("Uso: .digixp 1-1000000. Suma XP al compañero actual y conserva el progreso sobrante.");
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
        Progress const progress = GainExperience({companion->Level, companion->Experience}, amount);
        bool const levelled = progress.Level > companion->Level;
        auto const form = levelled ? ResolveEvolution(*companion, progress.Level) :
                                     std::make_pair(companion->SpeciesId, companion->Evolution);
        bool const evolved = form.first != companion->SpeciesId;
        float const health = actor ? actor->GetHealthPct() : companion->Health;
        auto transaction = std::make_shared<Transaction<Connection>>();
        transaction->Append(Make(SaveProgress, form.first, progress.Level, progress.Experience, health,
                                 uint8(form.second), OwnerId(player), companion->Id));
        transaction->Append(Make(LearnEligibleTechniques, companion->Id));
        if (evolved)
        {
            if (!companion->Forms.count(form.first))
                transaction->Append(Make(CloneFormEquipment, companion->SpeciesId, OwnerId(player), companion->Id));
            transaction->Append(Make(UnlockForm, companion->Id, form.first));
            transaction->Append(Make(RecordEvolutionHistory, companion->Id, companion->SpeciesId, form.first,
                                     progress.Level, "EVOLVE"));
            transaction->Append(Make(PruneEquipment, OwnerId(player), companion->Id));
        }
        if (levelled)
            transaction->Append(Make(AuditLevel, player->GetSession()->GetAccountId(), OwnerId(player), companion->Id,
                                     companion->SpeciesId, form.first, companion->Level, progress.Level));
        if (Database->ExecuteTransaction(transaction) != 0 || !Reload(player))
        {
            handler->SendSysMessage("No se pudo guardar la experiencia. Consulta el registro del servidor.");
            return true;
        }
        Tamer& updated = Tamers.at(OwnerId(player));
        Companion* current = Active(updated);
        if (actor && current && (evolved || levelled) && !ApplyForm(player, actor, *current))
            LOG_ERROR("module.digimon", "Cannot apply admin XP to actor for owner {}", OwnerId(player));
        if (actor && levelled)
            PlayLevelUpVisual(actor);
        if (current)
        {
            uint64 const required = current->Level < 100 ? 100u + 75u * current->Level : 0;
            handler->SendSysMessage("XP añadida: " + std::to_string(amount) + ". " + Catalog.at(current->SpeciesId).Name +
                                    " nivel " + std::to_string(current->Level) + ", progreso " +
                                    std::to_string(current->Experience) + "/" + std::to_string(required) + ".");
            if (evolved)
                Notify(player, "¡Digievolución! Tu compañero ahora es " + Catalog.at(current->SpeciesId).Name + ".");
        }
        if (evolved)
        {
            SendTalents(player, updated);
            SendDetails(player, updated);
        }
        SendCombatState(player, updated);
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
        else if (command.substr(0, 5) == "mode ")
        {
            std::string const mode(command.substr(5));
            if (mode != "aggressive" && mode != "defensive" && mode != "passive")
            {
                Notify(player, "Conductas disponibles: aggressive, defensive o passive.");
                return true;
            }
            state.Behaviour = mode;
            if (Creature* actor = Actor(player, state); actor && actor->GetCharmerGUID() == player->GetGUID())
                actor->SetReactState(mode == "aggressive" ? REACT_AGGRESSIVE :
                                     mode == "passive" ? REACT_PASSIVE : REACT_DEFENSIVE);
            if (Creature* actor = Actor(player, state); actor && mode == "passive")
            {
                actor->AttackStop();
                actor->GetThreatMgr().ClearAllThreat();
                actor->GetMotionMaster()->MoveFollow(player, 2.0f, 0.8f);
            }
            static std::map<std::string, std::string> const labels = {
                {"aggressive", "Agresivo"}, {"defensive", "Defensivo"}, {"passive", "Pasivo"}};
            Notify(player, "Conducta del compañero: " + labels.at(mode) + ".");
            SendCombatState(player, state);
        }
        else if (command == "details")
        {
            state.Addon = true;
            SendDetails(player, state);
        }
        else if (command == "talents")
        {
            state.Addon = true;
            SendTalents(player, state);
        }
        else if (command.substr(0, 5) == "role ")
        {
            Notify(player, "La función pertenece a la forma evolutiva actual y no se cambia manualmente.");
            SendTalents(player, state);
        }
        else if (command == "talent reset")
        {
            Companion* companion = Active(state);
            if (!companion || !player->IsAlive() || Busy(player, state))
                return false;
            PersistHealth(player, state);
            auto transaction = std::make_shared<Transaction<Connection>>();
            transaction->Append(Make(ClearTalentPoints, OwnerId(player), companion->Id));
            transaction->Append(Make(PruneEquipment, OwnerId(player), companion->Id));
            if (Database->ExecuteTransaction(transaction) == 0 && Reload(player))
            {
                Tamer& updated = Tamers.at(OwnerId(player));
                if (Companion* active = Active(updated))
                    if (Creature* actor = Actor(player, updated))
                    {
                        ApplyStats(actor, *active);
                        RefreshTamerSpeed(player, actor, *active);
                    }
                SendTalents(player, updated);
                SendDetails(player, updated);
                SendCombatState(player, updated);
                Notify(player, "Puntos de talento devueltos para reasignarlos.");
            }
            else
                Notify(player, "No se pudieron reasignar los talentos.");
        }
        else if (command.substr(0, 7) == "talent ")
        {
            uint32 node = 0;
            auto parsed = std::from_chars(command.data() + 7, command.data() + command.size(), node);
            Companion* companion = Active(state);
            if (parsed.ec != std::errc() || parsed.ptr != command.data() + command.size() ||
                node < 1 || node > 27 || !companion || !player->IsAlive() || Busy(player, state))
                return false;
            if (!CanInvestTalent(companion->Level, companion->Talents, uint8(node)))
            {
                Notify(player, "No quedan puntos disponibles o este talento ya está al máximo.");
                return true;
            }
            uint8 const previous = companion->Talents[node];
            PersistHealth(player, state);
            if (Execute(SpendTalentPoint, uint8(node), OwnerId(player), companion->Id, uint8(node),
                                      FindTalentNode(uint8(node))->MaxRank))
            {
                Execute(LearnEligibleTechniques, companion->Id);
                if (Reload(player))
                {
                    Tamer& updated = Tamers.at(OwnerId(player));
                    if (Companion* active = Active(updated))
                    {
                        if (Creature* actor = Actor(player, updated))
                        {
                            ApplyStats(actor, *active);
                            RefreshTamerSpeed(player, actor, *active);
                        }
                        Notify(player, active->Talents[node] > previous ? "Punto de talento asignado." :
                                   "No se pudo asignar el punto.");
                    }
                    SendTalents(player, updated);
                    SendDetails(player, updated);
                    SendCombatState(player, updated);
                }
            }
            else
                Notify(player, "No se pudo guardar el talento.");
        }
        else if (command == "storage" || command.substr(0, 8) == "storage ")
        {
            state.Addon = true;
            uint32 box = 1;
            if (command.size() > 8)
            {
                auto parsed = std::from_chars(command.data() + 8, command.data() + command.size(), box);
                if (parsed.ec != std::errc() || parsed.ptr != command.data() + command.size() || !box)
                    return false;
            }
            SendStorage(player, state, static_cast<uint16>(box));
        }
        else if (command == "buybox")
        {
            uint16 const boxes = state.StorageBoxes;
            if (boxes >= 100)
                Notify(player, "Has alcanzado el máximo de 100 cajas.");
            else
            {
                uint64 const price = 10000ULL * (boxes - 1) * (boxes - 1);
                if (player->GetMoney() < price || !player->HasItemCount(900240, 1))
                    Notify(player, "Necesitas " + std::to_string(price / 10000) + " de oro y un Núcleo de expansión.");
                else if (Execute(BuyStorageBox, OwnerId(player), boxes))
                {
                    player->ModifyMoney(-static_cast<int32>(price));
                    player->DestroyItemCount(900240, 1, true);
                    if (Reload(player))
                    {
                        SendStorage(player, Tamers.at(OwnerId(player)), uint16(boxes + 1));
                        Notify(player, "Caja comprada. El precio de la siguiente aumentará.");
                    }
                }
                else
                    Notify(player, "No se pudo completar la compra. No se descontó oro ni el núcleo.");
            }
        }
        else if (command.substr(0, 9) == "team add " || command.substr(0, 11) == "team store ")
        {
            bool const store = command.substr(0, 11) == "team store ";
            size_t const start = store ? 11 : 9;
            uint32 id = 0;
            auto parsed = std::from_chars(command.data() + start, command.data() + command.size(), id);
            if (parsed.ec != std::errc() || parsed.ptr != command.data() + command.size() || !id)
                return false;
            Companion const* selectedCompanion = nullptr;
            for (Companion const& c : state.Companions)
                if (c.Id == id)
                    selectedCompanion = &c;
            bool saved = false;
            if (selectedCompanion && store && selectedCompanion->PartySlot < 6 && id != state.Active)
            {
                uint32 stored = 0;
                if (auto count = Query(CountStorage, OwnerId(player)))
                    stored = count->Fetch()[0].Get<uint32>();
                if (stored < uint32(state.StorageBoxes) * 15)
                    saved = Execute(StoreFromParty, OwnerId(player), id);
            }
            else if (selectedCompanion && !store && selectedCompanion->PartySlot >= 6)
                saved = Execute(MoveToParty, OwnerId(player), id);
            if (!saved)
                Notify(player, store ? "No se puede guardar el Digimon activo o la caja está llena."
                                     : "No está en tu colección, el equipo está lleno o no hay hueco en las cajas.");
            if (Reload(player))
            {
                Tamer& updated = Tamers.at(OwnerId(player));
                bool placed = false;
                for (Companion const& c : updated.Companions)
                    if (c.Id == id)
                        placed = store ? c.PartySlot >= 6 : c.PartySlot < 6;
                SendDetails(player, updated);
                SendStorage(player, updated, 1);
                if (placed)
                    Notify(player, store ? "Digimon guardado en el Digialmacén." : "Digimon trasladado al Digiequipo.");
                else
                    Notify(player, store ? "No se pudo guardar el Digimon." : "No se pudo trasladar al Digiequipo.");
            }
        }
        else if (command.substr(0, 13) == "capture-step ")
        {
            size_t const separator = command.find(' ', 13);
            if (separator == std::string_view::npos)
                return false;
            uint32 node = 0;
            auto parsed = std::from_chars(command.data() + separator + 1, command.data() + command.size(), node);
            if (parsed.ec != std::errc() || parsed.ptr != command.data() + command.size())
                return false;
            CaptureStep(player, state, command.substr(13, separator - 13), node);
        }
        else if (command.substr(0, 15) == "capture-cancel ")
        {
            if (!state.Capture.Nonce.empty() && command.substr(15) == state.Capture.Nonce)
            {
                state.Capture = {};
                SendV2(player, "CAPTURE_END|FAIL");
                Notify(player, "Has cancelado el intento de captura.");
            }
        }
        else if (command == "routes")
        {
            state.Addon = true;
            SendRoutes(player, state);
        }
        else if (command.substr(0, 6) == "claim ")
        {
            uint32 target = 0;
            auto parsed = std::from_chars(command.data() + 6, command.data() + command.size(), target);
            if (parsed.ec != std::errc() || parsed.ptr != command.data() + command.size())
                return false;
            ClaimCatalyst(player, state, target);
            SendRoutes(player, state);
        }
        else if (command.substr(0, 6) == "train ")
        {
            uint32 target = 0;
            auto parsed = std::from_chars(command.data() + 6, command.data() + command.size(), target);
            if (parsed.ec != std::errc() || parsed.ptr != command.data() + command.size())
                return false;
            ObjectGuid const source = player->GetTarget();
            if (!AtTrainer(player, source))
            {
                Notify(player, "Acepta las misiones de evolución hablando con el Guía del Mundo Digital.");
                return false;
            }
            StartTraining(player, source, state, target);
            SendRoutes(player, state);
        }
        else if (command.substr(0, 6) == "equip " || command.substr(0, 8) == "unequip ")
        {
            bool remove = command.substr(0, 8) == "unequip ";
            uint32 id = 0;
            auto parsed = std::from_chars(command.data() + (remove ? 8 : 6), command.data() + command.size(), id);
            if (parsed.ec != std::errc() || parsed.ptr != command.data() + command.size())
                return false;
            SetTechnique(player, state, id, remove);
            SendDetails(player, Tamers.at(OwnerId(player)));
            SendCombatState(player, Tamers.at(OwnerId(player)));
        }
        else if (command.substr(0, 9) == "activate ")
        {
            uint32 id = 0;
            auto parsed = std::from_chars(command.data() + 9, command.data() + command.size(), id);
            if (parsed.ec != std::errc() || parsed.ptr != command.data() + command.size())
                return false;
            ActivateCompanion(player, state, id);
            SendDetails(player, Tamers.at(OwnerId(player)));
            SendCombatState(player, Tamers.at(OwnerId(player)));
        }
        else if (command == "evolutions" || command == "routes")
        {
            state.Addon = true;
            SendRoutes(player, state);
        }
        else if (command == "starters")
        {
            state.Addon = true;
            if (!state.Active && state.Companions.empty() && player->HasItemCount(Config.Device, 1, true))
                SendStarterOptions(player);
            else
                Notify(player, "Completa la historia de llegada para desbloquear la elección de compañero.");
        }
        else if (command.substr(0, 7) == "choose ")
        {
            uint32 id = 0;
            auto const parsed = std::from_chars(command.data() + 7, command.data() + command.size(), id);
            if (parsed.ec != std::errc() || parsed.ptr != command.data() + command.size() || !id)
                return false;
            state.Addon = true;
            Select(player, player->GetGUID(), true, ChooseStarter, id, false);
            SendCombatState(player, Tamers.at(OwnerId(player)));
        }
        else if (command.substr(0, 7) == "evolve ")
        {
            uint32 target = 0;
            auto const parsed = std::from_chars(command.data() + 7, command.data() + command.size(), target);
            if (parsed.ec != std::errc() || parsed.ptr != command.data() + command.size() || !target)
                return false;
            state.Addon = true;
            Select(player, player->GetGUID(), false, ChooseEvolution, target, false);
            SendEvolutionOptions(player, Tamers.at(OwnerId(player)));
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
    static bool NativeControlBarExperiment(ChatHandler* handler, char const* args)
    {
        std::lock_guard<std::recursive_mutex> guard(Mutex);
        Player* player = Administrator(handler);
        if (!Config.Experiments || !player || player->IsInCombat() || !args)
            return false;

        auto state = Tamers.find(OwnerId(player));
        Creature* actor = state != Tamers.end() ? Actor(player, state->second) : nullptr;
        if (!actor || !actor->IsAlive() || actor->IsInCombat())
        {
            handler->SendSysMessage("Invoca un Digimon vivo y sal de combate antes de probar la barra nativa.");
            return true;
        }

        std::string_view const option(args);
        if (option == "off")
        {
            if (actor->GetCharmerGUID() == player->GetGUID())
            {
                actor->RemoveCharmedBy(player);
                // AzerothCore deletes CharmInfo when a non-guardian charm ends.
                // Manual techniques still need its global cooldown manager.
                if (!actor->GetCharmInfo())
                    actor->InitCharmInfo();
            }
            handler->SendSysMessage("Prueba de barra nativa desactivada.");
            return true;
        }

        if (option != "on" || player->GetCharm() || player->GetPet() || actor->GetCharmerGUID())
        {
            handler->SendSysMessage("Usa .digimonlabbar on/off; no puedes controlar dos criaturas a la vez.");
            return true;
        }

        if (!actor->SetCharmedBy(player, CHARM_TYPE_CHARM))
        {
            handler->SendSysMessage("El cliente no acepto el control nativo de este Digimon.");
            return true;
        }

        // This probe exposes only the client's native commands and reactions.
        // Technique casting remains on the existing server-validated path.
        actor->GetCharmInfo()->InitPetActionBar();
        actor->SetReactState(state->second.Behaviour == "aggressive" ? REACT_AGGRESSIVE :
                             state->second.Behaviour == "passive" ? REACT_PASSIVE : REACT_DEFENSIVE);
        player->CharmSpellInitialize();
        handler->SendSysMessage("Barra nativa de prueba: atacar, seguir y posturas. Sin tecnicas aun.");
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

struct DigimonCompanionAI : DigimonCombatAI
{
    TaskScheduler CombatScheduler;
    uint32 BehaviourTimer = 0;

    explicit DigimonCompanionAI(Creature* creature) : DigimonCombatAI(creature)
    {
        CombatScheduler.Schedule(std::chrono::seconds(4), [this](TaskContext context)
        {
            std::lock_guard<std::recursive_mutex> guard(Mutex);
            if (me->IsAlive() && Managed(me))
            {
                Player* owner = ObjectAccessor::FindPlayer(me->GetOwnerGUID());
                Companion* companion = owner ? Active(Tamers.at(OwnerId(owner))) : nullptr;
                uint32 const bonus = companion ? 2u * (companion->Talents[7] + companion->Talents[21]) : 0u;
                me->ModifyPower(POWER_FOCUS,
                                int32((24.0f + bonus) * sWorld->getRate(RATE_POWER_FOCUS)));
            }
            context.Repeat();
        });
        CombatScheduler.Schedule(std::chrono::seconds(8), [this](TaskContext context)
        {
            std::lock_guard<std::recursive_mutex> guard(Mutex);
            if (Player* owner = ObjectAccessor::FindPlayer(me->GetOwnerGUID());
                owner && owner->IsInMap(me) && Managed(me))
                if (auto tamer = Tamers.find(OwnerId(owner)); tamer != Tamers.end())
                    if (Companion* companion = Active(tamer->second))
                        RefreshTamerSpeed(owner, me, *companion);
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
        Techniques.Update(diff);
        BehaviourTimer = BehaviourTimer > diff ? BehaviourTimer - diff : 0;
        if (me->GetCharmerGUID() == owner->GetGUID())
        {
            std::lock_guard<std::recursive_mutex> guard(Mutex);
            if (auto state = Tamers.find(OwnerId(owner)); state != Tamers.end())
            {
                ReactStates const reaction = me->GetReactState();
                state->second.Behaviour = reaction == REACT_AGGRESSIVE ? "aggressive" :
                                          reaction == REACT_PASSIVE ? "passive" : "defensive";
            }
        }
        if (!me->GetVictim() && !me->IsInCombat() && !BehaviourTimer)
        {
            BehaviourTimer = 1000;
            std::lock_guard<std::recursive_mutex> guard(Mutex);
            auto state = Tamers.find(OwnerId(owner));
            if (state != Tamers.end() && state->second.Behaviour != "passive")
            {
                Unit* target = state->second.Behaviour == "defensive" ? owner->getAttackerForHelper() : nullptr;
                if (state->second.Behaviour == "aggressive")
                {
                    std::vector<Unit*> candidates;
                    auto check = [&](Unit* unit)
                    {
                        if (!unit || !unit->IsCreature() || !unit->IsAlive() || unit->GetOwnerGUID() ||
                            unit->GetCharmerGUID() || !me->IsValidAttackTarget(unit) ||
                            !owner->IsValidAttackTarget(unit) || !me->IsWithinLOSInMap(unit) ||
                            !owner->IsWithinLOSInMap(unit))
                            return false;
                        return true;
                    };
                    Acore::UnitListSearcher<decltype(check)> search(me, candidates, check);
                    Cell::VisitObjects(me, search, 18.0f);
                    std::sort(candidates.begin(), candidates.end(), [this](Unit* a, Unit* b)
                    {
                        return me->GetDistance(a) < me->GetDistance(b);
                    });
                    if (!candidates.empty())
                        target = candidates.front();
                }
                if (target && target->IsCreature() && target->IsAlive() && !target->GetOwnerGUID() &&
                    !target->GetCharmerGUID() && me->IsValidAttackTarget(target) && owner->IsValidAttackTarget(target) &&
                    me->IsWithinDistInMap(target, 40.0f) && me->IsWithinLOSInMap(target) && owner->IsWithinLOSInMap(target))
                    me->AI()->AttackStart(target);
            }
        }
        if (Unit* target = me->GetVictim())
        {
            if (target->IsAlive() && me->IsValidAttackTarget(target))
                DoMeleeAttackIfReady();
            else
                EnterEvadeMode(EVADE_REASON_NO_HOSTILES);
        }
        else if (!me->IsInCombat() && !owner->IsInCombat())
        {
            if (!me->IsWithinDistInMap(owner, 60.0f))
            {
                Position const position = owner->GetNearPosition(2.0f, 0.8f);
                me->NearTeleportTo(position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                                   position.GetOrientation());
            }
            if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
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
    new DigimonQuestNPC();
    new DigimonConsumable();
    new DigimonDevice();
    new DigimonCommand();
    RegisterCreatureAI(DigimonPreviewAI);
    RegisterCreatureAI(DigimonCompanionAI);
    RegisterCreatureAI(DigimonWildAI);
    RegisterCreatureAI(DigimonLabFollowerAI);
}
