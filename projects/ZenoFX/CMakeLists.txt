add_subdirectory(ZFX)

add_library(zfxold SHARED 
    nw.cpp pw.cpp pnw.cpp ppw.cpp p2w.cpp pmw.cpp tw.cpp ne.cpp se.cpp FDGather.cpp refutils.cpp dbg_printf.h
)

target_include_directories(zfxold PRIVATE .)

target_link_libraries(zfxold PRIVATE $<BUILD_INTERFACE:ZFX>)
target_link_libraries(zfxold PRIVATE zenocore)
#target_sources(zeno PRIVATE
#    nw.cpp pw.cpp pnw.cpp ppw.cpp p2w.cpp pmw.cpp tw.cpp ne.cpp se.cpp FDGather.cpp refutils.cpp dbg_printf.h
#    )

#if (ZENO_WITH_zenvdb)
    #option(ZENOFX_ENABLE_OPENVDB "Enable OpenVDB wrangler in ZenoFX" ON)
    #option(ZENOFX_ENABLE_LBVH "Enable Linear BVH wrangler in ZenoFX" ON)
    set(ZENOFX_ENABLE_OPENVDB ${ZENO_WITH_zenvdb})
    set(ZENOFX_ENABLE_LBVH ${ZENO_WITH_zenvdb})
#else()
    #option(ZENOFX_ENABLE_OPENVDB "Enable OpenVDB wrangler in ZenoFX" OFF)
    #option(ZENOFX_ENABLE_LBVH "Enable Linear BVH wrangler in ZenoFX" OFF)
#if (ZENO_WITH_zenvdb)
#endif()

if (ZENOFX_ENABLE_OPENVDB)
    if (NOT ZENO_WITH_zenvdb)
        message(FATAL_ERROR "ZENO_WITH_zenvdb must be ON when ZENOFX_ENABLE_OPENVDB is ON! Please -DZENO_WITH_zenvdb:BOOL=ON")
    endif()
    if (ZENO_SYSTEM_OPENVDB)
        find_package(OpenVDB REQUIRED)
        target_link_libraries(zfxold PRIVATE OpenVDB::openvdb)
    endif()
    target_sources(zfxold PRIVATE vw.cpp)
    target_include_directories(zfxold PRIVATE ../zenvdb/include)
endif()

if (ZENOFX_ENABLE_LBVH)
    target_sources(zfxold PRIVATE pnbvhw.cpp LinearBvh.cpp LinearBvh.h SpatialUtils.hpp)
endif()

find_package(OpenMP)
if (TARGET OpenMP::OpenMP_CXX)
    message(STATUS "found package: OpenMP::OpenMP_CXX")
    target_link_libraries(zfxold PRIVATE OpenMP::OpenMP_CXX)
endif()
