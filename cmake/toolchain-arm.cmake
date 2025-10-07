# Cross-compilation toolchain file for ARM architectures
# This file is used by CMake to set up cross-compilation environment

# Set the system name
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Specify the cross compiler (use gnueabi for soft-float)
set(CMAKE_C_COMPILER arm-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabi-g++)

# Specify the tools
set(CMAKE_AR arm-linux-gnueabi-ar)
set(CMAKE_STRIP arm-linux-gnueabi-strip)
set(CMAKE_RANLIB arm-linux-gnueabi-ranlib)

# Where is the target environment located
set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabi /opt/vc)

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Additional include directories for Raspberry Pi headers
include_directories(/opt/vc/include)
include_directories(/opt/vc/include/interface/vcos/pthreads)
include_directories(/opt/vc/include/interface/vmcs_host/linux)
