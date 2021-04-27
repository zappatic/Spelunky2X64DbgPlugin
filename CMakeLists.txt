# At least 3.5 is required for CMAKE_MSVC_RUNTIME_LIBRARY
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

# Set up a more familiar Visual Studio configuration
# Override these options with -DCMAKE_OPTION=Value
#
# See: https://cmake.org/cmake/help/latest/command/set.html#set-cache-entry
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO" CACHE STRING "") # /DEBUG:FULL
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO" CACHE STRING "") # /DEBUG:FULL
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

# Plugin project
project(Spelunky2)
set(CMAKE_CXX_STANDARD 17)

# Enable solution folder support
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# https://github.com/TheLartians/CPM.cmake/releases
include(CPM.cmake)

# Add x64dbg plugin SDK dependency
CPMAddPackage(
	NAME x64dbg
	GITHUB_REPOSITORY x64dbg/sdk
	GIT_TAG x64dbg_9f9b1ba1c03ffb9dc921b3e1f8615412c65723f5
)

# Add Qt5
set(Qt5ROOT "C:/Qt/qt-5.6.3-x64-msvc2013/5.6.3/msvc2013_64")
set(Qt5Core_DIR "${Qt5ROOT}/lib/cmake/Qt5Core")
set(Qt5Widgets_DIR "${Qt5ROOT}/lib/cmake/Qt5Widgets")

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
if(QT_NAMESPACE)
    add_definitions(-DQT_NAMESPACE=${QT_NAMESPACE})
endif()


# Create a plugin
configure_file(config/pluginconfig.h.in include/pluginconfig.h ESCAPE_QUOTES)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

qt5_add_resources(SOURCES resources/spelunky2.qrc)

x64dbg_plugin(${PROJECT_NAME}
	src/pluginmain.cpp
	src/QtPlugin.cpp
	include/Spelunky2.h
	include/Data/MemoryMappedData.h
	include/Data/EntityDB.h
	include/Data/Entity.h
	include/Data/EntityList.h
	include/Data/State.h
	include/Data/Texture.h
	include/Views/ViewToolbar.h
	include/Views/ViewEntityDB.h
	include/Views/ViewEntity.h
	include/Views/ViewEntities.h
	include/Views/ViewState.h
	include/QtHelpers/HTMLDelegate.h
	include/QtHelpers/TreeViewMemoryFields.h
	src/Spelunky2.cpp
	src/Data/MemoryMappedData.cpp
	src/Data/EntityDB.cpp
	src/Data/Entity.cpp
	src/Data/EntityList.cpp
	src/Data/State.cpp
	src/Data/Texture.cpp
	src/Views/ViewToolbar.cpp
	src/Views/ViewEntityDB.cpp
	src/Views/ViewEntity.cpp
	src/Views/ViewEntities.cpp
	src/Views/ViewState.cpp
	src/QtHelpers/HTMLDelegate.cpp
	src/QtHelpers/TreeViewMemoryFields.cpp
	${CMAKE_CURRENT_BINARY_DIR}/include/pluginconfig.h
	resources/spelunky2.qrc
)

target_include_directories(${PROJECT_NAME} PRIVATE ${Qt5ROOT}/include/QtCore)
target_include_directories(${PROJECT_NAME} PRIVATE ${Qt5ROOT}/include/QtWidgets)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets)

# Set the plugin as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Copy the plugin to the x64dbg plugins folder
add_custom_command(	TARGET ${PROJECT_NAME} 
					POST_BUILD
					COMMAND  ${CMAKE_COMMAND} -E "copy" "${CMAKE_CURRENT_BINARY_DIR}/Release/Spelunky2.dp64" "C:/Users/main/Documents/git/x64dbg/bin/x64/plugins/")