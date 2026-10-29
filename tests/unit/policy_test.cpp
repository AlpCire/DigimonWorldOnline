// SPDX-License-Identifier: GPL-2.0-or-later
#include "../../modules/mod-digimon-tamer/src/DigimonPolicy.h"
#include "../../modules/mod-digimon-tamer/src/DigimonTalentTree.h"
#include "../../modules/mod-digimon-tamer/src/DigimonSpecialization.h"
#include <iostream>
#include <stdexcept>

static unsigned Checks = 0;
static void Check(bool condition, char const* message)
{
    ++Checks;
    if (!condition)
        throw std::runtime_error(message);
}

int main()
{
    using namespace Digimon;
    std::array<uint8_t, 28> impmonRanks{};
    impmonRanks[1] = 4;
    Check(FormTalentDamageBonus(SpecializationProfile::Impmon, impmonRanks,
        false, false, true, false, true) == 8,
        "Impmon four ranks grant eight percent, once");
    std::array<uint8_t, 28> devimonRanks{};
    devimonRanks[1] = 4;
    Check(FormTalentDamageBonus(SpecializationProfile::Shadow, devimonRanks,
        false, false, true, false, true) == 8,
        "Devimon must use only its own eight percent after evolution");
    Check(FormTalentDamageBonus(SpecializationProfile::Impmon, impmonRanks,
        false, false, true, false, true) == 8,
        "Returning to Impmon restores its original bonus without stacking");
    Check(FormTalentDamageBonus(SpecializationProfile::Arcane, devimonRanks,
        false, false, true, false, true) == 0,
        "Wizardmon signature affects techniques only");
    Check(FormTalentDamageBonus(SpecializationProfile::Arcane, devimonRanks,
        true, false, true, false, true) == 12,
        "Wizardmon signature boosts technique damage");
    std::array<uint8_t, 28> angemonRanks{};
    angemonRanks[1] = 4;
    Check(FormTalentDamageBonus(SpecializationProfile::Angel, angemonRanks,
        true, true, true, false, true) == 0,
        "Angemon healing signature does not add damage");
    Check(FormTalentHealingBonus(SpecializationProfile::Angel, angemonRanks, false) == 8,
        "Angemon signature grants healing to its active form");
    Check(ProfileFromCode("ANGEL") == SpecializationProfile::Angel,
        "Angemon profile loads from database code");
    Check(ProfileFromCode("UNKNOWN") == SpecializationProfile::Basic,
        "New unspecialized forms remain safe while Patamon line expands");
    Check(ScaleXP(1000, 0.70) == 700, "Tamer XP must be 70 percent");
    Check(ScaleXP(1000, 1.0) == 1000, "Companion XP must remain independent");
    Check(ScaleXP(1, 0.7) == 0, "Small XP uses explicit floor rounding");
    Check(ScaleXP(100, -1) == 0, "Negative rates fail closed");
    Check(ScaleXP(100, std::numeric_limits<double>::infinity()) == 0, "Infinite rates fail closed");
    Check(ScaleXP(UINT32_MAX, 2) == UINT32_MAX, "XP arithmetic must saturate");
    Check(TechniqueSlots(Stage::Rookie) == 3, "Rookie has three ordinary slots");
    Check(TechniqueSlots(Stage::Champion) == 4, "Champion has four ordinary slots");
    Check(TechniqueSlots(Stage::Ultimate) == 4, "Ultimate has four ordinary slots");
    Check(TechniqueSlots(Stage::Mega) == 4, "Mega has four ordinary slots");
    Check(CanEquip(Stage::Rookie, 2, false), "Rookie can equip a third technique");
    Check(!CanEquip(Stage::Rookie, 3, false), "Rookie cannot equip a fourth technique");
    Check(!CanEquip(Stage::Mega, 4, false), "Mega extra slot is reserved for a special technique");
    Check(CanEquip(Stage::Mega, 4, true), "Mega special slot must be available");
    Check(!CanEquip(Stage::Ultimate, 4, true), "Ultimate cannot use Mega special slot");
    Progress const unchanged = GainExperience({1, 0}, 174);
    Check(unchanged.Level == 1 && unchanged.Experience == 174, "XP below threshold must persist");
    Progress const levelled = GainExperience({1, 174}, 1);
    Check(levelled.Level == 2 && levelled.Experience == 0, "Exact threshold must level once");
    Progress const multiple = GainExperience({1, 0}, 425);
    Check(multiple.Level == 3 && multiple.Experience == 0, "A reward may cross multiple levels");
    Progress const levelHundred = GainExperience({99, 7524}, 1);
    Check(levelHundred.Level == 100 && levelHundred.Experience == 0, "Level 100 is reachable exactly");
    Progress const capped = GainExperience({100, 6000}, 1000);
    Check(capped.Level == 100 && capped.Experience == 0, "Progress must stop at level 100");
    Progress const extreme = GainExperience({1, UINT64_MAX}, 1000);
    Check(extreme.Level == 100 && extreme.Experience == 0, "Malformed large XP must not wrap into low XP");
    bool healthRoundTrip = true;
    for (uint32_t maximum = 1; maximum <= 1000; ++maximum)
        for (uint32_t health = 0; health <= maximum; ++health)
            healthRoundTrip = healthRoundTrip && HealthFromPercent(maximum, 100.0f * health / maximum) == health;
    Check(healthRoundTrip, "Repeated health serialization must not lose HP for maxima 1 through 1000");
    Check(HealthFromPercent(85, -10) == 0, "Negative health clamps to zero");
    Check(HealthFromPercent(85, 150) == 85, "Excess health clamps to maximum");
    Check(HealthFromPercent(85, std::numeric_limits<float>::quiet_NaN()) == 0, "Invalid health fails closed");
    Check(!EvolutionReady(21, 22, Stage::Rookie, Stage::Champion), "Patamon must stay Rookie at level 21");
    Check(EvolutionReady(22, 22, Stage::Rookie, Stage::Champion), "Patamon can become Angemon at 22");
    Check(EvolutionReady(30, 22, Stage::Rookie, Stage::Champion), "Level jumps must cross the evolution threshold");
    Check(!EvolutionReady(22, 22, Stage::Champion, Stage::Rookie), "Evolution must never lower a stage");
    Check(EvolutionReady(10, 10, Stage::Rookie, Stage::Rookie), "Same-stage forms can evolve when explicitly routed");
    Check(!EvolutionReady(22, 1, Stage::Rookie, Stage::Champion), "Reject invalid evolution threshold");
    Check(EvolutionReady(100, 100, Stage::Ultimate, Stage::Mega), "Level 100 evolution requirements are valid");
    Check(!EvolutionReady(100, 101, Stage::Ultimate, Stage::Mega), "Evolution thresholds cannot exceed level cap");
    Check(DevolutionReady(21, 22, Stage::Champion, Stage::Rookie), "Angemon returns to Patamon below 22");
    Check(!DevolutionReady(22, 22, Stage::Champion, Stage::Rookie), "Angemon remains Angemon at 22");
    Check(DevolutionReady(1, 22, Stage::Champion, Stage::Rookie), "A downward level jump crosses the threshold");
    Check(!DevolutionReady(21, 22, Stage::Rookie, Stage::Champion), "Devolution cannot raise a stage");
    Check(!DevolutionReady(1, 1, Stage::Champion, Stage::Rookie), "Invalid reverse threshold is rejected");
    Check(HasUniqueEvolutionChoice(1), "A single eligible branch can evolve automatically");
    Check(!HasUniqueEvolutionChoice(2), "Multiple eligible branches must await a player choice");
    Check(!HasUniqueEvolutionChoice(0), "No eligible branch must leave the form unchanged");
    Progress const evolving = GainExperience({21, 1674}, 2);
    Check(evolving.Level == 22 && evolving.Experience == 1, "Evolution must preserve level and overflow XP");
    std::array<uint8_t, 28> ranks{};
    Check(TamerMovementBonus(0) == 0 && TamerMovementBonus(1) == 2 &&
          TamerMovementBonus(5) == 10 && TamerMovementBonus(6) == 10,
          "Tamer movement talent grants two percent per rank, capped at ten");
    Check(TalentNodes.size() == 27, "Three complete talent branches are defined");
    Check(!CanInvestTalent(1, ranks, 1), "No talent point before level two");
    Check(CanInvestTalent(2, ranks, 1), "Level two grants the first point");
    Check(!CanInvestTalent(2, ranks, 6), "Second tier requires branch investment");
    ranks[1] = 5;
    Check(!CanInvestTalent(6, ranks, 6), "Budget cannot exceed level minus one");
    Check(CanInvestTalent(7, ranks, 6), "Second tier opens at five points and prerequisite rank");
    Check(!CanInvestTalent(7, ranks, 9), "Third tier remains locked");
    ranks[4] = 5;
    Check(!CanInvestTalent(12, ranks, 9), "A missing prerequisite blocks an unlocked tier");
    ranks[7] = 3;
    Check(CanInvestTalent(15, ranks, 9), "Prerequisite opens the active technique");
    Check(!TalentTechniqueUnlocked(ranks, 2000), "Technique stays locked until allocated");
    ranks[9] = 1;
    Check(TalentTechniqueUnlocked(ranks, 2000), "Technique unlock follows allocation");
    Check(!CanInvestTalent(100, ranks, 9), "Single-rank nodes cannot overfill");
    ranks[19] = 1;
    Check(TalentTechniqueUnlocked(ranks, 2004), "Guard barrier unlocks shared technique");
    ranks[19] = 0; ranks[27] = 1;
    Check(TalentTechniqueUnlocked(ranks, 2004), "Support barrier also unlocks shared technique");
    ranks[3] = 5;
    Check(TalentTechniqueUnlocked(ranks, 2002), "Restoration retains existing heal unlock");
    std::cout << "PASS: " << Checks << " independent progression, evolution and slot policy checks\n";
}
