add_definitions(-DBT_THREAD_SAFE)
add_compile_options(-w)

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bullet3/CMakeLists.txt)
    message(FATAL_ERROR "bullet3 submodule not found! Please run: git submodule update --init --recursive")
endif()
add_subdirectory(bullet3)
add_subdirectory(bullet3/HACD)

file (GLOB ZEN_RIGID_SOURCE *.cpp *.h)

list (REMOVE_ITEM ZEN_RIGID_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/RigidFracture.cpp)
if (ZENO_WITH_CUDA)
    list (APPEND ZEN_RIGID_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/RigidFracture.cpp)
endif(ZENO_WITH_CUDA)

add_library(zs_rigid SHARED ${ZEN_RIGID_SOURCE})
set_target_properties(zs_rigid PROPERTIES OUTPUT_NAME "zs_rigid")

target_link_libraries(zs_rigid PRIVATE zenocore)

zeno_disable_warning(${ZEN_RIGID_SOURCE})
target_include_directories(zs_rigid PRIVATE .)
target_include_directories(zs_rigid PRIVATE bullet3/src)

target_link_libraries(zs_rigid PRIVATE LinearMath)
target_link_libraries(zs_rigid PRIVATE Bullet3Common)
target_link_libraries(zs_rigid PRIVATE BulletInverseDynamics)
target_link_libraries(zs_rigid PRIVATE BulletCollision)
target_link_libraries(zs_rigid PRIVATE BulletDynamics)
target_link_libraries(zs_rigid PRIVATE BulletSoftBody)
#target_link_libraries(zs_rigid PRIVATE HACD) 应该是注释了代码导致导入库没编出来，先放着
target_link_libraries(zs_rigid PRIVATE VHACD)
target_link_libraries(zs_rigid PRIVATE BussIK)
target_link_libraries(zs_rigid PRIVATE URDFImporter)
