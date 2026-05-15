#include "game/WorldState.h"

#include <sstream>
#include <utility>

std::string_view WorldFlagName(WorldFlag flag)
{
    switch (flag) {
    case WorldFlag::PowerRestored:
        return "powerRestored";
    case WorldFlag::ManifestCollected:
        return "manifestCollected";
    case WorldFlag::ServiceRouteUsed:
        return "serviceRouteUsed";
    case WorldFlag::MaintenanceBoxInspected:
        return "maintenanceBoxInspected";
    case WorldFlag::RouteOpened:
        return "routeOpened";
    case WorldFlag::ExitReached:
        return "exitReached";
    case WorldFlag::FerryOfficeJobStarted:
        return "ferryOfficeJobStarted";
    case WorldFlag::ServiceVehicleUsed:
        return "serviceVehicleUsed";
    case WorldFlag::DockRoadReached:
        return "dockRoadReached";
    case WorldFlag::ServiceRunConfirmed:
        return "serviceRunConfirmed";
    case WorldFlag::FerryOfficeJobComplete:
        return "ferryOfficeJobComplete";
    case WorldFlag::Count:
    default:
        return "unknown";
    }
}

void WorldState::clear()
{
    m_flags = {};
    m_events.clear();
    m_nextEventId = 1;
}

bool WorldState::isFlagSet(WorldFlag flag) const
{
    return m_flags[flagIndex(flag)];
}

bool WorldState::setFlag(WorldFlag flag, bool value, std::string source)
{
    const std::size_t index = flagIndex(flag);
    if (m_flags[index] == value) {
        return false;
    }

    m_flags[index] = value;
    m_events.push_back(makeEvent(flag, value, std::move(source)));
    return true;
}

const std::vector<WorldEvent>& WorldState::events() const
{
    return m_events;
}

std::size_t WorldState::eventCount() const
{
    return m_events.size();
}

const WorldEvent* WorldState::lastEvent() const
{
    if (m_events.empty()) {
        return nullptr;
    }

    return &m_events.back();
}

std::string WorldState::lastEventText() const
{
    const WorldEvent* event = lastEvent();
    if (!event) {
        return "none";
    }

    std::ostringstream output;
    output << "#" << event->id << " " << event->name;
    if (!event->source.empty()) {
        output << " from " << event->source;
    }
    return output.str();
}

std::string WorldState::debugSummary() const
{
    std::ostringstream output;
    output << WorldFlagName(WorldFlag::PowerRestored) << "=" << (isFlagSet(WorldFlag::PowerRestored) ? "true" : "false")
           << " " << WorldFlagName(WorldFlag::ManifestCollected) << "=" << (isFlagSet(WorldFlag::ManifestCollected) ? "true" : "false")
           << " " << WorldFlagName(WorldFlag::ServiceRouteUsed) << "=" << (isFlagSet(WorldFlag::ServiceRouteUsed) ? "true" : "false")
           << " " << WorldFlagName(WorldFlag::MaintenanceBoxInspected) << "=" << (isFlagSet(WorldFlag::MaintenanceBoxInspected) ? "true" : "false")
           << " " << WorldFlagName(WorldFlag::RouteOpened) << "=" << (isFlagSet(WorldFlag::RouteOpened) ? "true" : "false")
           << " " << WorldFlagName(WorldFlag::ExitReached) << "=" << (isFlagSet(WorldFlag::ExitReached) ? "true" : "false")
           << " " << WorldFlagName(WorldFlag::FerryOfficeJobStarted) << "=" << (isFlagSet(WorldFlag::FerryOfficeJobStarted) ? "true" : "false")
           << " " << WorldFlagName(WorldFlag::ServiceVehicleUsed) << "=" << (isFlagSet(WorldFlag::ServiceVehicleUsed) ? "true" : "false")
           << " " << WorldFlagName(WorldFlag::DockRoadReached) << "=" << (isFlagSet(WorldFlag::DockRoadReached) ? "true" : "false")
           << " " << WorldFlagName(WorldFlag::ServiceRunConfirmed) << "=" << (isFlagSet(WorldFlag::ServiceRunConfirmed) ? "true" : "false")
           << " " << WorldFlagName(WorldFlag::FerryOfficeJobComplete) << "=" << (isFlagSet(WorldFlag::FerryOfficeJobComplete) ? "true" : "false")
           << " eventCount=" << eventCount()
           << " lastEvent=\"" << lastEventText() << "\"";
    return output.str();
}

std::size_t WorldState::flagIndex(WorldFlag flag)
{
    return static_cast<std::size_t>(flag);
}

WorldEvent WorldState::makeEvent(WorldFlag flag, bool value, std::string source)
{
    WorldEvent event;
    event.id = m_nextEventId++;
    event.flag = flag;
    event.value = value;
    event.source = std::move(source);

    std::ostringstream name;
    name << WorldFlagName(flag) << "=" << (value ? "true" : "false");
    event.name = name.str();
    return event;
}
