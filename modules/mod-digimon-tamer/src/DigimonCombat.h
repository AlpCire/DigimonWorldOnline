// SPDX-License-Identifier: GPL-2.0-or-later
bool ControlSpell(SpellInfo const* spell)
{
    for (auto const& effect : spell->Effects)
        if (effect.ApplyAuraName == SPELL_AURA_MOD_STUN || effect.ApplyAuraName == SPELL_AURA_MOD_FEAR ||
            effect.ApplyAuraName == SPELL_AURA_MOD_CONFUSE || effect.ApplyAuraName == SPELL_AURA_MOD_CHARM)
            return true;
    return false;
}

SpellCastResult CastProfileSpell(Creature* actor, Unit* target, uint32 id, int32 amount, uint32 duration)
{
    SpellInfo const* spell = sSpellMgr->GetSpellInfo(id);
    if (!spell || !target || !target->IsAlive())
        return SPELL_FAILED_BAD_TARGETS;
    bool const control = ControlSpell(spell);
    if (control && (target->IsPlayer() || (target->IsCreature() &&
        (target->ToCreature()->isWorldBoss() || target->ToCreature()->HasFlagsExtra(CREATURE_FLAG_EXTRA_DUNGEON_BOSS)))))
        return SPELL_FAILED_IMMUNE;
    CustomSpellValues values;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        auto const& effect = spell->Effects[i];
        if (effect.Effect == SPELL_EFFECT_SCHOOL_DAMAGE || effect.Effect == SPELL_EFFECT_HEALTH_LEECH ||
            effect.ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE || effect.ApplyAuraName == SPELL_AURA_PERIODIC_LEECH ||
            effect.ApplyAuraName == SPELL_AURA_SCHOOL_ABSORB || effect.ApplyAuraName == SPELL_AURA_MOD_DAMAGE_DONE)
            values.AddSpellMod(SpellValueMod(SPELLVALUE_BASE_POINT0 + i),
                               effect.ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE && spell->Effects[0].Effect == SPELL_EFFECT_SCHOOL_DAMAGE ? amount / 4 : amount);
    }
    SpellCastResult const result = actor->CastCustomSpell(id, values, target, TRIGGERED_FULL_MASK);
    if (result == SPELL_CAST_OK && duration)
        if (Aura* aura = target->GetAura(id, actor->GetGUID()))
        {
            int32 const limited = control ? std::min<uint32>(duration, 4000) : duration;
            aura->SetMaxDuration(limited);
            aura->SetDuration(limited);
        }
    return result;
}

bool ValidCombatTarget(Player* player, Creature* actor, Unit* target)
{
    return target && target->IsCreature() && target->IsAlive() && !target->GetOwnerGUID() &&
           !target->GetCharmerGUID() && actor->IsValidAttackTarget(target) && player->IsValidAttackTarget(target) &&
           player->IsWithinDistInMap(target, 40.0f) && player->IsWithinLOSInMap(target) && actor->IsWithinLOSInMap(target);
}

void ProfilePulse(Player* player, Creature* actor, Unit* selected, uint32 technique, CombatProfile const& p,
                  int32 amount, Position const& center)
{
    std::vector<Unit*> targets;
    if (p.Mode == "AREA" || p.Mode == "GROUND" || p.Mode == "CONE")
    {
        auto check = [&](Unit* unit)
        {
            if (!ValidCombatTarget(player, actor, unit))
                return false;
            return p.Mode == "CONE" ? actor->IsWithinDistInMap(unit, p.Range) && actor->HasInArc(float(M_PI / 2), unit) :
                                     unit->GetExactDist(center) <= p.Radius;
        };
        Acore::UnitListSearcher<decltype(check)> search(actor, targets, check);
        Cell::VisitObjects(actor, search, 40.0f);
        std::sort(targets.begin(), targets.end(), [actor](Unit* a, Unit* b) { return actor->GetDistance(a) < actor->GetDistance(b); });
        if (targets.size() > 8)
            targets.resize(8);
    }
    else if (p.Mode == "SELF" || ValidCombatTarget(player, actor, selected))
        targets.push_back(selected);
    for (Unit* target : targets)
    {
        if (p.Primary)
            CastProfileSpell(actor, target, p.Primary, amount, p.Duration);
        if (p.Secondary && (technique != 1009 || roll_chance_i(25)))
            CastProfileSpell(actor, target, p.Secondary, amount / 2, p.Duration);
    }
}

void UseProfile(Player* player, Tamer& state, Companion& companion, Creature* actor, uint32 id, CombatProfile const& p)
{
    uint64 const now = GameTime::GetGameTime().count();
    uint64 const key = (uint64(companion.Id) << 32) | id;
    if (TechniqueCooldowns[key] > now || TechniqueGlobalCooldowns[companion.Id] > now)
    {
        Notify(player, "La tecnica todavia esta recuperandose.");
        return;
    }
    bool const self = p.Mode == "SELF";
    Unit* target = self ? actor : player->GetSelectedUnit();
    if (!target || (!self && !ValidCombatTarget(player, actor, target)))
    {
        Notify(player, "Selecciona una criatura enemiga cercana y a la vista.");
        return;
    }
    if (!self && !actor->IsWithinDistInMap(target, p.Range))
    {
        actor->AI()->AttackStart(target);
        Notify(player, "El companero se acerca. Vuelve a usar la tecnica al estar a su alcance.");
        return;
    }
    if (actor->GetPower(POWER_FOCUS) < p.Cost || actor->HasUnitState(UNIT_STATE_STUNNED | UNIT_STATE_FLEEING | UNIT_STATE_CONFUSED))
    {
        Notify(player, "El companero necesita energia o esta bajo un efecto de control.");
        return;
    }
    if (!self && p.Primary && ControlSpell(sSpellMgr->GetSpellInfo(p.Primary)) &&
        (target->ToCreature()->isWorldBoss() || target->ToCreature()->HasFlagsExtra(CREATURE_FLAG_EXTRA_DUNGEON_BOSS)))
    {
        Notify(player, "Este control no afecta a jefes.");
        return;
    }
    if (!self)
    {
        actor->AI()->AttackStart(target);
        actor->SetFacingToObject(target);
    }
    int32 const amount = std::max(1, int32(p.Base + p.PerLevel * companion.Level + p.Weapon *
        (actor->GetFloatValue(UNIT_FIELD_MINDAMAGE) + actor->GetFloatValue(UNIT_FIELD_MAXDAMAGE)) / 2.0f));
    actor->ModifyPower(POWER_FOCUS, -int32(p.Cost));
    TechniqueCooldowns[key] = now + (TechniqueCatalog.at(id).CooldownMs + 999) / 1000;
    TechniqueGlobalCooldowns[companion.Id] = now + 1;
    static_cast<DigimonCombatAI*>(actor->AI())->Techniques.CancelGroup(901); // A new order cancels the previous channel, not DoTs or ground areas.
    if (p.Utility)
        CastProfileSpell(actor, actor, p.Utility, amount, p.Duration);
    if (p.Mode == "DASH" || p.Mode == "STEP")
    {
        // Native, path-checked charge; a step uses the native short teleport spell.
        actor->CastSpell(target, p.Mode == "STEP" ? 36554 : 100, TRIGGERED_FULL_MASK);
    }
    Position const center = target->GetPosition();
    Position const origin = actor->GetPosition();
    bool const delayed = p.Mode == "DELAY" || p.Mode == "DASH";
    if (!delayed)
        ProfilePulse(player, actor, target, id, p, amount, center);
    ObjectGuid const ownerGuid = player->GetGUID();
    ObjectGuid const actorGuid = actor->GetGUID();
    ObjectGuid const targetGuid = target->GetGUID();
    uint32 const companionId = companion.Id;
    for (uint8 hit = delayed ? 0 : 1; hit < p.Hits; ++hit)
    {
        uint32 const delay = p.Mode == "DELAY" ? p.Duration : p.Mode == "DASH" ? 700 :
                             (p.Mode == "GROUND" ? 2000 : p.Mode == "CHANNEL" ? 1000 : 250) * hit;
        static_cast<DigimonCombatAI*>(actor->AI())->Techniques.Schedule(std::chrono::milliseconds(delay), p.Mode == "CHANNEL" ? 901 : 902,
            [ownerGuid, actorGuid, targetGuid, companionId, id, p, amount, center, origin](TaskContext)
        {
            std::lock_guard<std::recursive_mutex> guard(Mutex);
            Player* owner = ObjectAccessor::FindPlayer(ownerGuid);
            if (!owner || !owner->IsAlive() || !Tamers.count(OwnerId(owner)))
                return;
            Tamer& current = Tamers.at(OwnerId(owner));
            Creature* caster = Actor(owner, current);
            if (!caster || !caster->IsAlive() || caster->GetGUID() != actorGuid || current.Active != companionId ||
                (p.Mode == "CHANNEL" && caster->GetExactDist(origin) > 0.75f))
                return;
            Unit* victim = ObjectAccessor::GetUnit(*caster, targetGuid);
            ProfilePulse(owner, caster, victim, id, p, amount, center);
        });
    }
    SendUI(player, "CAST|" + std::to_string(TechniqueCatalog.at(id).Spell));
    SendCombatState(player, state);
}
