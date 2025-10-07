CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -O2 -std=c99
CXXFLAGS = -Wall -Wextra -O2 -std=c++17

# Detect Raspberry Pi with MMAL support
RASPBERRY_PI = $(shell test -f /opt/vc/include/interface/mmal/mmal.h && echo "yes" || echo "no")
MMAL_LIBS = $(shell test -f /opt/vc/lib/libmmal.so && echo "yes" || echo "no")

ifeq ($(RASPBERRY_PI),yes)
    ifeq ($(MMAL_LIBS),yes)
        # Raspberry Pi configuration - hardware acceleration enabled
        INCLUDES = -Iinclude -Isrc -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux
        LIBS = -L/opt/vc/lib -lmmal -lmmal_core -lmmal_util -lmmal_vc_client -lvcos -lbcm_host
        CFLAGS += -DRASPBERRY_PI
        $(info ✅ MMAL libraries found - hardware acceleration enabled)
    else
        # MMAL headers found but libraries missing - use NO_HARDWARE mode
        INCLUDES = -Iinclude -Isrc
        LIBS = 
        CFLAGS += -DRASPBERRY_PI -DNO_HARDWARE
        $(warning ⚠️  MMAL headers found but libraries missing - using NO_HARDWARE mode)
    endif
else
    # Non-Pi systems or MMAL not available - hardware not available
    INCLUDES = -Iinclude -Isrc
    LIBS = 
    CFLAGS += -DNO_HARDWARE
    $(info ℹ️  MMAL not available - using NO_HARDWARE mode)
endif

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
TMP_DIR = tmp
EXAMPLES_DIR = examples
TESTS_DIR = tests

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

# Hardware components are always included
SOURCES += src/h264_hw_decoder.c src/mjpeg_hw_encoder.c
OBJECTS += $(BUILD_DIR)/h264_hw_decoder.o $(BUILD_DIR)/mjpeg_hw_encoder.o

# Targets
LIBRARY = $(BUILD_DIR)/libh264_to_jpeg.a
EXAMPLE = $(BUILD_DIR)/example
V4L2_TEST = $(BUILD_DIR)/v4l2_h264_test
TEST = $(BUILD_DIR)/test_h264_to_jpeg
MINIMAL_TEST = $(BUILD_DIR)/minimal_test
SAFE_TEST = $(BUILD_DIR)/safe_test
DEBUG_TEST = $(BUILD_DIR)/debug_test
HELLO = $(BUILD_DIR)/hello
PI_ZERO_TEST = $(BUILD_DIR)/pi_zero_test

.PHONY: all clean test example v4l2_test minimal_test safe_test debug_test hello pi_zero_test install

all: $(LIBRARY) $(EXAMPLE) $(V4L2_TEST)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(TMP_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIBRARY): $(OBJECTS) | $(BUILD_DIR)
	ar rcs $@ $(OBJECTS)

$(EXAMPLE): $(EXAMPLES_DIR)/simple_debug.c $(LIBRARY) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBRARY) $(LIBS)

$(V4L2_TEST): $(EXAMPLES_DIR)/v4l2_h264_test.c $(LIBRARY) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBRARY) $(LIBS)

# Hardware test example
HW_TEST = $(BUILD_DIR)/hw_test
$(HW_TEST): $(EXAMPLES_DIR)/hw_test.c $(LIBRARY) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBRARY) $(LIBS)

$(TEST): $(TESTS_DIR)/simple_test.c $(LIBRARY) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBRARY) $(LIBS)

# Minimal test (no library dependencies)
$(MINIMAL_TEST): $(EXAMPLES_DIR)/minimal_test.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

# Safe test (forced NO_HARDWARE mode)
$(SAFE_TEST): $(EXAMPLES_DIR)/safe_test.c $(LIBRARY) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -DNO_HARDWARE -ffloat-store -fno-math-errno -fno-trapping-math -fno-signaling-nans -o $@ $< $(LIBRARY) $(LIBS)

# Debug test
$(DEBUG_TEST): $(EXAMPLES_DIR)/debug_test.c $(LIBRARY) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -ffloat-store -fno-math-errno -fno-trapping-math -fno-signaling-nans -o $@ $< $(LIBRARY) $(LIBS)

# Hello test (absolute minimal)
$(HELLO): $(EXAMPLES_DIR)/hello.c | $(BUILD_DIR)
	$(CC) -o $@ $<

# Pi Zero test (ARMv6 optimized)
$(PI_ZERO_TEST): $(EXAMPLES_DIR)/pi_zero_test.c | $(BUILD_DIR)
	$(CC) -march=armv6 -mfloat-abi=softfp -marm -o $@ $<

# No MMAL test (no hardware dependencies)
NO_MMAL_TEST = $(BUILD_DIR)/no_mmal_test
$(NO_MMAL_TEST): $(EXAMPLES_DIR)/no_mmal_test.c | $(BUILD_DIR)
	$(CC) -ffloat-store -fno-math-errno -fno-trapping-math -fno-signaling-nans -o $@ $<

# Ultra minimal test (no project headers)
ULTRA_MINIMAL_TEST = $(BUILD_DIR)/ultra_minimal_test
$(ULTRA_MINIMAL_TEST): $(EXAMPLES_DIR)/ultra_minimal_test.c | $(BUILD_DIR)
	$(CC) -ffloat-store -fno-math-errno -fno-trapping-math -fno-signaling-nans -o $@ $<

# Ultra minimal test with soft-float flags
ULTRA_MINIMAL_SOFT_TEST = $(BUILD_DIR)/ultra_minimal_soft_test
$(ULTRA_MINIMAL_SOFT_TEST): $(EXAMPLES_DIR)/ultra_minimal_test.c | $(BUILD_DIR)
	$(CC) -march=armv6 -mfloat-abi=softfp -marm -ffloat-store -fno-math-errno -fno-trapping-math -fno-signaling-nans -o $@ $<

# Ultra minimal test with no floating point
ULTRA_MINIMAL_NOFP_TEST = $(BUILD_DIR)/ultra_minimal_nofp_test
$(ULTRA_MINIMAL_NOFP_TEST): $(EXAMPLES_DIR)/ultra_minimal_test.c | $(BUILD_DIR)
	$(CC) -march=armv6 -mfloat-abi=softfp -marm -ffloat-store -fno-math-errno -fno-trapping-math -fno-signaling-nans -o $@ $<

# Ultra minimal test with static linking
ULTRA_MINIMAL_STATIC_TEST = $(BUILD_DIR)/ultra_minimal_static_test
$(ULTRA_MINIMAL_STATIC_TEST): $(EXAMPLES_DIR)/ultra_minimal_test.c | $(BUILD_DIR)
	$(CC) -march=armv6 -mfloat-abi=softfp -marm -static -ffloat-store -fno-math-errno -fno-trapping-math -fno-signaling-nans -o $@ $<

# No project headers test (simulates project structures without including headers)
NO_PROJECT_HEADERS_TEST = $(BUILD_DIR)/no_project_headers_test
$(NO_PROJECT_HEADERS_TEST): $(EXAMPLES_DIR)/no_project_headers_test.c | $(BUILD_DIR)
	$(CC) -march=armv6 -mfloat-abi=softfp -marm -ffloat-store -fno-math-errno -fno-trapping-math -fno-signaling-nans -o $@ $<

# Syscall test (only system calls, no stdio)
SYSCALL_TEST = $(BUILD_DIR)/syscall_test
$(SYSCALL_TEST): $(EXAMPLES_DIR)/syscall_test.c | $(BUILD_DIR)
	$(CC) -march=armv6 -mfloat-abi=softfp -marm -ffloat-store -fno-math-errno -fno-trapping-math -fno-signaling-nans -o $@ $<

# No MMAL headers test (simulates project functions without MMAL headers)
NO_MMAL_HEADERS_TEST = $(BUILD_DIR)/no_mmal_headers_test
$(NO_MMAL_HEADERS_TEST): $(EXAMPLES_DIR)/no_mmal_headers_test.c | $(BUILD_DIR)
	$(CC) -march=armv6 -mfloat-abi=softfp -marm -ffloat-store -fno-math-errno -fno-trapping-math -fno-signaling-nans -o $@ $<

# No project includes test (simulates ALL project functions without ANY includes)
NO_PROJECT_INCLUDES_TEST = $(BUILD_DIR)/no_project_includes_test
$(NO_PROJECT_INCLUDES_TEST): $(EXAMPLES_DIR)/no_project_includes_test.c | $(BUILD_DIR)
	$(CC) -march=armv6 -mfloat-abi=softfp -marm -ffloat-store -fno-math-errno -o $@ $<

# No project includes test with strict no-float flags
NO_PROJECT_INCLUDES_STRICT_TEST = $(BUILD_DIR)/no_project_includes_strict_test
$(NO_PROJECT_INCLUDES_STRICT_TEST): $(EXAMPLES_DIR)/no_project_includes_test.c | $(BUILD_DIR)
	$(CC) -march=armv6 -mfloat-abi=softfp -marm -ffloat-store -fno-math-errno -fno-trapping-math -fno-signaling-nans -o $@ $<

test: $(TEST)
	./$(TEST)

example: $(EXAMPLE)
	./$(EXAMPLE)

v4l2_test: $(V4L2_TEST)
	@echo "V4L2 H.264 test compiled successfully!"
	@echo "Usage: ./$(V4L2_TEST) [device] [width] [height]"
	@echo "Example: ./$(V4L2_TEST) /dev/video0 1280 720"

hw_test: $(HW_TEST)
	./$(HW_TEST)

minimal_test: $(MINIMAL_TEST)
	./$(MINIMAL_TEST)

safe_test: $(SAFE_TEST)
	./$(SAFE_TEST)

debug_test: $(DEBUG_TEST)
	./$(DEBUG_TEST)

ultra_minimal_test: $(ULTRA_MINIMAL_TEST)
	./$(ULTRA_MINIMAL_TEST)

no_project_headers_test: $(NO_PROJECT_HEADERS_TEST)
	./$(NO_PROJECT_HEADERS_TEST)

syscall_test: $(SYSCALL_TEST)
	./$(SYSCALL_TEST)

no_mmal_headers_test: $(NO_MMAL_HEADERS_TEST)
	./$(NO_MMAL_HEADERS_TEST)

no_project_includes_test: $(NO_PROJECT_INCLUDES_TEST)
	./$(NO_PROJECT_INCLUDES_TEST)

no_project_includes_strict_test: $(NO_PROJECT_INCLUDES_STRICT_TEST)
	./$(NO_PROJECT_INCLUDES_STRICT_TEST)

hello: $(HELLO)
	./$(HELLO)

pi_zero_test: $(PI_ZERO_TEST)
	./$(PI_ZERO_TEST)

clean:
	rm -rf $(BUILD_DIR) $(TMP_DIR)

install: $(LIBRARY)
	@echo "Installing library and headers..."
	@mkdir -p /usr/local/lib
	@mkdir -p /usr/local/include/h264_to_jpeg
	@cp $(LIBRARY) /usr/local/lib/
	@cp $(HEADERS) /usr/local/include/h264_to_jpeg/
	@echo "Installation complete"

# Development targets
debug: CFLAGS += -g -DDEBUG
debug: $(LIBRARY) $(EXAMPLE)

.PHONY: help ultra_minimal_test no_project_headers_test syscall_test no_mmal_headers_test
help:
	@echo "Available targets:"
	@echo "  all      - Build library and example"
	@echo "  clean    - Remove build files"
	@echo "  test     - Run tests"
	@echo "  example  - Run example"
	@echo "  install  - Install library and headers"
	@echo "  debug    - Build with debug symbols"
	@echo "  no_mmal_test - Build no MMAL test"
	@echo "  ultra_minimal_test - Build ultra minimal test"
	@echo "  no_project_headers_test - Build no project headers test"
	@echo "  syscall_test - Build syscall test"
	@echo "  no_mmal_headers_test - Build no MMAL headers test"
