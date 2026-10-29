// SPDX-License-Identifier: GPL-2.0-or-later
// Included inside namespace Digimon after the existing companion helpers.
struct TrialProgress
{
    uint16 Victories = 0;
    bool Completed = false;
    uint16 Catalysts = 0;
    uint16 Affinity = 0;
};

TrialProgress ReadTrial(Companion const& companion, uint32 target, RouteRequirement const& requirement)
{
    TrialProgress progress;
    if (auto row = Query(SelectTrial, requirement.Affinity, companion.Id, target))
    {
        Field* f = row->Fetch();
        progress = {f[0].Get<uint16>(), f[1].Get<uint8>() != 0, f[2].Get<uint16>(), f[3].Get<uint16>()};
    }
    return progress;
}

void CleanSpentCatalysts(Player* player)
{
    if (auto rows = Query(SelectSpentCatalysts, OwnerId(player)))
        do
        {
            if (Item* item = player->GetItemByGuid(ObjectGuid::Create<HighGuid::Item>(rows->Fetch()[0].Get<uint32>())))
                player->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
        } while (rows->NextRow());
}

Item* FindCatalyst(Player* player, uint32 entry)
{
    CleanSpentCatalysts(player);
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot); item && item->GetEntry() == entry)
            return item;
    for (uint8 bagSlot = INVENTORY_SLOT_BAG_START; bagSlot < INVENTORY_SLOT_BAG_END; ++bagSlot)
        if (Bag* bag = player->GetBagByPos(bagSlot))
            for (uint32 slot = 0; slot < bag->GetBagSize(); ++slot)
                if (Item* item = bag->GetItemByPos(slot); item && item->GetEntry() == entry)
                    return item;
    return nullptr;
}

void ClaimCatalyst(Player* player, Tamer& state, uint32 target)
{
    Companion* companion = Active(state);
    if (!companion || Busy(player, state) || !player->IsAlive())
        return;
    auto req = Requirements.find({companion->SpeciesId, target});
    if (req == Requirements.end() || !req->second.Catalysts)
        return;
    TrialProgress const progress = ReadTrial(*companion, target, req->second);
    if (!progress.Completed || !progress.Catalysts)
        return;
    ItemPosCountVec destination;
    Item* item = nullptr;
    if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, destination, req->second.CatalystItem, 1) == EQUIP_ERR_OK)
        item = player->StoreNewItem(destination, req->second.CatalystItem, true);
    if (!item)
    {
        Notify(player, "Deja un espacio libre en las mochilas para el catalizador.");
        return;
    }
    auto transaction = CharacterDatabase.BeginTransaction();
    // Only this new, non-stackable item is saved here. Other pending inventory changes remain queued.
    auto* inventory = CharacterDatabase.GetPreparedStatement(CHAR_REP_INVENTORY_ITEM);
    inventory->SetData(0, OwnerId(player));
    inventory->SetData(1, item->GetContainer() ? item->GetContainer()->GetGUID().GetCounter() : 0u);
    inventory->SetData(2, item->GetSlot());
    inventory->SetData(3, item->GetGUID().GetCounter());
    transaction->Append(inventory);
    item->RemoveFromUpdateQueueOf(player);
    item->SaveToDB(transaction);
    // Module and core inventory records share the characters database. Values are typed server IDs.
    transaction->Append("INSERT INTO digimon_catalyst_grants(companion_id,target_species,item_guid) VALUES({},{},{})",
                        companion->Id, target, item->GetGUID().GetCounter());
    transaction->Append("UPDATE digimon_trials SET catalysts=0 WHERE companion_id={} AND target_species={}",
                        companion->Id, target);
    auto result = CharacterDatabase.AsyncCommitTransaction(transaction);
    if (!result.m_future.get())
    {
        player->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
        Notify(player, "No se pudo entregar el catalizador. La recompensa sigue pendiente.");
        return;
    }
    Notify(player, "Catalizador recibido en la mochila. Se consumira al descubrir la evolucion; las formas conocidas no lo gastan.");
}

bool TrialReady(Player* player, Companion const& companion, uint32 target)
{
    auto requirement = Requirements.find({companion.SpeciesId, target});
    if (requirement == Requirements.end())
        return true;
    TrialProgress const p = ReadTrial(companion, target, requirement->second);
    return p.Completed && (!requirement->second.Catalysts || FindCatalyst(player, requirement->second.CatalystItem)) && p.Affinity >= requirement->second.Points;
}

void StartTraining(Player* player, ObjectGuid source, Tamer& state, uint32 target)
{
    Companion* companion = Active(state);
    if (!companion || Busy(player, state) || !player->IsAlive())
        return;
    auto requirement = Requirements.find({companion->SpeciesId, target});
    auto routes = Evolutions.find(companion->SpeciesId);
    if (requirement == Requirements.end() || routes == Evolutions.end() || !Catalog.count(target) ||
        !Catalog.at(target).Enabled || companion->Forms.count(target) ||
        std::none_of(routes->second.begin(), routes->second.end(),
                     [target](Evolution const& route) { return route.Target == target; }))
        return;
    uint32 const questId = NativeRouteQuest(target);
    Quest const* quest = questId ? sObjectMgr->GetQuestTemplate(questId) : nullptr;
    if (!quest)
    {
        Notify(player, "La misión nativa de esta evolución todavía no está instalada.");
        return;
    }
    if (player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
    {
        Notify(player, "Ya tienes esta misión en el registro.");
        return;
    }
    if (player->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
    {
        player->PlayerTalkClass->SendQuestGiverRequestItems(quest, source, true, true);
        return;
    }
    player->PlayerTalkClass->SendQuestGiverQuestDetails(quest, source, true);
}

bool CreditTraining(Companion const& companion, Creature const& defeated,
                    std::shared_ptr<Transaction<Connection>> const& transaction, uint32& creditedTarget)
{
    auto training = Query(SelectTraining, companion.Id);
    if (!training)
        return false;
    Field* f = training->Fetch();
    uint32 const target = f[0].Get<uint32>();
    auto requirement = Requirements.find({companion.SpeciesId, target});
    if (requirement == Requirements.end() || companion.Forms.count(target))
        return false;
    RouteRequirement const& r = requirement->second;
    if (r.MissionCreatureEntry && defeated.GetEntry() != r.MissionCreatureEntry)
        return false;
    if (!r.MissionCreatureEntry)
        return false;
    transaction->Append(Make(CreditTrial, r.Victories, r.Victories, r.Victories, companion.Id, target));
    transaction->Append(Make(CreditAffinity, companion.Id, r.Affinity));
    creditedTarget = target;
    return true;
}

void SendV2(Player* player, std::string const& message)
{
    if (message.size() > 245)
        return;
    WorldPacket packet;
    ChatHandler::BuildChatPacket(packet, CHAT_MSG_WHISPER, LANG_ADDON, player->GetGUID(), player->GetGUID(),
                                "WDMMO\tV2|" + message, 0);
    player->GetSession()->SendPacket(&packet);
}

uint32 RouteDisplay(uint32 speciesId)
{
    auto species = Catalog.find(speciesId);
    if (species == Catalog.end())
        return 0;
    CreatureTemplate const* creature = sObjectMgr->GetCreatureTemplate(species->second.Entry);
    CreatureModel const* model = creature ? creature->GetFirstValidModel() : nullptr;
    return model ? model->CreatureDisplayID : 0;
}

void SendActiveMission(Player* player, Tamer& state)
{
    Companion* companion = Active(state);
    auto training = companion ? Query(SelectTraining, companion->Id) : nullptr;
    if (!training || !companion)
    {
        SendV2(player, "ACTIVE_MISSION|0");
        return;
    }
    uint32 const target = training->Fetch()[0].Get<uint32>();
    auto requirement = Requirements.find({companion->SpeciesId, target});
    if (requirement == Requirements.end())
    {
        SendV2(player, "ACTIVE_MISSION|0");
        return;
    }
    TrialProgress const progress = ReadTrial(*companion, target, requirement->second);
    CreatureTemplate const* objective = sObjectMgr->GetCreatureTemplate(requirement->second.MissionCreatureEntry);
    std::string targetName = objective ? objective->Name : "objetivo exacto";
    if (requirement->second.MissionCreatureEntry == 705)
        targetName = "Lobo joven";
    SendV2(player, "ACTIVE_MISSION|" + std::to_string(target) + "|" + requirement->second.Mission + "|" +
        std::to_string(progress.Victories) + "|" + std::to_string(requirement->second.Victories) + "|" +
        targetName + "|" + (progress.Completed ? "1" : "0"));
}

void SendRoutes(Player* player, Tamer& state)
{
    SendV2(player, "ROUTES_BEGIN");
    Companion* companion = Active(state);
    if (companion)
    {
        // Show the incoming edge as well so the Digivice displays the lineage
        // (for example Pagumon -> Impmon) alongside the current branch choices.
        for (auto const& [source, routes] : Evolutions)
            for (Evolution const& route : routes)
                if (route.Target == companion->SpeciesId && Catalog.count(source))
                    SendV2(player, "ANCESTOR|" + std::to_string(source) + "|" + Catalog.at(source).Name + "|" +
                                  std::to_string(companion->SpeciesId) + "|" + Catalog.at(companion->SpeciesId).Name +
                                  "|" + std::to_string(route.Level));
        if (auto routes = Evolutions.find(companion->SpeciesId); routes != Evolutions.end())
            for (Evolution const& route : routes->second)
            {
                if (!Catalog.count(route.Target) || companion->Forms.count(route.Target))
                    continue;
                auto req = Requirements.find({companion->SpeciesId, route.Target});
                std::string missing = "Nivel " + std::to_string(companion->Level) + "/" + std::to_string(route.Level);
                bool ready = Catalog.at(route.Target).Enabled && companion->Level >= route.Level && !Busy(player, state);
                if (req != Requirements.end())
                {
                    TrialProgress const p = ReadTrial(*companion, route.Target, req->second);
                    ready = ready && TrialReady(player, *companion, route.Target);
                    missing += " / " + req->second.Affinity + " " + std::to_string(p.Affinity) + "/" + std::to_string(req->second.Points);
                    missing += " / Victorias " + std::to_string(p.Victories) + "/" + std::to_string(req->second.Victories);
                    missing += " / Nucleo " + std::to_string(FindCatalyst(player, req->second.CatalystItem) ? 1 : 0) + "/" + std::to_string(req->second.Catalysts);
                }
                SendV2(player, "ROUTE|" + std::to_string(route.Target) + "|" + std::to_string(route.Level) + "|" +
                              Catalog.at(route.Target).Name + "|" + (ready ? "READY" : "LOCKED") + "|" + missing +
                              "|" + std::to_string(RouteDisplay(route.Target)));
                if (req != Requirements.end())
                {
                    SendV2(player, "MISSION|" + std::to_string(route.Target) + "|" + req->second.Mission);
                    TrialProgress const p = ReadTrial(*companion, route.Target, req->second);
                    SendV2(player, "REWARD|" + std::to_string(route.Target) + "|" +
                        std::to_string(p.Completed && p.Catalysts && req->second.Catalysts) + "|" + std::to_string(req->second.CatalystItem));
                }
            }
        for (uint32 species : companion->Forms)
            if (species != companion->SpeciesId && Catalog.count(species) && Catalog.at(species).Enabled)
                SendV2(player, "ROUTE|" + std::to_string(species) + "|1|" + Catalog.at(species).Name +
                              "|REUSE|Forma descubierta: conserva nivel, experiencia y tecnicas aprendidas.|" +
                              std::to_string(RouteDisplay(species)));
    }
    SendV2(player, "ROUTES_END");
    SendActiveMission(player, state);
}

void NotifyMissionProgress(Player* player, Tamer& state, uint32 target)
{
    Companion* companion = Active(state);
    if (!companion)
        return;
    auto requirement = Requirements.find({companion->SpeciesId, target});
    if (requirement == Requirements.end())
        return;
    TrialProgress const progress = ReadTrial(*companion, target, requirement->second);
    Notify(player, requirement->second.Mission + ": " + std::to_string(progress.Victories) + "/" +
                  std::to_string(requirement->second.Victories) + ".");
    if (state.Addon)
        SendActiveMission(player, state);
}

void SendTalents(Player* player, Tamer& state)
{
    Companion const* companion = Active(state);
    if (!companion)
    {
        std::string empty = "TALENTS|0|0|DPS|0|0|0";
        for (uint8 i = 1; i <= TalentNodes.size(); ++i)
            empty += "|0";
        SendV2(player, empty);
        return;
    }
    std::string message = "TALENTS|" + std::to_string(companion->Id) + "|" +
                          std::to_string(companion->SpeciesId) + "|" + companion->Role + "|" +
                          std::to_string(uint8(companion->Profile)) + "|" +
                          std::to_string(companion->Level) + "|" +
                          std::to_string(SpentTalentPoints(companion->Talents));
    for (TalentNode const& node : TalentNodes)
        message += "|" + std::to_string(companion->Talents[node.Id]);
    SendV2(player, message);
}

void SendDetails(Player* player, Tamer& state)
{
    Companion* companion = Active(state);
    SendV2(player, "DETAILS_BEGIN");
    for (Companion const& c : state.Companions)
        if (c.PartySlot < 6)
            if (auto species = Catalog.find(c.SpeciesId); species != Catalog.end())
        {
            uint32 display = 0;
            if (auto creature = sObjectMgr->GetCreatureTemplate(species->second.Entry))
                if (auto model = creature->GetFirstValidModel())
                    display = model->CreatureDisplayID;
            SendV2(player, "COMPANION|" + std::to_string(c.Id) + "|" + std::to_string(c.SpeciesId) + "|" +
                          species->second.Name + "|" + std::to_string(uint8(c.Evolution)) + "|" + std::to_string(c.Level) +
                          "|" + std::to_string(c.Experience) + "|" + std::to_string(display) + "|" +
                          std::to_string(species->second.Entry) + "|" + std::to_string(c.Id == state.Active) + "|" +
                          std::to_string(c.Bond) + "|" + std::to_string(c.Happiness) + "|" + std::to_string(c.PartySlot));
        }
    if (companion)
        for (auto const& [id, t] : TechniqueCatalog)
            if (TechniqueAllowed(companion->SpeciesId, id) && t.Enabled)
            {
                std::string status = IsEquipped(*companion, id) ? "EQUIPPED" :
                    companion->Learned.count(id) && companion->Level >= t.RequiredLevel &&
                    TalentTechniqueUnlocked(companion->Talents, id) ? "LEARNED" : "LOCKED";
                SendV2(player, "TECH|" + std::to_string(id) + "|" + TechniqueDisplayName(t) + "|" +
                              std::to_string(t.Spell) + "|" + std::to_string(t.RequiredLevel) + "|" + status + "|" +
                              std::to_string(t.CooldownMs / 1000));
            }
    SendV2(player, "DETAILS_END");
    SendTalents(player, state);
}

void SendStorage(Player* player, Tamer const& state, uint16 box)
{
    box = std::clamp<uint16>(box, 1, std::max<uint16>(1, state.StorageBoxes));
    uint64 const price = state.StorageBoxes > 1 ? 10000ULL * (state.StorageBoxes - 1) * (state.StorageBoxes - 1) : 0;
    SendV2(player, "STORAGE|" + std::to_string(state.StorageBoxes) + "|" + std::to_string(box) + "|" +
                  std::to_string(player->GetMoney()) + "|" + std::to_string(player->GetItemCount(900240)) + "|" +
                  std::to_string(price));
    uint32 const offset = uint32(box - 1) * 15;
    if (auto rows = Query(SelectStorage, OwnerId(player), offset))
        do
        {
            Field* fields = rows->Fetch();
            uint32 const speciesId = fields[1].Get<uint32>();
            auto species = Catalog.find(speciesId);
            if (species == Catalog.end())
                continue;
            uint32 display = 0;
            if (auto creature = sObjectMgr->GetCreatureTemplate(species->second.Entry))
                if (auto model = creature->GetFirstValidModel())
                    display = model->CreatureDisplayID;
            SendV2(player, "BOX_ITEM|" + std::to_string(fields[0].Get<uint32>()) + "|" +
                          std::to_string(speciesId) + "|" + species->second.Name + "|" +
                          std::to_string(fields[2].Get<uint8>()) + "|" + std::to_string(fields[3].Get<uint8>()) + "|" +
                          std::to_string(fields[4].Get<uint64>()) + "|" + std::to_string(display));
        } while (rows->NextRow());
    SendV2(player, "BOX_END");
}
