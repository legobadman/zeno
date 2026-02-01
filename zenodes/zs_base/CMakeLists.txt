file(GLOB_RECURSE source CONFIGURE_DEPENDS *.h *.cpp)

add_library(zs_base SHARED ${source})

if(MSVC)
    target_compile_options(zs_base PRIVATE /MP)
endif()

target_include_directories(zs_base PRIVATE ${PROJECT_SOURCE_DIR}/interface ${CMAKE_CURRENT_SOURCE_DIR}/include)

# if (ZENO_WITH_PYTHON)
#     find_package(Python3 COMPONENTS Development REQUIRED)

#     message(STATUS "python3 libraries for zenqt: ${Python3_LIBRARIES}")
#     message(STATUS "python3 include directories for zenqt: ${Python3_INCLUDE_DIRS}")
#     target_link_libraries(zs_base PRIVATE ${Python3_LIBRARIES} zenpybind)
#     target_include_directories(zs_base PRIVATE ${Python3_INCLUDE_DIRS})

#     target_compile_definitions(zs_base PRIVATE -DZENO_WITH_PYTHON)
# endif()


