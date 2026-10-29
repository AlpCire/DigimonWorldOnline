// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef DIGIMON_TALENT_TREE_H
#define DIGIMON_TALENT_TREE_H

#include <array>
#include <cstdint>

namespace Digimon
{
struct TalentNode
{
    uint8_t Id;
    uint8_t Branch;
    uint8_t Tier;
    uint8_t Column;
    uint8_t MaxRank;
    uint8_t RequiredBranchPoints;
    uint8_t Prerequisite;
    uint8_t PrerequisiteRank;
};

// Investment ids remain stable for existing characters. Each branch has 48
// ranks; a level-100 companion earns 99 points across 144 available ranks.
inline constexpr std::array<TalentNode, 27> TalentNodes = {{
    {1, 1, 0, 0, 10, 0, 0, 0},
    {2, 2, 0, 0, 10, 0, 0, 0},
    {3, 3, 0, 0, 10, 0, 0, 0},
    {4, 1, 0, 1, 5, 0, 0, 0},
    {5, 1, 0, 2, 10, 0, 0, 0},
    {6, 1, 1, 0, 5, 5, 1, 3},
    {7, 1, 1, 2, 5, 5, 5, 3},
    {8, 1, 2, 0, 10, 10, 6, 3},
    {9, 1, 2, 2, 1, 10, 7, 3},
    {10, 1, 3, 1, 1, 15, 8, 3},
    {11, 1, 4, 1, 1, 20, 9, 1},
    {12, 2, 0, 1, 10, 0, 0, 0},
    {13, 2, 0, 2, 5, 0, 0, 0},
    {14, 2, 1, 0, 5, 5, 2, 3},
    {15, 2, 1, 2, 10, 5, 13, 3},
    {16, 2, 2, 0, 5, 10, 14, 3},
    {17, 2, 2, 2, 1, 10, 15, 3},
    {18, 2, 3, 1, 1, 15, 16, 3},
    {19, 2, 4, 1, 1, 20, 17, 1},
    {20, 3, 0, 1, 10, 0, 0, 0},
    {21, 3, 0, 2, 5, 0, 0, 0},
    {22, 3, 1, 0, 10, 5, 3, 3},
    {23, 3, 1, 2, 5, 5, 21, 3},
    {24, 3, 2, 0, 5, 10, 22, 3},
    {25, 3, 2, 2, 1, 10, 23, 3},
    {26, 3, 3, 1, 1, 15, 24, 3},
    {27, 3, 4, 1, 1, 20, 25, 1}
}};

inline constexpr TalentNode const* FindTalentNode(uint8_t id)
{
    for (TalentNode const& node : TalentNodes)
        if (node.Id == id)
            return &node;
    return nullptr;
}

// Two percent per rank, with a five-rank cap. The spell receives the final
// percentage, not the number of invested points.
inline constexpr uint8_t TamerMovementBonus(uint8_t rank)
{
    return 2u * (rank < 5u ? rank : 5u);
}

inline constexpr uint32_t SpentTalentPoints(std::array<uint8_t, 28> const& ranks)
{
    uint32_t spent = 0;
    for (TalentNode const& node : TalentNodes)
        spent += ranks[node.Id];
    return spent;
}

inline constexpr uint32_t BranchTalentPoints(std::array<uint8_t, 28> const& ranks, uint8_t branch)
{
    uint32_t spent = 0;
    for (TalentNode const& node : TalentNodes)
        if (node.Branch == branch)
            spent += ranks[node.Id];
    return spent;
}

inline constexpr bool CanInvestTalent(uint8_t level, std::array<uint8_t, 28> const& ranks, uint8_t id)
{
    TalentNode const* node = FindTalentNode(id);
    return node && level >= 2 && SpentTalentPoints(ranks) < uint32_t(level - 1) &&
           ranks[id] < node->MaxRank &&
           BranchTalentPoints(ranks, node->Branch) >= node->RequiredBranchPoints &&
           (!node->Prerequisite || ranks[node->Prerequisite] >= node->PrerequisiteRank);
}

inline constexpr bool TalentTechniqueUnlocked(std::array<uint8_t, 28> const& ranks, uint32_t technique)
{
    switch (technique)
    {
    case 2000:
        return ranks[9] >= 1;
    case 2001:
        return ranks[17] >= 1;
    case 2002:
        return ranks[3] >= 5;
    case 2003:
        return ranks[11] >= 1;
    case 2004:
        return ranks[19] >= 1 || ranks[27] >= 1;
    case 2005:
        return ranks[25] >= 1;
    default:
        return true;
    }
}

inline constexpr bool IsTalentTechnique(uint32_t technique)
{
    return technique >= 2000 && technique <= 2005;
}
} // namespace Digimon
#endif
