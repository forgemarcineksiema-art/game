#include "SandboxLayer.h"

#include "engine/core/Logger.h"
#include "engine/math/Math.h"

#include <iomanip>
#include <sstream>

void SandboxLayer::onAttach()
{
    engine::Logger::info("Sandbox layer attached.");
    m_player.setObstacles(m_scene.obstacles());
    updateDebugText();
}

void SandboxLayer::onUpdate(double deltaSeconds, const engine::InputState& input)
{
    ++m_frameIndex;
    const float dt = static_cast<float>(deltaSeconds);
    m_player.update(dt, input, m_camera.state().yawRadians);

    CameraTarget target;
    target.position = m_player.state().position;
    target.yawRadians = m_player.state().facingYawRadians;
    m_camera.update(dt, input, target);

    updateDebugText();
    if (m_frameIndex == 1 || m_frameIndex % 120 == 0) {
        engine::Logger::info(m_debugText);
    }
}

void SandboxLayer::onRender(engine::IRenderer& renderer)
{
    renderer.setDebugCamera(m_camera.debugCamera());
    renderer.drawDebugGridAndAxes();
    for (const PlayerObstacle& obstacle : m_scene.obstacles()) {
        renderer.drawDebugBox(obstacle.center, obstacle.halfExtents, {0.9f, 0.72f, 0.28f, 1.0f});
    }

    const PlayerState& player = m_player.state();
    renderer.drawDebugBox(player.position + engine::Vec3 {0.0f, m_player.settings().height * 0.5f, 0.0f},
        {m_player.settings().radius, m_player.settings().height * 0.5f, m_player.settings().radius},
        {0.25f, 0.55f, 1.0f, 1.0f});

    const engine::Vec3 facing = engine::ForwardFromYaw(player.facingYawRadians);
    renderer.drawDebugLine(player.position + engine::Vec3 {0.0f, 1.0f, 0.0f},
        player.position + engine::Vec3 {0.0f, 1.0f, 0.0f} + facing * 1.25f,
        {1.0f, 1.0f, 1.0f, 1.0f});
    renderer.drawDebugBox(m_camera.state().target, {0.08f, 0.08f, 0.08f}, {1.0f, 0.25f, 0.7f, 1.0f});
    renderer.drawDebugText(m_debugText);
}

void SandboxLayer::onDetach()
{
    engine::Logger::info("Sandbox layer detached.");
}

std::string SandboxLayer::debugText() const
{
    return m_debugText;
}

void SandboxLayer::updateDebugText()
{
    const PlayerState& player = m_player.state();
    const ThirdPersonCameraState& camera = m_camera.state();

    std::ostringstream output;
    output << std::fixed << std::setprecision(2)
           << "player=(" << player.position.x << "," << player.position.y << "," << player.position.z << ") "
           << "speed=" << player.horizontalSpeed << " "
           << (player.sprinting ? "sprint" : "walk") << " "
           << (player.grounded ? "grounded" : "air") << " "
           << "camera yaw=" << engine::Degrees(camera.yawRadians)
           << " pitch=" << engine::Degrees(camera.pitchRadians)
           << " dist=" << camera.distance;
    m_debugText = output.str();
}
