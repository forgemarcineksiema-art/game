#pragma once

#include "engine/math/Math.h"
#include "engine/renderer/RendererTypes.h"

#include <filesystem>
#include <string>
#include <vector>

namespace engine {

struct StaticMeshVertex {
    Vec3 position;
};

struct Bounds3 {
    Vec3 min;
    Vec3 max;
};

struct StaticMeshAsset {
    std::string id;
    std::filesystem::path sourcePath;
    std::vector<StaticMeshVertex> vertices;
    std::vector<unsigned int> indices;
    Bounds3 bounds {};

    bool isValid() const;
};

struct StaticMeshInstance {
    std::string assetId;
    Vec3 position {0.0f, 0.0f, 0.0f};
    Vec3 scale {1.0f, 1.0f, 1.0f};
    float yawRadians = 0.0f;
    Color tint {1.0f, 1.0f, 1.0f, 1.0f};
    bool visible = true;
};

struct StaticMeshLoadResult {
    StaticMeshAsset mesh;
    std::string error;

    bool ok() const;
};

Bounds3 ComputeBounds(const std::vector<StaticMeshVertex>& vertices);
StaticMeshLoadResult LoadStaticMeshFromGltf(const std::filesystem::path& path);
std::vector<Vec3> BuildFlatTriangleList(const StaticMeshAsset& mesh, const StaticMeshInstance& instance);

} // namespace engine
