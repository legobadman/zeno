if (ZENO_WITH_PYTHON)
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

    add_subdirectory(pybind11)
    pybind11_add_module(ze 
        zpygraph.cpp
        zpygraph.h
        zpynode.cpp
        zpynode.h
        zpysession.cpp)

    target_include_directories(ze PUBLIC ${CMAKE_SOURCE_DIR}/zenocore/include)
    target_link_libraries(ze PUBLIC zenocore)
    set_target_properties(ze PROPERTIES SUFFIX ".pyd")
endif()