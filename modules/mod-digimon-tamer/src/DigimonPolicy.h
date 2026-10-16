// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef DIGIMON_POLICY_H
#define DIGIMON_POLICY_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace Digimon
{
enum class Stage : uint8_t
{
    Rookie = 1,
    Champion = 2,
    Ultimate = 3,
    Mega = 4
};

inline uint8_t TechniqueSlots(Stage stage)
{
    switch (stage)
    {
    case Stage::Rookie:
        return 2;
    case Stage::Champion:
        return 3;
    case Stage::Ultimate:
        return 4;
    case Stage::Mega:
        return 4;
    }
    return 0;
}

inline uint32_t ScaleXP(uint32_t amount, double rate)
{
    if (!std::isfinite(rate) || rate < 0.0)
        return 0;
    return static_cast<uint32_t>(std::min<double>(std::numeric_limits<uint32_t>::max(),
                                                  std::floor(static_cast<double>(amount) * rate + 0.00000001)));
}

inline bool CanEquip(Stage stage, uint8_t slot, bool special)
{
    if (special)
        return stage == Stage::Mega && slot == 4;
    return slot < TechniqueSlots(stage);
}

inline uint32_t HealthFromPercent(uint32_t maximum, float percent)
{
    if (!std::isfinite(percent))
        return 0;
    // Round saved percentages back to health points. Flooring loses one HP
    // repeatedly when float serialization lands just below the original value.
    return static_cast<uint32_t>(std::llround(maximum * double(std::clamp(percent, 0.0f, 100.0f)) / 100.0));
}

struct Progress
{
    uint8_t Level;
    uint64_t Experience;
};

inline Progress GainExperience(Progress progress, uint32_t amount, uint8_t maxLevel = 80)
{
    progress.Experience = amount > std::numeric_limits<uint64_t>::max() - progress.Experience
                              ? std::numeric_limits<uint64_t>::max()
                              : progress.Experience + amount;
    while (progress.Level < maxLevel)
    {
        uint64_t const required = 100u + 75u * progress.Level;
        if (progress.Experience < required)
            break;
        progress.Experience -= required;
        ++progress.Level;
    }
    if (progress.Level >= maxLevel)
        progress.Experience = 0;
    return progress;
}
} // namespace Digimon
#endif
