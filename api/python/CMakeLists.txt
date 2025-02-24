if (ZENO_WITH_PYTHON)
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

    add_subdirectory(pybind11)
    pybind11_add_module(zen 
        zpygraph.cpp
        zpygraph.h
        zpynode.cpp
        zpynode.h
        zpysession.cpp)

    target_include_directories(zen PUBLIC ${CMAKE_SOURCE_DIR}/zenocore/include)
    target_link_libraries(zen PUBLIC zenocore)
    set_target_properties(zen PROPERTIES SUFFIX ".pyd")

    add_library(zenpybind STATIC
        zpygraph.cpp
        zpygraph.h
        zpynode.cpp
        zpynode.h
        zpysession.cpp)
    #target_link_libraries(zenpybind PRIVATE ${Python3_LIBRARIES})
    target_include_directories(zenpybind PRIVATE ${Python3_INCLUDE_DIRS})
    target_include_directories(zenpybind PUBLIC ${CMAKE_SOURCE_DIR}/zenocore/include pybind11/include)
    target_compile_definitions(zenpybind PUBLIC -DZENO_PYAPI_STATICLIB)
    target_link_libraries(zenpybind PRIVATE zenocore)

endif()