#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <cstdint>
#include <cstddef>
#include <array>

namespace JonsEngine
{
    /*
     * Definitions of general data types commonly used across subsystems
     */

    enum TextureType
    {
        TEXTURE_TYPE_DIFFUSE = 0,
        TEXTURE_TYPE_NORMAL,
        TEXTURE_TYPE_SKYBOX
    };
    
    // TODO: move mesh/texture id definitions
    typedef uint32_t DX11MeshID;
    typedef uint32_t DX11MaterialID;

    const DX11MeshID INVALID_MESH_ID = 0;
    const DX11MaterialID INVALID_MATERIAL_ID = 0;

    typedef glm::vec2 Vec2;
    typedef glm::vec3 Vec3;
    typedef glm::vec4 Vec4;
    typedef glm::mat3 Mat3;
    typedef glm::mat4 Mat4;
    typedef glm::fquat Quaternion;
}