# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} /usr/lib/cmake/OpenVDB /usr/local/lib/cmake/OpenVDB ${CMAKE_CURRENT_SOURCE_DIR}/cmake)  # cihou zhxx
# ^^^NONO^^^ Please add `export OpenVDB_DIR=/usr/local/lib/cmake/OpenVDB` in your .bashrc to YI-LAO-YONG-YI!!!!!!

if (ZENO_NO_WARNING)
    set(CMAKE_CXX_FLAGS "${ZENO_OLD_CXX_FLAGS} -w")
    add_compile_options(-w)
endif()

set(ZENO_EXTENSIONS
    ZenoFX
    oldzenbase
    zenvdb
    FastFLIP
    Rigid
    cgmesh
    mesher
    Euler
    FEM
    DemBones
    FBX
    Alembic
    GLTF
    LSystem
    Skinning
    SampleModel
    Functional
    TreeSketch
    CalcGeometryUV
    MeshSubdiv
    CUDA
    CuEulerian
    CuLagrange
    PyZpc
    Audio
    ImgCV
    GUI
    PBD
    USD
    Python
    Parameterization
    UnrealTool
    ChatZeno
    RPC
    Roads
    Geometry
    PluginPOC
    Nemo
    )

#test custom module
#add_subdirectory(customPlugin1)

foreach (name IN ITEMS ${ZENO_EXTENSIONS})
    option(ZENO_WITH_${name} "Build ZENO with extension: ${name}" OFF)
    if (ZENO_WITH_${name})
        message(STATUS "Building ZENO with extension: ${name}")
        add_subdirectory(${name})
    endif()
endforeach()

set(ZENO_EXTENSION_TOOLS
    FLIPtools
    cgmeshTools
    BulletTools
    HerculesTools
    )
foreach (name IN ITEMS ${ZENO_EXTENSION_TOOLS})
    option(ZENO_WITH_TOOL_${name} "Build ZENO with extension tool: ${name}" OFF)
    if (ZENO_WITH_TOOL_${name})
        message(STATUS "Building ZENO with extension tool: ${name}")
        add_subdirectory(tools/${name})
    endif()
endforeach()
