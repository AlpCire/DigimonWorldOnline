// SPDX-License-Identifier: GPL-2.0-or-later
#include "../../modules/mod-digimon-tamer/src/DigimonPolicy.h"
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
    Check(ScaleXP(1000, 0.70) == 700, "Tamer XP must be 70 percent");
    Check(ScaleXP(1000, 1.0) == 1000, "Companion XP must remain independent");
    Check(ScaleXP(1, 0.7) == 0, "Small XP uses explicit floor rounding");
    Check(ScaleXP(100, -1) == 0, "Negative rates fail closed");
    Check(ScaleXP(100, std::numeric_limits<double>::infinity()) == 0, "Infinite rates fail closed");
    Check(ScaleXP(UINT32_MAX, 2) == UINT32_MAX, "XP arithmetic must saturate");
    Check(TechniqueSlots(Stage::Rookie) == 2, "Rookie has two ordinary slots");
    Check(TechniqueSlots(Stage::Champion) == 3, "Champion has three ordinary slots");
    Check(TechniqueSlots(Stage::Ultimate) == 4, "Ultimate has four ordinary slots");
    Check(TechniqueSlots(Stage::Mega) == 4, "Mega has four ordinary slots");
    Check(!CanEquip(Stage::Rookie, 2, false), "Rookie cannot equip a third technique");
    Check(!CanEquip(Stage::Mega, 4, false), "Mega extra slot is reserved for a special technique");
    Check(CanEquip(Stage::Mega, 4, true), "Mega special slot must be available");
    Check(!CanEquip(Stage::Ultimate, 4, true), "Ultimate cannot use Mega special slot");
    Progress const unchanged = GainExperience({1, 0}, 174);
    Check(unchanged.Level == 1 && unchanged.Experience == 174, "XP below threshold must persist");
    Progress const levelled = GainExperience({1, 174}, 1);
    Check(levelled.Level == 2 && levelled.Experience == 0, "Exact threshold must level once");
    Progress const multiple = GainExperience({1, 0}, 425);
    Check(multiple.Level == 3 && multiple.Experience == 0, "A reward may cross multiple levels");
    Progress const capped = GainExperience({79, 6000}, 1000);
    Check(capped.Level == 80 && capped.Experience == 0, "Progress must stop at level 80");
    Progress const extreme = GainExperience({1, UINT64_MAX}, 1000);
    Check(extreme.Level == 80 && extreme.Experience == 0, "Malformed large XP must not wrap into low XP");
    bool healthRoundTrip = true;
    for (uint32_t maximum = 1; maximum <= 1000; ++maximum)
        for (uint32_t health = 0; health <= maximum; ++health)
            healthRoundTrip = healthRoundTrip && HealthFromPercent(maximum, 100.0f * health / maximum) == health;
    Check(healthRoundTrip, "Repeated health serialization must not lose HP for maxima 1 through 1000");
    Check(HealthFromPercent(85, -10) == 0, "Negative health clamps to zero");
    Check(HealthFromPercent(85, 150) == 85, "Excess health clamps to maximum");
    Check(HealthFromPercent(85, std::numeric_limits<float>::quiet_NaN()) == 0, "Invalid health fails closed");
    std::cout << "PASS: " << Checks << " independent progression and slot policy checks\n";
}
