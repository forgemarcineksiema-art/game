#pragma once

#include "engine/math/Math.h"

#include <string_view>

namespace FerryOffice {

namespace Names {
inline constexpr std::string_view FerryManifest = "Ferry Manifest";
inline constexpr std::string_view FerryOfficeNotice = "Ferry Office Notice";
inline constexpr std::string_view MaintenanceBox = "Maintenance Box";
inline constexpr std::string_view ServiceGateCollider = "service-gate";
inline constexpr std::string_view ServiceVault = "Service Barrier Vault";
inline constexpr std::string_view WallButton = "Wall Button";
inline constexpr std::string_view ExitMarker = "Exit Summary Marker";
inline constexpr std::string_view ServiceRunMarker = "Service Run Marker";
inline constexpr std::string_view DockRoadRelay = "Dock Road Relay";
inline constexpr std::string_view RelayServiceLog = "Relay Service Log";
inline constexpr std::string_view DockRoadClearanceTag = "Dock Road Clearance Tag";
inline constexpr std::string_view HarborPartsCrate = "Harbor Parts Crate";
inline constexpr std::string_view FerryOfficePartsShelf = "Ferry Office Parts Shelf";
inline constexpr std::string_view FerryOfficeWorkBoard = "Ferry Office Work Board";
inline constexpr std::string_view FerryOfficeHandoffNote = "Ferry Office Handoff Note";
} // namespace Names

namespace Prompts {
inline constexpr std::string_view FerryManifest = "Collect Ferry Manifest";
inline constexpr std::string_view FerryOfficeNotice = "Read Ferry Office Notice";
inline constexpr std::string_view MaintenanceBox = "Inspect Maintenance Box";
inline constexpr std::string_view ServiceVault = "Vault Service Barrier";
inline constexpr std::string_view WallButton = "Open Service Gate";
inline constexpr std::string_view ExitMarker = "Review Ferry Office Summary";
inline constexpr std::string_view ServiceRunMarker = "Review Service Run Marker";
inline constexpr std::string_view DockRoadRelay = "Reset Dock Road Relay";
inline constexpr std::string_view RelayServiceLog = "Log Relay Reset";
inline constexpr std::string_view DockRoadClearanceTag = "Tag Dock Road Clear";
inline constexpr std::string_view HarborPartsCrate = "Collect Harbor Parts";
inline constexpr std::string_view FerryOfficePartsShelf = "Deliver Harbor Parts";
inline constexpr std::string_view FerryOfficeWorkBoard = "Update Work Board";
inline constexpr std::string_view FerryOfficeHandoffNote = "File Handoff Note";
} // namespace Prompts

namespace Messages {
inline constexpr std::string_view FerryManifest = "Collected Ferry Manifest.";
inline constexpr std::string_view FerryOfficeNotice = "Notice: restore local power before opening the service route.";
inline constexpr std::string_view MaintenanceBox = "Maintenance box inspected: local power restored.";
inline constexpr std::string_view ServiceVault = "Service Barrier Vault";
inline constexpr std::string_view WallButton = "Service gate opened.";
inline constexpr std::string_view ExitMarker = "Ferry Office summary marker reached.";
inline constexpr std::string_view ServiceRunMarker =
    "Service run marker checked. Complete the service route and vehicle checkpoint before final confirmation.";
inline constexpr std::string_view DockRoadRelay = "Dock road relay reset: service-yard lights are back on.";
inline constexpr std::string_view RelayServiceLog = "Relay reset logged for the Ferry Office service board.";
inline constexpr std::string_view DockRoadClearanceTag = "Dock road tagged clear for the next ferry run.";
inline constexpr std::string_view HarborPartsCrate = "Harbor parts crate picked up for the Ferry Office shelf.";
inline constexpr std::string_view FerryOfficePartsShelf = "Harbor parts delivered to the Ferry Office shelf.";
inline constexpr std::string_view FerryOfficeWorkBoard = "Ferry Office work board updated with the Harbor Parts return.";
inline constexpr std::string_view FerryOfficeHandoffNote = "Ferry Office handoff note filed for the next ferry crew.";
} // namespace Messages

namespace Positions {
inline constexpr engine::Vec3 DockStart {0.0f, 0.08f, -1.0f};
inline constexpr engine::Vec3 OfficeMarker {0.0f, 1.05f, 3.75f};
inline constexpr engine::Vec3 FerryManifest {0.0f, 0.45f, 1.35f};
inline constexpr engine::Vec3 FerryOfficeNotice {-1.8f, 0.55f, 3.2f};
inline constexpr engine::Vec3 MaintenanceBox {2.8f, 0.65f, 1.9f};
inline constexpr engine::Vec3 WallButton {-0.9f, 0.7f, 2.15f};
inline constexpr engine::Vec3 ExitMarker {0.0f, 0.55f, 4.55f};
inline constexpr engine::Vec3 ServiceRunMarker {18.6f, 0.55f, -0.55f};
inline constexpr engine::Vec3 DockRoadRelay {18.05f, 0.65f, 0.25f};
inline constexpr engine::Vec3 RelayServiceLog {17.55f, 0.55f, 0.46f};
inline constexpr engine::Vec3 DockRoadClearanceTag {17.1f, 0.55f, 0.08f};
inline constexpr engine::Vec3 HarborPartsCrate {16.7f, 0.55f, -0.62f};
inline constexpr engine::Vec3 FerryOfficePartsShelf {-1.72f, 0.62f, 3.24f};
inline constexpr engine::Vec3 FerryOfficeWorkBoard {-2.08f, 0.68f, 2.82f};
inline constexpr engine::Vec3 FerryOfficeHandoffNote {-2.35f, 0.68f, 3.42f};
inline constexpr engine::Vec3 ServiceRunCheckpoint {19.35f, 0.08f, -2.2f};
inline constexpr engine::Vec3 ServiceVaultStart {2.8f, 0.0f, -0.35f};
inline constexpr engine::Vec3 ServiceVaultEnd {2.8f, 0.0f, 1.45f};
inline constexpr engine::Vec3 ServiceVaultFacing {0.0f, 0.0f, 1.0f};
inline constexpr engine::Vec3 ServiceGateCenter {0.0f, 0.75f, 2.35f};
inline constexpr engine::Vec3 ServiceGateHalfExtents {2.45f, 0.75f, 0.16f};
inline constexpr engine::Vec3 ServiceBarrierCenter {2.8f, 0.25f, 0.55f};
inline constexpr engine::Vec3 ServiceBarrierHalfExtents {1.0f, 0.25f, 0.45f};
} // namespace Positions

namespace Radii {
inline constexpr float FerryManifest = 1.8f;
inline constexpr float FerryOfficeNotice = 1.7f;
inline constexpr float MaintenanceBox = 1.4f;
inline constexpr float WallButton = 1.7f;
inline constexpr float ExitMarker = 1.8f;
inline constexpr float ServiceRunMarker = 1.8f;
inline constexpr float DockRoadRelay = 1.4f;
inline constexpr float RelayServiceLog = 1.4f;
inline constexpr float DockRoadClearanceTag = 1.4f;
inline constexpr float HarborPartsCrate = 1.4f;
inline constexpr float FerryOfficePartsShelf = 1.5f;
inline constexpr float FerryOfficeWorkBoard = 1.4f;
inline constexpr float FerryOfficeHandoffNote = 1.3f;
inline constexpr float ServiceRunCheckpoint = 1.5f;
} // namespace Radii

namespace Traversal {
inline constexpr float ServiceVaultFocusRadius = 1.25f;
inline constexpr float ServiceVaultRequiredFacingDot = 0.15f;
inline constexpr float ServiceVaultDurationSeconds = 0.55f;
} // namespace Traversal

} // namespace FerryOffice
