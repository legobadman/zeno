#pragma once

#include <vec.h>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <inodedata.h>
#include <inodeimpl.h>
#include "api.h"
#include "zcommon.h"
#include <Windows.h>
#include "vecutil.h"
#include "typehelper.h"
#include "utils.h"
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace zeno {

static std::vector<glm::vec3> to_glm_points(const std::vector<zeno::vec3f>& v) {
    std::vector<glm::vec3> r(v.size());
    for (size_t i = 0; i < v.size(); i++)
        r[i] = glm::vec3(v[i][0], v[i][1], v[i][2]);
    return r;
}

static glm::mat4 calc_rotate_matrix(
    float xangle,
    float yangle,
    float zangle,
    Rotate_Orientaion orientaion
) {
    float rad_x = xangle * (M_PI / 180.0);
    float rad_y = yangle * (M_PI / 180.0);
    float rad_z = zangle * (M_PI / 180.0);
#if 0
    switch (orientaion)
    {
    case Orientaion_XY:
        rad_x = (xangle + 90) * (M_PI / 180.0);
        break;
    case Orientaion_YZ:
        rad_z = (zangle - 90) * (M_PI / 180.0);
        break;
    case Orientaion_ZX:
        break;
    }
#endif
    glm::mat4 mx = glm::mat4(
        1, 0, 0, 0,
        0, cos(rad_x), sin(rad_x), 0,
        0, -sin(rad_x), cos(rad_x), 0,
        0, 0, 0, 1);
    glm::mat4 my = glm::mat4(
        cos(rad_y), 0, -sin(rad_y), 0,
        0, 1, 0, 0,
        sin(rad_y), 0, cos(rad_y), 0,
        0, 0, 0, 1);
    glm::mat4 mz = glm::mat4(
        cos(rad_z), sin(rad_z), 0, 0,
        -sin(rad_z), cos(rad_z), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1);
    return mz * my * mx;
}

} // namespace zeno

