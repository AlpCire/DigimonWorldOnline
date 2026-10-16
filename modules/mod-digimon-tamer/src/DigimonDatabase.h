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
