cmake_minimum_required(VERSION 3.19)

if(NOT ZENO_WITH_CUDA)
  message(FATAL_ERROR "CUDA must be ON when CuOcean is ON! "
    "Please specify: -DZENO_WITH_CUDA:BOOL=ON -DZENO_WITH_zenvdb:BOOL=ON -DZENO_WITH_ZenoFX:BOOL=ON")
endif()

# pbd
add_library(z_cueulerian SHARED 
  oceanfft/Ocean.cu
)

target_link_libraries(z_cueulerian PRIVATE zshelper CUDA::cufft)

#[[ fluid
target_sources(z_cueulerian PRIVATE
  volume/VolumeTopology.cu
  volume/VolumeOperator.cu
  volume/Transfer.cu
  volume/Conversion.cu
)
]]
target_sources(z_cueulerian PRIVATE
  volume/VolumeOps.cu
)

# shallow water & terrain
target_sources(z_cueulerian PRIVATE
  swe/FDGather.cu
  swe/Erode.cu
  swe/SWE_dense.cu
)

# level set function
target_sources(z_cueulerian PRIVATE
  levelset/Grid_creator.cu
  levelset/Grid_creator_adaptive.cu
  levelset/Reinitialization.cu
  levelset/Extrapolation.cu
)

# navier stokes
target_sources(z_cueulerian PRIVATE
  navierstokes/NS_pipeline.cu
  navierstokes/NS_boundary.cu
  navierstokes/NS_linearsolver.cu
  navierstokes/NS_topo.cu
  navierstokes/NS_tracer.cu
  navierstokes/NS_noise.cu
)

# hybrid
target_sources(z_cueulerian PRIVATE
  hybrid/P2G.cu
  hybrid/G2P.cu
)