#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace zeno {

/** Build transform matrix from translate, rotate (euler deg, YXZ order), scale.
 *  Order: M = Translate * Rotate * Scale (applied to point as M * vec4(pos,1)) */
inline glm::mat4 buildTransformMatrix(
    const glm::vec3& translate,
    const glm::vec3& rotateDeg,
    const glm::vec3& scale)
{
    glm::mat4 matTrans = glm::translate(glm::mat4(1.0f), translate);
    glm::mat4 matRotate = glm::eulerAngleYXZ(
        rotateDeg.y * glm::pi<float>() / 180.f,
        rotateDeg.x * glm::pi<float>() / 180.f,
        rotateDeg.z * glm::pi<float>() / 180.f);
    glm::mat4 matScale = glm::scale(glm::mat4(1.0f), scale);
    return matTrans * matRotate * matScale;
}

/** Apply transform matrix to positions and optionally normals.
 *  Normals use inverse-transpose of upper 3x3 and renormalization. */
inline void applyTransform(
    const glm::mat4& M,
    const glm::vec3* positions, size_t nPos,
    glm::vec3* outPositions,
    const glm::vec3* normals, glm::vec3* outNormals)
{
    glm::mat3 matNormal = glm::transpose(glm::inverse(glm::mat3(M)));
    for (size_t i = 0; i < nPos; i++) {
        glm::vec4 hp = M * glm::vec4(positions[i], 1.f);
        outPositions[i] = glm::vec3(hp);
        if (normals && outNormals) {
            glm::vec3 n = matNormal * normals[i];
            float len = glm::length(n);
            if (len > 1e-6f) n /= len;
            outNormals[i] = n;
        }
    }
}

}
