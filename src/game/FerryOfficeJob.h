#pragma once

#include "engine/math/Math.h"
#include "game/WorldState.h"

#include <string>
#include <string_view>

struct FerryOfficeJobConfig {
    engine::Vec3 vehicleCheckpointPosition {19.35f, 0.08f, -2.2f};
    float vehicleCheckpointRadius = 1.5f;
    std::string jobName = "Ferry Office Service Call";
};

enum class FerryOfficeJobPhase {
    CollectManifest,
    UseServiceRoute,
    RestorePower,
    OpenServiceGate,
    UseServiceVehicle,
    ReachDockRoad,
    ConfirmServiceRun,
    Complete,
};

std::string_view FerryOfficeJobPhaseName(FerryOfficeJobPhase phase);

class FerryOfficeJob {
public:
    void configure(FerryOfficeJobConfig config);

    bool recordJobStarted(WorldState& state, std::string source) const;
    bool recordServiceVehicleUsed(WorldState& state) const;
    bool updateVehicleCheckpoint(WorldState& state, engine::Vec3 vehiclePosition, bool vehicleOccupied) const;
    bool confirmServiceRun(WorldState& state, std::string source) const;

    FerryOfficeJobPhase phase(const WorldState& state) const;
    bool isReadyForConfirmation(const WorldState& state) const;
    bool isComplete(const WorldState& state) const;
    std::string currentObjectiveText(const WorldState& state) const;
    std::string debugSummary(const WorldState& state) const;

    const FerryOfficeJobConfig& config() const;

private:
    FerryOfficeJobConfig m_config;
};
