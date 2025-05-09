file(GLOB PROJECT_SOURCE include/*/*.h *.cpp include/zeno/packed3grids.cpp)

if (UNIX)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} /usr/local/lib/cmake/OpenVDB)
endif()


add_library(zenvdb SHARED ${PROJECT_SOURCE})

if (MSVC)
    # 节数超过对象文件格式限制: 请使用 /bigobj 进行编译 
    target_compile_options(zenvdb
        PRIVATE
        $<IF:$<COMPILE_LANGUAGE:CXX>, /bigobj, >
    )
endif()

target_include_directories(zenvdb PRIVATE ../oldzenbase/include)

target_include_directories(zenvdb PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/Zeno/zeno>)

if (ZENO_INSTALL_TARGET AND ZENO_BUILD_SHARED)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION include/Zeno/zeno)
endif()

find_package(OpenMP)
if (TARGET OpenMP::OpenMP_CXX)
    message("found package: OpenMP::OpenMP_CXX")
    target_link_libraries(zenvdb PRIVATE OpenMP::OpenMP_CXX)
endif()

#find_package(Blosc)
#if (TARGET Blosc::blosc)
    #message(STATUS "Found Blosc::blosc")
    #target_link_libraries(zenvdb PRIVATE Blosc::blosc)
#endif()
find_package(TBB REQUIRED COMPONENTS tbb tbbmalloc)
message(STATUS "Found TBB ${TBB_VERSION}")
target_link_libraries(zenvdb PRIVATE zenocore TBB::tbbmalloc TBB::tbb)

option(ZENO_SYSTEM_OPENVDB "Use OpenVDB found in system instead of submodule" OFF)
if (ZENO_SYSTEM_OPENVDB)
    find_package(OpenVDB REQUIRED)
    message(STATUS "Found OpenVDB ${OpenVDB_VERSION}")
    target_link_libraries(zenvdb PRIVATE OpenVDB::openvdb)
else()
    if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/openvdb/CMakeLists.txt)
        message(FATAL_ERROR "openvdb submodule not found! Please run: git submodule update --init --recursive")
    endif()
    set(OPENVDB_BUILD_BINARIES OFF CACHE BOOL "GUN TMD VDB_PRINT")
    set(OPENVDB_CORE_SHARED OFF CACHE BOOL "GUN TMD SHARED")
    set(OPENVDB_CORE_STATIC ON CACHE BOOL "MAKE STATIC GREAT AGAIN")
    set(DISABLE_DEPENDENCY_VERSION_CHECKS ON CACHE BOOL "GUN TMD VERSION")
    #set(USE_EXR ${ZENO_ENABLE_OPENEXR} CACHE BOOL "FUCK FOLLOW MY EXR" FORCE)
    #set(USE_IMATH_HALF ${ZENO_ENABLE_OPENEXR} CACHE BOOL "FUCK FOLLOW MY ILM" FORCE)
    set(USE_EXPLICIT_INSTANTIATION OFF CACHE BOOL "GUN TMD MFC" FORCE)
    add_subdirectory(openvdb)
    target_link_libraries(zenvdb INTERFACE $<BUILD_INTERFACE:openvdb> PRIVATE openvdb)
endif()
if (WIN32)
  target_compile_options(zenvdb PRIVATE -DNOMINMAX -D_USE_MATH_DEFINES)
endif()
