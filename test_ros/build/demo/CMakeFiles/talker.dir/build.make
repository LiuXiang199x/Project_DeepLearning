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
CMAKE_SOURCE_DIR = /home/xiang/test_ros/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/xiang/test_ros/build

# Include any dependencies generated for this target.
include demo/CMakeFiles/talker.dir/depend.make

# Include the progress variables for this target.
include demo/CMakeFiles/talker.dir/progress.make

# Include the compile flags for this target's objects.
include demo/CMakeFiles/talker.dir/flags.make

demo/CMakeFiles/talker.dir/requires:

.PHONY : demo/CMakeFiles/talker.dir/requires

demo/CMakeFiles/talker.dir/clean:
	cd /home/xiang/test_ros/build/demo && $(CMAKE_COMMAND) -P CMakeFiles/talker.dir/cmake_clean.cmake
.PHONY : demo/CMakeFiles/talker.dir/clean

demo/CMakeFiles/talker.dir/depend:
	cd /home/xiang/test_ros/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/xiang/test_ros/src /home/xiang/test_ros/src/demo /home/xiang/test_ros/build /home/xiang/test_ros/build/demo /home/xiang/test_ros/build/demo/CMakeFiles/talker.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : demo/CMakeFiles/talker.dir/depend

