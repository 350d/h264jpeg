cmake_minimum_required(VERSION 3.10)
project(h264_to_jpeg VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_TESTS "Enable tests" ON)

# Include directories
include_directories(include)
include_directories(src)

# Source files
set(SOURCES
    src/h264_hw_decoder.c
    src/mjpeg_hw_encoder.c
    src/h264_to_jpeg.c
)

# Add Raspberry Pi definitions
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    add_definitions(-DRASPBERRY_PI)
endif()

# Header files
set(HEADERS
    include/h264_to_jpeg.h
    include/h264_hw_decoder.h
    include/mjpeg_hw_encoder.h
)

# Create library
add_library(h264_to_jpeg ${SOURCES} ${HEADERS})

# Link libraries - hardware only, no external dependencies

# Add MMAL libraries on Raspberry Pi (only when libraries are available)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    find_library(MMAL_LIB mmal)
    find_library(MMAL_CORE_LIB mmal_core)
    find_library(MMAL_UTIL_LIB mmal_util)
    find_library(MMAL_VC_CLIENT_LIB mmal_vc_client)
    find_library(VCOS_LIB vcos)
    find_library(BCM_HOST_LIB bcm_host)
    
    if(MMAL_LIB AND MMAL_CORE_LIB AND MMAL_UTIL_LIB AND MMAL_VC_CLIENT_LIB AND VCOS_LIB AND BCM_HOST_LIB)
        message(STATUS "MMAL libraries found, linking with hardware support")
        target_link_libraries(h264_to_jpeg 
            ${MMAL_LIB} 
            ${MMAL_CORE_LIB} 
            ${MMAL_UTIL_LIB} 
            ${MMAL_VC_CLIENT_LIB} 
            ${VCOS_LIB} 
            ${BCM_HOST_LIB}
        )
        target_include_directories(h264_to_jpeg PRIVATE 
            /opt/vc/include
            /opt/vc/include/interface/vcos/pthreads
            /opt/vc/include/interface/vmcs_host/linux
        )
    else()
        message(STATUS "MMAL libraries not found, building without hardware support")
        target_compile_definitions(h264_to_jpeg PRIVATE NO_HARDWARE)
    endif()
endif()

# Create example executable
add_executable(example examples/simple_debug.c)
target_link_libraries(example h264_to_jpeg)

# Create V4L2 H.264 test executable
add_executable(v4l2_h264_test examples/v4l2_h264_test.c)
target_link_libraries(v4l2_h264_test h264_to_jpeg)

# Create hardware test executable
add_executable(hw_test examples/hw_test.c)
target_link_libraries(hw_test h264_to_jpeg)

# Install
install(TARGETS h264_to_jpeg
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${HEADERS} DESTINATION include/h264_to_jpeg)

# Tests
if(ENABLE_TESTS)
    enable_testing()
    add_executable(test_h264_to_jpeg tests/simple_test.c)
    target_link_libraries(test_h264_to_jpeg h264_to_jpeg)
    add_test(NAME h264_to_jpeg_tests COMMAND test_h264_to_jpeg)
endif()
