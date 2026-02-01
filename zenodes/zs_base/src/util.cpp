#include "utils.h"
#include <zvec.h>
#include <vecutil.h>

namespace zeno {

    std::unique_ptr<Vec3f[]> convert_points_to_abi(
        const std::vector<vec3f>& points,
        size_t& outSize
    ) {
        std::unique_ptr<Vec3f[]> abiPoints;

        outSize = points.size();
        if (outSize) {
            abiPoints = std::make_unique<Vec3f[]>(outSize);
            for (size_t i = 0; i < outSize; ++i) {
                abiPoints[i] = toAbiVec3f(points[i]);
            }
        }
        return abiPoints;
    }
}