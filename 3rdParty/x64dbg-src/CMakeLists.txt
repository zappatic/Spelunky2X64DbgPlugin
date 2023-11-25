add_library(x64dbg INTERFACE)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pluginsdk/*.h)
set_property(TARGET x64dbg PROPERTY INTERFACE_SOURCES ${HEADERS})
set_property(GLOBAL PROPERTY x64dbg_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(x64dbg INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(x64dbg INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

function(x64dbg_plugin target)
	# Add plugin dll
	add_library(${target} SHARED ${ARGN})
	# Change extension
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set_target_properties(${target} PROPERTIES SUFFIX ".dp64")
	else()
		set_target_properties(${target} PROPERTIES SUFFIX ".dp32")
	endif()
	# Link to the x64dbg SDK
	target_link_libraries(${target} PRIVATE x64dbg)
	# Group source files
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ARGN})
	# Group the pluginsdk sources in the project for IntelliSense 
	get_property(x64dbg_SOURCE_DIR GLOBAL PROPERTY x64dbg_SOURCE_DIR)
	get_target_property(x64dbg_SOURCES x64dbg INTERFACE_SOURCES)
	source_group(TREE ${x64dbg_SOURCE_DIR} FILES ${x64dbg_SOURCES})
endfunction()