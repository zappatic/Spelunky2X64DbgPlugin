# x64dbg plugin SDK

Repository with the latest pluginsdk versions for x64dbg. Automatically updated when a new x64dbg snapshot is released.

## Contents

- `pluginsdk`: Headers and libraries for x64bg's plugin sdk
- `commithash.txt`: Commit hash of the [x64dbg repo](https://github.com/x64dbg/x64dbg).
- `LICENSE`: Boost Software License - Version 1.0 - August 17th, 2003. You are free to develop plugins for any usage under any license you like. Remember that x64dbg itself is licensed under GPL though.

## Usage

`CMakeLists.txt`:

```cmake
# Use a reasonably modern CMake version
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

# Plugin project
project(MyPlugin)

# Enable solution folder support
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# https://github.com/TheLartians/CPM.cmake/releases
include(cmake/CPM.cmake)

# Add x64dbg plugin SDK dependency
CPMAddPackage(
	NAME x64dbg
	GITHUB_REPOSITORY https://github.com/x64dbg/sdk
	GIT_TAG x64dbg_9f9b1ba1c03ffb9dc921b3e1f8615412c65723f5
)

# Create a plugin
x64dbg_plugin(${PROJECT_NAME}
	plugin.cpp
)
```
