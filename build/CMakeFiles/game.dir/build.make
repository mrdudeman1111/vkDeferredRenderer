# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /run/media/ethanw/LinuxGames/Repos/Trolling

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /run/media/ethanw/LinuxGames/Repos/Trolling/build

# Include any dependencies generated for this target.
include CMakeFiles/game.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/game.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/game.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/game.dir/flags.make

CMakeFiles/game.dir/Descriptor.cpp.o: CMakeFiles/game.dir/flags.make
CMakeFiles/game.dir/Descriptor.cpp.o: /run/media/ethanw/LinuxGames/Repos/Trolling/Descriptor.cpp
CMakeFiles/game.dir/Descriptor.cpp.o: CMakeFiles/game.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/run/media/ethanw/LinuxGames/Repos/Trolling/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/game.dir/Descriptor.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/game.dir/Descriptor.cpp.o -MF CMakeFiles/game.dir/Descriptor.cpp.o.d -o CMakeFiles/game.dir/Descriptor.cpp.o -c /run/media/ethanw/LinuxGames/Repos/Trolling/Descriptor.cpp

CMakeFiles/game.dir/Descriptor.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/game.dir/Descriptor.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /run/media/ethanw/LinuxGames/Repos/Trolling/Descriptor.cpp > CMakeFiles/game.dir/Descriptor.cpp.i

CMakeFiles/game.dir/Descriptor.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/game.dir/Descriptor.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /run/media/ethanw/LinuxGames/Repos/Trolling/Descriptor.cpp -o CMakeFiles/game.dir/Descriptor.cpp.s

CMakeFiles/game.dir/Pipeline.cpp.o: CMakeFiles/game.dir/flags.make
CMakeFiles/game.dir/Pipeline.cpp.o: /run/media/ethanw/LinuxGames/Repos/Trolling/Pipeline.cpp
CMakeFiles/game.dir/Pipeline.cpp.o: CMakeFiles/game.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/run/media/ethanw/LinuxGames/Repos/Trolling/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/game.dir/Pipeline.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/game.dir/Pipeline.cpp.o -MF CMakeFiles/game.dir/Pipeline.cpp.o.d -o CMakeFiles/game.dir/Pipeline.cpp.o -c /run/media/ethanw/LinuxGames/Repos/Trolling/Pipeline.cpp

CMakeFiles/game.dir/Pipeline.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/game.dir/Pipeline.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /run/media/ethanw/LinuxGames/Repos/Trolling/Pipeline.cpp > CMakeFiles/game.dir/Pipeline.cpp.i

CMakeFiles/game.dir/Pipeline.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/game.dir/Pipeline.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /run/media/ethanw/LinuxGames/Repos/Trolling/Pipeline.cpp -o CMakeFiles/game.dir/Pipeline.cpp.s

CMakeFiles/game.dir/Renderer.cpp.o: CMakeFiles/game.dir/flags.make
CMakeFiles/game.dir/Renderer.cpp.o: /run/media/ethanw/LinuxGames/Repos/Trolling/Renderer.cpp
CMakeFiles/game.dir/Renderer.cpp.o: CMakeFiles/game.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/run/media/ethanw/LinuxGames/Repos/Trolling/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/game.dir/Renderer.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/game.dir/Renderer.cpp.o -MF CMakeFiles/game.dir/Renderer.cpp.o.d -o CMakeFiles/game.dir/Renderer.cpp.o -c /run/media/ethanw/LinuxGames/Repos/Trolling/Renderer.cpp

CMakeFiles/game.dir/Renderer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/game.dir/Renderer.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /run/media/ethanw/LinuxGames/Repos/Trolling/Renderer.cpp > CMakeFiles/game.dir/Renderer.cpp.i

CMakeFiles/game.dir/Renderer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/game.dir/Renderer.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /run/media/ethanw/LinuxGames/Repos/Trolling/Renderer.cpp -o CMakeFiles/game.dir/Renderer.cpp.s

CMakeFiles/game.dir/main.cpp.o: CMakeFiles/game.dir/flags.make
CMakeFiles/game.dir/main.cpp.o: /run/media/ethanw/LinuxGames/Repos/Trolling/main.cpp
CMakeFiles/game.dir/main.cpp.o: CMakeFiles/game.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/run/media/ethanw/LinuxGames/Repos/Trolling/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/game.dir/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/game.dir/main.cpp.o -MF CMakeFiles/game.dir/main.cpp.o.d -o CMakeFiles/game.dir/main.cpp.o -c /run/media/ethanw/LinuxGames/Repos/Trolling/main.cpp

CMakeFiles/game.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/game.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /run/media/ethanw/LinuxGames/Repos/Trolling/main.cpp > CMakeFiles/game.dir/main.cpp.i

CMakeFiles/game.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/game.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /run/media/ethanw/LinuxGames/Repos/Trolling/main.cpp -o CMakeFiles/game.dir/main.cpp.s

# Object files for target game
game_OBJECTS = \
"CMakeFiles/game.dir/Descriptor.cpp.o" \
"CMakeFiles/game.dir/Pipeline.cpp.o" \
"CMakeFiles/game.dir/Renderer.cpp.o" \
"CMakeFiles/game.dir/main.cpp.o"

# External object files for target game
game_EXTERNAL_OBJECTS =

game: CMakeFiles/game.dir/Descriptor.cpp.o
game: CMakeFiles/game.dir/Pipeline.cpp.o
game: CMakeFiles/game.dir/Renderer.cpp.o
game: CMakeFiles/game.dir/main.cpp.o
game: CMakeFiles/game.dir/build.make
game: CMakeFiles/game.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/run/media/ethanw/LinuxGames/Repos/Trolling/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable game"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/game.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/game.dir/build: game
.PHONY : CMakeFiles/game.dir/build

CMakeFiles/game.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/game.dir/cmake_clean.cmake
.PHONY : CMakeFiles/game.dir/clean

CMakeFiles/game.dir/depend:
	cd /run/media/ethanw/LinuxGames/Repos/Trolling/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /run/media/ethanw/LinuxGames/Repos/Trolling /run/media/ethanw/LinuxGames/Repos/Trolling /run/media/ethanw/LinuxGames/Repos/Trolling/build /run/media/ethanw/LinuxGames/Repos/Trolling/build /run/media/ethanw/LinuxGames/Repos/Trolling/build/CMakeFiles/game.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/game.dir/depend

