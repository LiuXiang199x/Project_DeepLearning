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
CMAKE_SOURCE_DIR = /home/xiang/Estudiar/AI部署/Version1/RL_api/rknn_RL

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/xiang/Estudiar/AI部署/Version1/RL_api/rknn_RL/build

# Include any dependencies generated for this target.
include CMakeFiles/rl_api.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/rl_api.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rl_api.dir/flags.make

CMakeFiles/rl_api.dir/src/main.cc.o: CMakeFiles/rl_api.dir/flags.make
CMakeFiles/rl_api.dir/src/main.cc.o: ../src/main.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xiang/Estudiar/AI部署/Version1/RL_api/rknn_RL/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/rl_api.dir/src/main.cc.o"
	aarch64-linux-gnu-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/rl_api.dir/src/main.cc.o -c /home/xiang/Estudiar/AI部署/Version1/RL_api/rknn_RL/src/main.cc

CMakeFiles/rl_api.dir/src/main.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rl_api.dir/src/main.cc.i"
	aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xiang/Estudiar/AI部署/Version1/RL_api/rknn_RL/src/main.cc > CMakeFiles/rl_api.dir/src/main.cc.i

CMakeFiles/rl_api.dir/src/main.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rl_api.dir/src/main.cc.s"
	aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xiang/Estudiar/AI部署/Version1/RL_api/rknn_RL/src/main.cc -o CMakeFiles/rl_api.dir/src/main.cc.s

CMakeFiles/rl_api.dir/src/main.cc.o.requires:

.PHONY : CMakeFiles/rl_api.dir/src/main.cc.o.requires

CMakeFiles/rl_api.dir/src/main.cc.o.provides: CMakeFiles/rl_api.dir/src/main.cc.o.requires
	$(MAKE) -f CMakeFiles/rl_api.dir/build.make CMakeFiles/rl_api.dir/src/main.cc.o.provides.build
.PHONY : CMakeFiles/rl_api.dir/src/main.cc.o.provides

CMakeFiles/rl_api.dir/src/main.cc.o.provides.build: CMakeFiles/rl_api.dir/src/main.cc.o


# Object files for target rl_api
rl_api_OBJECTS = \
"CMakeFiles/rl_api.dir/src/main.cc.o"

# External object files for target rl_api
rl_api_EXTERNAL_OBJECTS =

../bin/rl_api: CMakeFiles/rl_api.dir/src/main.cc.o
../bin/rl_api: CMakeFiles/rl_api.dir/build.make
../bin/rl_api: ../../librknn_api/lib64/librknn_api.so
../bin/rl_api: ../../mrpt_api/libmrpt-base.so
../bin/rl_api: ../../mrpt_api/libmrpt-maps.so
../bin/rl_api: ../../mrpt_api/libmrpt-obs.so
../bin/rl_api: ../../mrpt_api/libmrpt-scanmatching.so
../bin/rl_api: ../../mrpt_api/libmrpt-slam.so
../bin/rl_api: CMakeFiles/rl_api.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/xiang/Estudiar/AI部署/Version1/RL_api/rknn_RL/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/rl_api"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rl_api.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rl_api.dir/build: ../bin/rl_api

.PHONY : CMakeFiles/rl_api.dir/build

CMakeFiles/rl_api.dir/requires: CMakeFiles/rl_api.dir/src/main.cc.o.requires

.PHONY : CMakeFiles/rl_api.dir/requires

CMakeFiles/rl_api.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/rl_api.dir/cmake_clean.cmake
.PHONY : CMakeFiles/rl_api.dir/clean

CMakeFiles/rl_api.dir/depend:
	cd /home/xiang/Estudiar/AI部署/Version1/RL_api/rknn_RL/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/xiang/Estudiar/AI部署/Version1/RL_api/rknn_RL /home/xiang/Estudiar/AI部署/Version1/RL_api/rknn_RL /home/xiang/Estudiar/AI部署/Version1/RL_api/rknn_RL/build /home/xiang/Estudiar/AI部署/Version1/RL_api/rknn_RL/build /home/xiang/Estudiar/AI部署/Version1/RL_api/rknn_RL/build/CMakeFiles/rl_api.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/rl_api.dir/depend

