// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef DIGIMON_DATABASE_H
#define DIGIMON_DATABASE_H

#include "MySQLConnection.h"
#include "PreparedStatement.h"
#include <array>

namespace Digimon
{
enum Statement : uint32
{
    SelectSpecies,
    SelectTechniques,
    LearnEligibleTechniques,
    InsertTamer,
    SelectTamer,
    SelectCollection,
    InsertStarter,
    SelectStarter,
    InsertInitialActive,
    SelectActive,
    UpdateSummoned,
    SaveHealth,
    SelectLearned,
    SelectEquipped,
    DebitTechnique,
    LearnAfterDebit,
    EquipTechnique,
    CloneFormEquipment,
    UnequipTechnique,
    SaveProgress,
    RewardBits,
    BeginCaptureAttempt,
    InsertCaptured,
    SelectCaptured,
    ChangeActive,
    EnsureWildLife,
    SelectWildLife,
    ResetWildLife,
    ClaimCapturedWild,
    SelectUnlocks,
    SetTeamLead,
    SelectAliases,
    SelectSpeciesTechniques,
    SelectEvolutions,
    PruneEquipment,
    AuditLevel,
    EnsureForms,
    SelectDiscoveredForms,
    UnlockForm,
    RecordEvolutionHistory,
    SelectRequirements,
    SelectTrial,
    SelectTraining,
    StartTrial,
    SetTraining,
    CreditTrial,
    CreditAffinity,
    CompleteNativeTrial,
    ClearTraining,
    SelectNativeQuest,
    SaveNativeQuest,
    DeleteNativeQuest,
    StampTraining,
    ConsumeCatalyst,
    SelectCombatProfiles,
    SelectSpentCatalysts,
    SpendPhysicalCatalyst,
    CreditBond,
    CreditCareBond,
    SelectConsumables,
    SaveCareAndEffects,
    RecordConsumedItem,
    SelectConsumedItems,
    DeleteConsumedItem,
    SelectStorage,
    BuyStorageBox,
    AssignCapturedParty,
    SetPartyLead,
    MoveToParty,
    StoreFromParty,
    CountStorage,
    SelectTalentPoints,
    SpendTalentPoint,
    ClearTalentPoints,
    PruneTalentPoints,
    CountStatements
};

class Connection final : public MySQLConnection
{
public:
    explicit Connection(MySQLConnectionInfo& info) : MySQLConnection(info) {}
    void DoPrepareStatements() override;
    PreparedStatement<Connection>* MakeStatement(Statement index);

private:
    void Register(Statement index, std::string_view sql);
    std::array<uint8, CountStatements> _parameters{};
};
} // namespace Digimon
#endif
