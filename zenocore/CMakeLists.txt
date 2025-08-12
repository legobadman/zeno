option(ZENO_BENCHMARKING "Enable ZENO benchmarking timer" ON)
option(ZENO_PARALLEL_STL "Enable parallel STL in ZENO" OFF)
option(ZENO_ENABLE_OPENMP "Enable OpenMP in ZENO for parallelism" ON)
option(ZENO_ENABLE_MAGICENUM "Enable magicenum in ZENO for enum reflection" OFF)
option(ZENO_ENABLE_BACKWARD "Enable ZENO fault handler for traceback" OFF)

file(GLOB_RECURSE source CONFIGURE_DEPENDS include/*.h src/*.cpp)

add_library(zenocore SHARED ${source})

if (ZENO_DEBUG_MSVC)
    zeno_dbg_msvc(zenocore)
endif()

if(MSVC)
    target_compile_options(zenocore PRIVATE /MP)
endif()

if (ZENO_ENABLE_OPENMP)
    find_package(OpenMP)
    if (TARGET OpenMP::OpenMP_CXX)
        message(STATUS "Found OpenMP::OpenMP_CXX")
        target_link_libraries(zenocore PRIVATE OpenMP::OpenMP_CXX)
        if (MSVC)
            include(CheckCXXCompilerFlag)
            check_cxx_compiler_flag("/openmp:experimental" MSVC_SUPPORT_OPENMP_EXPERIMENTAL)
            if (MSVC_SUPPORT_OPENMP_EXPERIMENTAL)
                target_compile_options(zenocore PRIVATE $<IF:$<COMPILE_LANGUAGE:CXX>,/openmp:experimental,>)
            endif()
            check_cxx_compiler_flag("/openmp:llvm" MSVC_SUPPORT_OPENMP_LLVM)
            if (MSVC_SUPPORT_OPENMP_LLVM)
                target_compile_options(zenocore PRIVATE $<IF:$<COMPILE_LANGUAGE:CXX>,/openmp:llvm,>)
            endif()
        endif()
    else()
        message(WARNING "Not found OpenMP")
    endif()
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(zenocore PRIVATE uuid)
endif()

if (ZENO_BENCHMARKING)
    target_compile_definitions(zenocore PUBLIC -DZENO_BENCHMARKING)
endif()

if (ZENO_PARALLEL_STL)
    find_package(Threads REQUIRED)
    target_link_libraries(zenocore PRIVATE Threads::Threads)
    if (NOT MSVC)
        find_package(TBB)
        if (TBB_FOUND)
            message(STATUS "Found TBB: ${TBB_DIR}")
            target_link_libraries(zenocore PRIVATE TBB::tbb)
        else()
            message(WARNING "TBB not found, disabling parallel STL")
            set(ZENO_PARALLEL_STL OFF)
        endif()
    endif()
    if (ZENO_PARALLEL_STL)
        target_compile_definitions(zenocore PUBLIC -DZENO_PARALLEL_STL)
    endif()
endif()

if (ZENO_ENABLE_BACKWARD)
    add_subdirectory(tpls/backward-cpp)
    target_compile_definitions(zenocore PUBLIC -DZENO_ENABLE_BACKWARD)
    target_sources(zenocore PRIVATE ${BACKWARD_ENABLE})
    add_backward(zeno)
endif()

if (ZENO_WITH_PYTHON)
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

    message(STATUS "python3 libraries for zenqt: ${Python3_LIBRARIES}")
    message(STATUS "python3 include directories for zenqt: ${Python3_INCLUDE_DIRS}")
    #target_link_libraries(zenocore PUBLIC ${Python3_LIBRARIES})
    #target_include_directories(zenocore PUBLIC ${Python3_INCLUDE_DIRS})
    target_compile_definitions(zenocore PRIVATE -DZENO_WITH_PYTHON)
    target_link_libraries(zenocore PRIVATE ${Python3_LIBRARIES})
    target_include_directories(zenocore PRIVATE ${Python3_INCLUDE_DIRS})
    #target_link_options(zenocore PRIVATE /NODEFAULTLIB:python311.lib)

    #add_subdirectory(pybind11)
    #pybind11_add_module(ze 
    #    src/core/Session.cpp
    #    src/core/ObjectManager.cpp
    #    src/core/Graph.cpp
    #    src/core/INode.cpp
    #    src/core/ReflectNodeClass.cpp)
    #target_link_libraries(ze PRIVATE zenocore)
    #set_target_properties(ze PROPERTIES SUFFIX ".pyd")
endif()

if (ZENO_BUILD_SHARED)
    target_compile_definitions(zenocore PRIVATE -DZENO_DLLEXPORT INTERFACE -DZENO_DLLIMPORT)
endif()

target_include_directories(zenocore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    #$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pybind11/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tpls/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tpls/deprecated>
    $<INSTALL_INTERFACE:include/Zeno/zeno/include>
    ${PROJECT_SOURCE_DIR}
)

if (ZENO_INSTALL_TARGET AND ZENO_BUILD_SHARED)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION include/Zeno/zeno)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tpls/include DESTINATION include/Zeno/zeno)
    install(TARGETS zenocore EXPORT ZenoTargets)
endif()

if (ZENO_ENABLE_MAGICENUM)
    target_compile_definitions(zenocore PUBLIC -DZENO_ENABLE_MAGICENUM)
endif()

if (MSVC)  # must use this to make msvc linker happy
    # 节数超过对象文件格式限制: 请使用 /bigobj 进行编译 
    target_compile_options(zenocore
        PRIVATE
        $<IF:$<COMPILE_LANGUAGE:CXX>, /bigobj, >
    )
endif()

if (ZENO_WITH_VLD)
    target_include_directories(zenocore PRIVATE "C:/vld/src")
    target_link_libraries(zenocore PRIVATE "C:/vld2/src/bin/x64/Debug-v143/vld.lib")
endif()
