#pragma once

#include "game/SceneDefinition.h"

#include <filesystem>
#include <string>

struct SceneLoadResult {
    SceneDefinition scene;
    std::string error;

    bool ok() const;
};

SceneLoadResult LoadSceneDefinition(const std::filesystem::path& path);
