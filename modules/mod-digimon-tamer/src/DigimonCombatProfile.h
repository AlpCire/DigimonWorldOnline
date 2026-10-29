// SPDX-License-Identifier: GPL-2.0-or-later
struct CombatProfile
{
    std::string Mode;
    uint32 Primary;
    uint32 Secondary;
    uint32 Utility;
    uint8 Hits;
    uint32 Duration;
    float Range;
    float Radius;
    float Base;
    float PerLevel;
    float Weapon;
    uint8 Cost;
};
std::unordered_map<uint32, CombatProfile> CombatProfiles;
std::unordered_map<uint32, uint64> TechniqueGlobalCooldowns;


struct DigimonCombatAI : ScriptedAI
{
    explicit DigimonCombatAI(Creature* creature) : ScriptedAI(creature) {}
    TaskScheduler Techniques;
};
