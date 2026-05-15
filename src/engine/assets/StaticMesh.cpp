#include "engine/assets/StaticMesh.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <utility>

namespace engine {
namespace {

struct BufferViewInfo {
    std::size_t byteOffset = 0;
    std::size_t byteLength = 0;
};

struct AccessorInfo {
    int bufferView = -1;
    std::size_t byteOffset = 0;
    int componentType = 0;
    std::size_t count = 0;
    std::string type;
};

StaticMeshLoadResult Fail(std::string error)
{
    StaticMeshLoadResult result;
    result.error = std::move(error);
    return result;
}

std::optional<std::string> ReadTextFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return std::nullopt;
    }
    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

std::optional<std::vector<std::uint8_t>> DecodeBase64(std::string_view input)
{
    const auto decode = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') {
            return c - 'A';
        }
        if (c >= 'a' && c <= 'z') {
            return c - 'a' + 26;
        }
        if (c >= '0' && c <= '9') {
            return c - '0' + 52;
        }
        if (c == '+') {
            return 62;
        }
        if (c == '/') {
            return 63;
        }
        return -1;
    };

    std::vector<std::uint8_t> output;
    int value = 0;
    int bits = -8;
    for (char c : input) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            continue;
        }
        if (c == '=') {
            break;
        }
        const int decoded = decode(c);
        if (decoded < 0) {
            return std::nullopt;
        }
        value = (value << 6) + decoded;
        bits += 6;
        if (bits >= 0) {
            output.push_back(static_cast<std::uint8_t>((value >> bits) & 0xFF));
            bits -= 8;
        }
    }
    return output;
}

template <typename T>
std::optional<T> ReadValue(const std::vector<std::uint8_t>& bytes, std::size_t offset)
{
    if (offset + sizeof(T) > bytes.size()) {
        return std::nullopt;
    }
    T value {};
    std::memcpy(&value, bytes.data() + offset, sizeof(T));
    return value;
}

Vec3 TransformPosition(Vec3 local, const StaticMeshInstance& instance)
{
    const Vec3 scaled {
        local.x * instance.scale.x,
        local.y * instance.scale.y,
        local.z * instance.scale.z,
    };
    const float c = std::cos(instance.yawRadians);
    const float s = std::sin(instance.yawRadians);
    const Vec3 rotated {
        scaled.x * c + scaled.z * s,
        scaled.y,
        -scaled.x * s + scaled.z * c,
    };
    return instance.position + rotated;
}

} // namespace

bool StaticMeshAsset::isValid() const
{
    return !vertices.empty() && !indices.empty();
}

bool StaticMeshLoadResult::ok() const
{
    return error.empty() && mesh.isValid();
}

Bounds3 ComputeBounds(const std::vector<StaticMeshVertex>& vertices)
{
    if (vertices.empty()) {
        return {};
    }

    Bounds3 bounds {vertices.front().position, vertices.front().position};
    for (const StaticMeshVertex& vertex : vertices) {
        bounds.min.x = std::min(bounds.min.x, vertex.position.x);
        bounds.min.y = std::min(bounds.min.y, vertex.position.y);
        bounds.min.z = std::min(bounds.min.z, vertex.position.z);
        bounds.max.x = std::max(bounds.max.x, vertex.position.x);
        bounds.max.y = std::max(bounds.max.y, vertex.position.y);
        bounds.max.z = std::max(bounds.max.z, vertex.position.z);
    }
    return bounds;
}

StaticMeshLoadResult LoadStaticMeshFromGltf(const std::filesystem::path& path)
{
    const auto gltfText = ReadTextFile(path);
    if (!gltfText) {
        return Fail("glTF mesh file not found: " + path.string());
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(*gltfText);
    } catch (const std::exception&) {
        return Fail("glTF mesh file is not valid JSON: " + path.string());
    }

    try {
    if (!root.contains("buffers") || !root["buffers"].is_array() || root["buffers"].empty()) {
        return Fail("glTF mesh must contain at least one buffer.");
    }

    const auto& buffer0 = root["buffers"][0];
    if (!buffer0.contains("uri") || !buffer0["uri"].is_string()) {
        return Fail("glTF mesh must contain an embedded buffer URI.");
    }

    const std::string uri = buffer0["uri"].get<std::string>();
    const std::string marker = "data:application/octet-stream;base64,";
    const std::size_t markerPos = uri.find(marker);
    if (markerPos == std::string::npos) {
        return Fail("glTF mesh must contain one embedded application/octet-stream base64 buffer.");
    }

    const auto buffer = DecodeBase64(std::string_view(uri).substr(markerPos + marker.size()));
    if (!buffer) {
        return Fail("glTF mesh embedded buffer could not be base64-decoded.");
    }

    std::vector<BufferViewInfo> bufferViews;
    if (root.contains("bufferViews") && root["bufferViews"].is_array()) {
        for (const auto& bv : root["bufferViews"]) {
            BufferViewInfo view;
            view.byteOffset = bv.value("byteOffset", 0U);
            view.byteLength = bv.value("byteLength", 0U);
            bufferViews.push_back(view);
        }
    }

    std::vector<AccessorInfo> accessors;
    if (root.contains("accessors") && root["accessors"].is_array()) {
        for (const auto& a : root["accessors"]) {
            AccessorInfo accessor;
            accessor.bufferView = a.value("bufferView", -1);
            accessor.byteOffset = a.value("byteOffset", 0U);
            accessor.componentType = a.value("componentType", 0);
            accessor.count = a.value("count", 0U);
            accessor.type = a.value("type", "");
            accessors.push_back(accessor);
        }
    }

    if (!root.contains("meshes") || !root["meshes"].is_array() || root["meshes"].empty()) {
        return Fail("glTF mesh must contain at least one mesh.");
    }

    const auto& mesh0 = root["meshes"][0];
    if (!mesh0.contains("primitives") || !mesh0["primitives"].is_array() || mesh0["primitives"].empty()) {
        return Fail("glTF mesh must contain at least one primitive.");
    }

    const auto& primitive = mesh0["primitives"][0];
    if (!primitive.contains("attributes") || !primitive["attributes"].is_object()) {
        return Fail("glTF mesh primitive must have attributes.");
    }

    const auto& attributes = primitive["attributes"];
    if (!attributes.contains("POSITION")) {
        return Fail("glTF mesh primitive must have a POSITION attribute.");
    }
    if (!attributes["POSITION"].is_number_integer()) {
        return Fail("glTF POSITION attribute must reference an integer accessor index.");
    }
    if (!primitive.contains("indices") || !primitive["indices"].is_number_integer()) {
        return Fail("glTF mesh must contain an integer indices accessor.");
    }

    const int positionAccessorIndex = attributes["POSITION"].get<int>();
    const int indexAccessorIndex = primitive["indices"].get<int>();

    if (indexAccessorIndex < 0) {
        return Fail("glTF mesh must contain indices accessor.");
    }
    if (positionAccessorIndex < 0 || static_cast<std::size_t>(positionAccessorIndex) >= accessors.size()
        || indexAccessorIndex < 0 || static_cast<std::size_t>(indexAccessorIndex) >= accessors.size()) {
        return Fail("glTF mesh references an accessor outside the accessors array.");
    }

    const AccessorInfo& positionAccessor = accessors[static_cast<std::size_t>(positionAccessorIndex)];
    const AccessorInfo& indexAccessor = accessors[static_cast<std::size_t>(indexAccessorIndex)];
    if (positionAccessor.componentType != 5126 || positionAccessor.type != "VEC3") {
        return Fail("glTF POSITION accessor must be FLOAT VEC3.");
    }
    if (indexAccessor.type != "SCALAR" || (indexAccessor.componentType != 5123 && indexAccessor.componentType != 5125)) {
        return Fail("glTF indices accessor must be UNSIGNED_SHORT or UNSIGNED_INT SCALAR.");
    }
    if (positionAccessor.bufferView < 0 || static_cast<std::size_t>(positionAccessor.bufferView) >= bufferViews.size()
        || indexAccessor.bufferView < 0 || static_cast<std::size_t>(indexAccessor.bufferView) >= bufferViews.size()) {
        return Fail("glTF mesh accessor references an invalid bufferView.");
    }

    const BufferViewInfo& positionView = bufferViews[static_cast<std::size_t>(positionAccessor.bufferView)];
    const BufferViewInfo& indexView = bufferViews[static_cast<std::size_t>(indexAccessor.bufferView)];

    StaticMeshAsset mesh;
    mesh.sourcePath = path;
    mesh.id = path.stem().string();
    mesh.vertices.reserve(positionAccessor.count);
    const std::size_t positionBase = positionView.byteOffset + positionAccessor.byteOffset;
    for (std::size_t index = 0; index < positionAccessor.count; ++index) {
        const std::size_t offset = positionBase + index * sizeof(float) * 3;
        const auto x = ReadValue<float>(*buffer, offset + sizeof(float) * 0);
        const auto y = ReadValue<float>(*buffer, offset + sizeof(float) * 1);
        const auto z = ReadValue<float>(*buffer, offset + sizeof(float) * 2);
        if (!x || !y || !z) {
            return Fail("glTF POSITION buffer data is shorter than the accessor declares.");
        }
        mesh.vertices.push_back({{*x, *y, *z}});
    }

    mesh.indices.reserve(indexAccessor.count);
    const std::size_t indexBase = indexView.byteOffset + indexAccessor.byteOffset;
    const std::size_t indexStride = indexAccessor.componentType == 5123 ? sizeof(std::uint16_t) : sizeof(std::uint32_t);
    for (std::size_t index = 0; index < indexAccessor.count; ++index) {
        const std::size_t offset = indexBase + index * indexStride;
        if (indexAccessor.componentType == 5123) {
            const auto value = ReadValue<std::uint16_t>(*buffer, offset);
            if (!value) {
                return Fail("glTF index buffer data is shorter than the accessor declares.");
            }
            mesh.indices.push_back(static_cast<unsigned int>(*value));
        } else {
            const auto value = ReadValue<std::uint32_t>(*buffer, offset);
            if (!value) {
                return Fail("glTF index buffer data is shorter than the accessor declares.");
            }
            mesh.indices.push_back(static_cast<unsigned int>(*value));
        }
    }

    if (mesh.indices.size() % 3 != 0) {
        return Fail("glTF index count must be divisible by three for the v0.12 triangle-list subset.");
    }
    if (std::any_of(mesh.indices.begin(), mesh.indices.end(), [&](unsigned int index) { return index >= mesh.vertices.size(); })) {
        return Fail("glTF index buffer references a vertex outside the POSITION accessor.");
    }

    mesh.bounds = ComputeBounds(mesh.vertices);
    return {mesh, {}};
    } catch (const std::exception&) {
        return Fail("glTF mesh contains unsupported JSON field types: " + path.string());
    }
}

std::vector<Vec3> BuildFlatTriangleList(const StaticMeshAsset& mesh, const StaticMeshInstance& instance)
{
    std::vector<Vec3> triangles;
    if (!instance.visible || mesh.vertices.empty() || mesh.indices.empty()) {
        return triangles;
    }

    triangles.reserve(mesh.indices.size());
    for (unsigned int index : mesh.indices) {
        if (index >= mesh.vertices.size()) {
            return {};
        }
        triangles.push_back(TransformPosition(mesh.vertices[index].position, instance));
    }
    return triangles;
}

} // namespace engine
