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
CMAKE_SOURCE_DIR = /home/xiang/Estudiar/AI部署/RL-SLAM/RL_api3/RlApi

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/xiang/Estudiar/AI部署/RL-SLAM/RL_api3/RlApi/build

# Include any dependencies generated for this target.
include CMakeFiles/RlApi.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/RlApi.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/RlApi.dir/flags.make

CMakeFiles/RlApi.dir/src/main.cc.o: CMakeFiles/RlApi.dir/flags.make
CMakeFiles/RlApi.dir/src/main.cc.o: ../src/main.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xiang/Estudiar/AI部署/RL-SLAM/RL_api3/RlApi/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/RlApi.dir/src/main.cc.o"
	aarch64-linux-gnu-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/RlApi.dir/src/main.cc.o -c /home/xiang/Estudiar/AI部署/RL-SLAM/RL_api3/RlApi/src/main.cc

CMakeFiles/RlApi.dir/src/main.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RlApi.dir/src/main.cc.i"
	aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xiang/Estudiar/AI部署/RL-SLAM/RL_api3/RlApi/src/main.cc > CMakeFiles/RlApi.dir/src/main.cc.i

CMakeFiles/RlApi.dir/src/main.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RlApi.dir/src/main.cc.s"
	aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xiang/Estudiar/AI部署/RL-SLAM/RL_api3/RlApi/src/main.cc -o CMakeFiles/RlApi.dir/src/main.cc.s

CMakeFiles/RlApi.dir/src/main.cc.o.requires:

.PHONY : CMakeFiles/RlApi.dir/src/main.cc.o.requires

CMakeFiles/RlApi.dir/src/main.cc.o.provides: CMakeFiles/RlApi.dir/src/main.cc.o.requires
	$(MAKE) -f CMakeFiles/RlApi.dir/build.make CMakeFiles/RlApi.dir/src/main.cc.o.provides.build
.PHONY : CMakeFiles/RlApi.dir/src/main.cc.o.provides

CMakeFiles/RlApi.dir/src/main.cc.o.provides.build: CMakeFiles/RlApi.dir/src/main.cc.o


# Object files for target RlApi
RlApi_OBJECTS = \
"CMakeFiles/RlApi.dir/src/main.cc.o"

# External object files for target RlApi
RlApi_EXTERNAL_OBJECTS =

../bin/RlApi: CMakeFiles/RlApi.dir/src/main.cc.o
../bin/RlApi: CMakeFiles/RlApi.dir/build.make
../bin/RlApi: ../../librknn_api/lib64/librknn_api.so
../bin/RlApi: ../../mrpt_api/libmrpt-base.so
../bin/RlApi: ../../mrpt_api/libmrpt-hwdrivers.so
../bin/RlApi: ../../mrpt_api/libmrpt-maps.so
../bin/RlApi: ../../mrpt_api/libmrpt-obs.so
../bin/RlApi: ../../mrpt_api/libmrpt-scanmatching.so
../bin/RlApi: ../../mrpt_api/libmrpt-slam.so
../bin/RlApi: CMakeFiles/RlApi.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/xiang/Estudiar/AI部署/RL-SLAM/RL_api3/RlApi/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/RlApi"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/RlApi.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/RlApi.dir/build: ../bin/RlApi

.PHONY : CMakeFiles/RlApi.dir/build

CMakeFiles/RlApi.dir/requires: CMakeFiles/RlApi.dir/src/main.cc.o.requires

.PHONY : CMakeFiles/RlApi.dir/requires

CMakeFiles/RlApi.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/RlApi.dir/cmake_clean.cmake
.PHONY : CMakeFiles/RlApi.dir/clean

CMakeFiles/RlApi.dir/depend:
	cd /home/xiang/Estudiar/AI部署/RL-SLAM/RL_api3/RlApi/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/xiang/Estudiar/AI部署/RL-SLAM/RL_api3/RlApi /home/xiang/Estudiar/AI部署/RL-SLAM/RL_api3/RlApi /home/xiang/Estudiar/AI部署/RL-SLAM/RL_api3/RlApi/build /home/xiang/Estudiar/AI部署/RL-SLAM/RL_api3/RlApi/build /home/xiang/Estudiar/AI部署/RL-SLAM/RL_api3/RlApi/build/CMakeFiles/RlApi.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/RlApi.dir/depend

