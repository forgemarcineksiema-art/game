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
} // namespace Names

namespace Prompts {
inline constexpr std::string_view FerryManifest = "Collect Ferry Manifest";
inline constexpr std::string_view FerryOfficeNotice = "Read Ferry Office Notice";
inline constexpr std::string_view MaintenanceBox = "Inspect Maintenance Box";
inline constexpr std::string_view ServiceVault = "Press Space: Vault Service Barrier";
inline constexpr std::string_view WallButton = "Open Service Gate";
inline constexpr std::string_view ExitMarker = "Review Ferry Office Summary";
inline constexpr std::string_view ServiceRunMarker = "Confirm Service Run";
} // namespace Prompts

namespace Messages {
inline constexpr std::string_view FerryManifest = "Collected Ferry Manifest.";
inline constexpr std::string_view FerryOfficeNotice = "Notice: restore local power before opening the service route.";
inline constexpr std::string_view MaintenanceBox = "Maintenance box inspected: local power restored.";
inline constexpr std::string_view ServiceVault = "Service Barrier Vault";
inline constexpr std::string_view WallButton = "Service gate opened.";
inline constexpr std::string_view ExitMarker = "Ferry Office summary marker reached.";
inline constexpr std::string_view ServiceRunMarker = "Service run confirmed at the dock-road marker.";
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
inline constexpr float ServiceRunCheckpoint = 1.5f;
} // namespace Radii

namespace Traversal {
inline constexpr float ServiceVaultFocusRadius = 1.25f;
inline constexpr float ServiceVaultRequiredFacingDot = 0.15f;
inline constexpr float ServiceVaultDurationSeconds = 0.55f;
} // namespace Traversal

} // namespace FerryOffice
