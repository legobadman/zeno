add_executable(zenobatch
    main.cpp
)

target_link_libraries(zenobatch PRIVATE zs_base zenovis)

if (ZENO_WITH_PYTHON)
    find_package(Python3 COMPONENTS Development REQUIRED)
    target_link_libraries(zenobatch PRIVATE ${Python3_LIBRARIES} zenpybind)
    target_include_directories(zenobatch PRIVATE ${Python3_INCLUDE_DIRS})
    target_compile_definitions(zenobatch PRIVATE -DZENO_WITH_PYTHON)
endif()