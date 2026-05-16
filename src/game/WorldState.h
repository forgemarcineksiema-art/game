#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

enum class WorldFlag {
    PowerRestored,
    ManifestCollected,
    ServiceRouteUsed,
    MaintenanceBoxInspected,
    RouteOpened,
    ExitReached,
    FerryOfficeJobStarted,
    ServiceVehicleUsed,
    DockRoadReached,
    ServiceRunConfirmed,
    FerryOfficeJobComplete,
    DockRoadRelayReset,
    DockRoadRelayLogged,
    DockRoadClearanceTagged,
    HarborPartsPickedUp,
    HarborPartsDelivered,
    FerryOfficeBoardUpdated,
    Count,
};

struct WorldEvent {
    int id = 0;
    std::string name;
    WorldFlag flag = WorldFlag::PowerRestored;
    bool value = false;
    std::string source;
};

std::string_view WorldFlagName(WorldFlag flag);
bool TryWorldFlagFromName(std::string_view name, WorldFlag& flag);

class WorldState {
public:
    void clear();

    bool isFlagSet(WorldFlag flag) const;
    bool setFlag(WorldFlag flag, bool value, std::string source);

    const std::vector<WorldEvent>& events() const;
    std::size_t eventCount() const;
    const WorldEvent* lastEvent() const;
    std::string lastEventText() const;
    std::string debugSummary() const;

private:
    static std::size_t flagIndex(WorldFlag flag);
    WorldEvent makeEvent(WorldFlag flag, bool value, std::string source);

    std::array<bool, static_cast<std::size_t>(WorldFlag::Count)> m_flags {};
    std::vector<WorldEvent> m_events;
    int m_nextEventId = 1;
};
