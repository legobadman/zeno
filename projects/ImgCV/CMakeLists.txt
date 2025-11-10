file(GLOB IMGCV_SOURCE *.cpp *.h)

add_library(zs_imgcv SHARED ${IMGCV_SOURCE})
target_link_libraries(zs_imgcv PRIVATE zenocore)

target_include_directories(zs_imgcv PRIVATE .)

#find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc highgui features2d calib3d stitching)
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc)

target_include_directories(zs_imgcv PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(zs_imgcv PRIVATE ${OpenCV_LIBS})

find_package(TIFF REQUIRED)
include_directories(${TIFF_INCLUDE_DIR})
target_link_libraries(zs_imgcv PRIVATE ${TIFF_LIBRARIES})
