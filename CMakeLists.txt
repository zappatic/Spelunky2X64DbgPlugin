cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(Qt5ROOT "" CACHE PATH "Path to Qt 5.6.3 folder")
set(X64DBG_PLUGINS_ROOT "" CACHE PATH "Path to 64-bit plugins folder of x64dbg")

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO" CACHE STRING "") # /DEBUG:FULL
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO" CACHE STRING "") # /DEBUG:FULL
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

project(Spelunky2)
set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0)
add_compile_definitions(NOMINMAX)
add_compile_definitions(WIN32_LEAN_AND_MEAN)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CPM.cmake)

CPMAddPackage(
	NAME x64dbg
	GITHUB_REPOSITORY x64dbg/sdk
	GIT_TAG x64dbg_9f9b1ba1c03ffb9dc921b3e1f8615412c65723f5
)

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
	include/Configuration.h
	include/Data/MemoryMappedData.h
	include/Data/EntityDB.h
	include/Data/Entity.h
	include/Data/State.h
	include/Data/LevelGen.h
	include/Data/ParticleDB.h
	include/Data/TextureDB.h
	include/Data/IDNameList.h
	include/Data/EntityList.h
	include/Data/ParticleEmittersList.h
	include/Data/VirtualTableLookup.h
	include/Data/StringsTable.h
	include/Data/CPPGenerator.h
	include/Views/ViewToolbar.h
	include/Views/ViewEntityDB.h
	include/Views/ViewParticleDB.h
	include/Views/ViewTextureDB.h
	include/Views/ViewEntity.h
	include/Views/ViewEntities.h
	include/Views/ViewState.h
	include/Views/ViewLevelGen.h
	include/Views/ViewVirtualTable.h
	include/Views/ViewStringsTable.h
	include/QtHelpers/HTMLDelegate.h
	include/QtHelpers/TreeViewMemoryFields.h
	include/QtHelpers/WidgetMemoryView.h
	include/QtHelpers/WidgetSpelunkyLevel.h
	include/QtHelpers/WidgetSpelunkyRooms.h
	include/QtHelpers/DialogEditSimpleValue.h
	include/QtHelpers/TableWidgetItemNumeric.h
	include/QtHelpers/TreeWidgetItemNumeric.h
	include/QtHelpers/ItemModelVirtualTable.h
	include/QtHelpers/ItemModelStringsTable.h
	include/QtHelpers/CPPSyntaxHighlighter.h
	src/Spelunky2.cpp
	src/Configuration.cpp
	src/Data/MemoryMappedData.cpp
	src/Data/EntityDB.cpp
	src/Data/Entity.cpp
	src/Data/State.cpp
	src/Data/LevelGen.cpp
	src/Data/ParticleDB.cpp
	src/Data/TextureDB.cpp
	src/Data/IDNameList.cpp
	src/Data/EntityList.cpp
	src/Data/ParticleEmittersList.cpp
	src/Data/VirtualTableLookup.cpp
	src/Data/StringsTable.cpp
	src/Data/CPPGenerator.cpp
	src/Views/ViewToolbar.cpp
	src/Views/ViewEntityDB.cpp
	src/Views/ViewParticleDB.cpp
	src/Views/ViewTextureDB.cpp
	src/Views/ViewEntity.cpp
	src/Views/ViewEntities.cpp
	src/Views/ViewState.cpp
	src/Views/ViewLevelGen.cpp
	src/Views/ViewVirtualTable.cpp
	src/Views/ViewStringsTable.cpp
	src/QtHelpers/HTMLDelegate.cpp
	src/QtHelpers/TreeViewMemoryFields.cpp
	src/QtHelpers/WidgetMemoryView.cpp
	src/QtHelpers/WidgetSpelunkyLevel.cpp
	src/QtHelpers/WidgetSpelunkyRooms.cpp
	src/QtHelpers/DialogEditSimpleValue.cpp
	src/QtHelpers/TableWidgetItemNumeric.cpp
	src/QtHelpers/TreeWidgetItemNumeric.cpp
	src/QtHelpers/ItemModelVirtualTable.cpp
	src/QtHelpers/ItemModelStringsTable.cpp
	src/QtHelpers/CPPSyntaxHighlighter.cpp
	${CMAKE_CURRENT_BINARY_DIR}/include/pluginconfig.h
	resources/spelunky2.qrc
)

target_include_directories(${PROJECT_NAME} PRIVATE ${Qt5ROOT}/include/QtCore)
target_include_directories(${PROJECT_NAME} PRIVATE ${Qt5ROOT}/include/QtWidgets)
target_include_directories(${PROJECT_NAME} PUBLIC 3rdParty/json/single_include/)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE 	Qt5::Core 
												Qt5::Widgets)

# Set the plugin as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Copy the plugin to the x64dbg plugins folder
add_custom_command(	TARGET ${PROJECT_NAME} 
					POST_BUILD
					COMMAND  ${CMAKE_COMMAND} -E "copy" "${CMAKE_CURRENT_BINARY_DIR}/Release/Spelunky2.dp64" ${X64DBG_PLUGINS_ROOT})

# Copy Spelunky2.json to the x64dbg plugins folder
add_custom_target(deploy-auxfiles)
add_custom_command(	TARGET deploy-auxfiles 
					POST_BUILD
					COMMAND  ${CMAKE_COMMAND} -E "copy" "${CMAKE_CURRENT_SOURCE_DIR}/resources/Spelunky2.json" ${X64DBG_PLUGINS_ROOT})

# Copy Spelunky2Entities.txt to the x64dbg plugins folder
add_custom_command(	TARGET deploy-auxfiles
					PRE_BUILD
					COMMAND  ${CMAKE_COMMAND} -E "copy" "${CMAKE_CURRENT_SOURCE_DIR}/resources/Spelunky2Entities.txt" ${X64DBG_PLUGINS_ROOT})

# Copy Spelunky2ParticleEmitters.txt to the x64dbg plugins folder
add_custom_command(	TARGET deploy-auxfiles
					PRE_BUILD
					COMMAND  ${CMAKE_COMMAND} -E "copy" "${CMAKE_CURRENT_SOURCE_DIR}/resources/Spelunky2ParticleEmitters.txt" ${X64DBG_PLUGINS_ROOT})

# Copy Spelunky2RoomCodes.json to the x64dbg plugins folder
add_custom_command(	TARGET deploy-auxfiles
					PRE_BUILD
					COMMAND  ${CMAKE_COMMAND} -E "copy" "${CMAKE_CURRENT_SOURCE_DIR}/resources/Spelunky2RoomCodes.json" ${X64DBG_PLUGINS_ROOT})

add_dependencies(${PROJECT_NAME} deploy-auxfiles)
