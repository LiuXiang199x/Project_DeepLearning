# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/xiang/Estudiar/AI部署/test_perform_model/test_performance

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/xiang/Estudiar/AI部署/test_perform_model/test_performance/build

# Include any dependencies generated for this target.
include CMakeFiles/test_performance.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/test_performance.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_performance.dir/flags.make

CMakeFiles/test_performance.dir/src/main.cc.o: CMakeFiles/test_performance.dir/flags.make
CMakeFiles/test_performance.dir/src/main.cc.o: ../src/main.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xiang/Estudiar/AI部署/test_perform_model/test_performance/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_performance.dir/src/main.cc.o"
	aarch64-linux-gnu-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_performance.dir/src/main.cc.o -c /home/xiang/Estudiar/AI部署/test_perform_model/test_performance/src/main.cc

CMakeFiles/test_performance.dir/src/main.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_performance.dir/src/main.cc.i"
	aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xiang/Estudiar/AI部署/test_perform_model/test_performance/src/main.cc > CMakeFiles/test_performance.dir/src/main.cc.i

CMakeFiles/test_performance.dir/src/main.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_performance.dir/src/main.cc.s"
	aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xiang/Estudiar/AI部署/test_perform_model/test_performance/src/main.cc -o CMakeFiles/test_performance.dir/src/main.cc.s

CMakeFiles/test_performance.dir/src/main.cc.o.requires:

.PHONY : CMakeFiles/test_performance.dir/src/main.cc.o.requires

CMakeFiles/test_performance.dir/src/main.cc.o.provides: CMakeFiles/test_performance.dir/src/main.cc.o.requires
	$(MAKE) -f CMakeFiles/test_performance.dir/build.make CMakeFiles/test_performance.dir/src/main.cc.o.provides.build
.PHONY : CMakeFiles/test_performance.dir/src/main.cc.o.provides

CMakeFiles/test_performance.dir/src/main.cc.o.provides.build: CMakeFiles/test_performance.dir/src/main.cc.o


# Object files for target test_performance
test_performance_OBJECTS = \
"CMakeFiles/test_performance.dir/src/main.cc.o"

# External object files for target test_performance
test_performance_EXTERNAL_OBJECTS =

../bin/test_performance: CMakeFiles/test_performance.dir/src/main.cc.o
../bin/test_performance: CMakeFiles/test_performance.dir/build.make
../bin/test_performance: ../../librknn_api/lib64/librknn_api.so
../bin/test_performance: CMakeFiles/test_performance.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/xiang/Estudiar/AI部署/test_perform_model/test_performance/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/test_performance"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_performance.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_performance.dir/build: ../bin/test_performance

.PHONY : CMakeFiles/test_performance.dir/build

CMakeFiles/test_performance.dir/requires: CMakeFiles/test_performance.dir/src/main.cc.o.requires

.PHONY : CMakeFiles/test_performance.dir/requires

CMakeFiles/test_performance.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_performance.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_performance.dir/clean

CMakeFiles/test_performance.dir/depend:
	cd /home/xiang/Estudiar/AI部署/test_perform_model/test_performance/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/xiang/Estudiar/AI部署/test_perform_model/test_performance /home/xiang/Estudiar/AI部署/test_perform_model/test_performance /home/xiang/Estudiar/AI部署/test_perform_model/test_performance/build /home/xiang/Estudiar/AI部署/test_perform_model/test_performance/build /home/xiang/Estudiar/AI部署/test_perform_model/test_performance/build/CMakeFiles/test_performance.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_performance.dir/depend

