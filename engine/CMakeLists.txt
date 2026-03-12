cmake_minimum_required(VERSION 3.5)
 
# Project Name
PROJECT(engine)
 
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
 
add_executable(${PROJECT_NAME} 
    engine.cpp
    rendering.cpp
    config.cpp
    model.cpp
    input.cpp
    menu.cpp
)

# TODO: GLUI support (library/headers not found in current setup)
# add_subdirectory(glui)
# include_directories(glui/include)
# target_link_libraries(${PROJECT_NAME} PRIVATE glui)
 
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(TINYXML2 REQUIRED tinyxml2)
	include_directories(${TINYXML2_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} PRIVATE ${TINYXML2_LIBRARIES})
	target_compile_definitions(${PROJECT_NAME} PRIVATE USE_TINYXML2)
else()
	message(WARNING "PkgConfig not found — attempting to locate tinyxml2 without pkg-config.")
	# Try CMake's FindTinyXML2 or manual search
	find_package(TinyXML2 QUIET)
	if(TinyXML2_FOUND)
		include_directories(${TinyXML2_INCLUDE_DIRS})
		if(TARGET TinyXML2::TinyXML2)
			target_link_libraries(${PROJECT_NAME} PRIVATE TinyXML2::TinyXML2)
			target_compile_definitions(${PROJECT_NAME} PRIVATE USE_TINYXML2)
		elseif(TARGET tinyxml2::tinyxml2)
			target_link_libraries(${PROJECT_NAME} PRIVATE tinyxml2::tinyxml2)
			target_compile_definitions(${PROJECT_NAME} PRIVATE USE_TINYXML2)
		elseif(TinyXML2_LIBRARIES)
			target_link_libraries(${PROJECT_NAME} PRIVATE ${TinyXML2_LIBRARIES})
			target_compile_definitions(${PROJECT_NAME} PRIVATE USE_TINYXML2)
		else()
			message(WARNING "TinyXML2 found but no usable link target/variables; attempting manual find.")
		endif()
	endif()
	if(NOT TARGET TinyXML2::TinyXML2 AND NOT TARGET tinyxml2::tinyxml2 AND NOT TinyXML2_LIBRARIES)
		find_path(TINYXML2_INCLUDE_DIR tinyxml2.h HINTS /usr/include /usr/local/include)
		find_library(TINYXML2_LIBRARY NAMES tinyxml2 tinyxml2_static HINTS /usr/lib /usr/local/lib /usr/lib64)
		if(TINYXML2_INCLUDE_DIR AND TINYXML2_LIBRARY)
			include_directories(${TINYXML2_INCLUDE_DIR})
			target_link_libraries(${PROJECT_NAME} PRIVATE ${TINYXML2_LIBRARY})
			target_compile_definitions(${PROJECT_NAME} PRIVATE USE_TINYXML2)
		else()
			message(WARNING "tinyxml2 not found. XML config parsing will be disabled at runtime.")
		endif()
	endif()
endif()
if(POLICY CMP0072)
	cmake_policy(SET CMP0072 NEW)
endif()
# Prefer GLVND on modern systems to avoid FindOpenGL dev warning
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
 
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)
 
if  (WIN32)
 
	message(STATUS "Toolkits_DIR set to: " ${TOOLKITS_FOLDER})
	set(TOOLKITS_FOLDER "" CACHE PATH "Path to Toolkits folder")
	
	if (NOT EXISTS "${TOOLKITS_FOLDER}/glut/GL/glut.h" OR NOT EXISTS "${TOOLKITS_FOLDER}/glut/glut32.lib")
		message(ERROR ": GLUT not found")
	endif (NOT EXISTS "${TOOLKITS_FOLDER}/glut/GL/glut.h" OR NOT EXISTS "${TOOLKITS_FOLDER}/glut/glut32.lib")	
	
	
	include_directories(${TOOLKITS_FOLDER}/glut )
	target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES} 
										  ${TOOLKITS_FOLDER}/glut/glut32.lib)
	
	if (EXISTS "${TOOLKITS_FOLDER}/glut/glut32.dll" )
		file(COPY ${TOOLKITS_FOLDER}/glut/glut32.dll DESTINATION ${CMAKE_BINARY_DIR})
	endif(EXISTS "${TOOLKITS_FOLDER}/glut/glut32.dll" )	
	
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
	
else (WIN32) #Linux and Mac
 
	set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated")
	find_package(GLUT REQUIRED)
	include_directories(${GLUT_INCLUDE_DIR})
	link_directories(${GLUT_LIBRARY_DIRS})
	add_definitions(${GLUT_DEFINITIONS})
	
	target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
	if(NOT GLUT_FOUND)
	   message(ERROR ": GLUT not found!")
	endif(NOT GLUT_FOUND)
	
endif(WIN32)