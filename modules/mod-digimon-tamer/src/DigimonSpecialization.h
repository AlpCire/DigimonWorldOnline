// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef DIGIMON_SPECIALIZATION_H
#define DIGIMON_SPECIALIZATION_H

#include "DigimonTalentTree.h"
#include <string_view>

namespace Digimon
{
enum class SpecializationProfile : uint8_t
{
    Basic,
    Impmon,
    Arcane,
    Shadow,
    Spectral,
    Hex,
    Gun,
    Angel
};

inline constexpr SpecializationProfile ProfileFromCode(std::string_view code)
{
    if (code == "IMPMON") return SpecializationProfile::Impmon;
    if (code == "ARCANE") return SpecializationProfile::Arcane;
    if (code == "SHADOW") return SpecializationProfile::Shadow;
    if (code == "SPECTRAL") return SpecializationProfile::Spectral;
    if (code == "HEX") return SpecializationProfile::Hex;
    if (code == "GUN") return SpecializationProfile::Gun;
    if (code == "ANGEL") return SpecializationProfile::Angel;
    return SpecializationProfile::Basic;
}

// Signature nodes 1 and 8 belong exclusively to the currently selected form.
// This function receives one form's ranks, never a sum of previous forms.
inline constexpr uint32_t FormTalentDamageBonus(SpecializationProfile profile,
    std::array<uint8_t, 28> const& ranks, bool techniqueDamage, bool targetBelowHalf,
    bool actorAboveThreeQuarters, bool focusBelowHalf, bool focusAtLeastHalf)
{
    uint32_t bonus = 0;
    switch (profile)
    {
    case SpecializationProfile::Angel:
        break;
    case SpecializationProfile::Arcane:
        if (techniqueDamage)
            bonus = 3u * ranks[1] + (targetBelowHalf ? 2u * ranks[8] : 0u);
        break;
    case SpecializationProfile::Shadow:
        bonus = 2u * ranks[1] + (targetBelowHalf ? 3u * ranks[8] : 0u);
        break;
    case SpecializationProfile::Spectral:
        bonus = (actorAboveThreeQuarters ? 2u * ranks[1] : 0u) +
                (!targetBelowHalf ? 2u * ranks[8] : 0u);
        break;
    case SpecializationProfile::Hex:
        bonus = (focusBelowHalf ? 2u * ranks[1] : 0u) +
                (!targetBelowHalf ? 2u * ranks[8] : 0u);
        break;
    case SpecializationProfile::Gun:
        bonus = 2u * ranks[1] + (focusAtLeastHalf ? 2u * ranks[8] : 0u);
        break;
    default:
        bonus = 2u * ranks[1] + (targetBelowHalf ? 2u * ranks[8] : 0u);
        break;
    }
    return bonus;
}

inline constexpr uint32_t FormTalentHealingBonus(SpecializationProfile profile,
    std::array<uint8_t, 28> const& ranks, bool targetBelowHalf)
{
    if (profile != SpecializationProfile::Angel)
        return 0;
    return 2u * ranks[1] + (targetBelowHalf ? 2u * ranks[8] : 0u);
}
} // namespace Digimon
#endif
