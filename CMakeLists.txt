cmake_minimum_required( VERSION 3.12 )

#main rendering project
project( bspvt )
set( CMAKE_CXX_STANDARD 17 )

#
if( APPLE )
    #set( OpenCV_DIR /usr/local/Cellar/opencv/4.4.0/lib/cmake/opencv4 )
    set( OpenCV_DIR /usr/local/Cellar/opencv/4.5.3_2/lib/cmake/opencv4 )
    set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-mavx2 -DGL_SILENCE_DEPRECATION -O2" )
    #set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-mavx2 -DGL_SILENCE_DEPRECATION -O2" )
    set( GLUT_INCLUDE_DIR "/usr/local/Cellar/freeglut/3.2.1_1/include/" )
    set( GLFW_LIBRARY glfw )
endif( APPLE )

if( WIN32 )
    set( opencv_DIR C:/usr/share/opencv/ )
    set( glfw3_DIR "C:/Program Files (x86)/GLFW/lib/cmake/glfw3/" )
    remove_definitions( -DLoadImage )
endif( WIN32 )

#
# package
#
find_package( glfw3 3.2 )
find_package( OpenGL )
find_package( OpenCV )

if( APPLE )
    set( EIGEN_INCLUDE_DIR /usr/local/Cellar/eigen/3.3.9/include/eigen3/ )
    include_directories( /usr/local/include ${OPENCV_INCLUDE_DIR} ${EIGEN_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/include/ )
    link_directories( /usr/local/lib/ )
    set( GLEW_LIBRARY glew )
endif( APPLE )

file( GLOB HEADER_FILES src/*.hpp src/*/*.hpp src/*/*/*.hpp src/*/*/*/*.hpp )
file( GLOB SOURCE_FILES src/main.cpp src/config.cpp src/*/*.cpp src/*/*/*.cpp )
list( FILTER SOURCE_FILES EXCLUDE REGEX "src/prt/precompute.cpp" )
list( FILTER SOURCE_FILES EXCLUDE REGEX "src/fit/fit/fitLTC.cpp" )
list( FILTER SOURCE_FILES EXCLUDE REGEX "src/ltc/ltc.cpp" )
#list( FILTER SOURCE_FILES EXCLUDE REGEX "src/prt/quadtree/renderer.cpp" )

if( WIN32 )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2" )
    set( CMAKE_SUPPRESS_REGENERATION true )
    add_definitions( -D_USE_MATH_DEFINES )
    add_definitions( /Dnot=! )
    link_directories( lib )
    include_directories( ${GLEW_INCLUDE_DIRS} ${GLFW3_INCLUDE_DIRS} )
    set( GLEW_LIBRARY glew32.lib )
    set( CMAKE_SUPPRESS_REGENERATION true )
    set( CMAKE_EXE_LINKER_FLAGS "glfw3.lib" )
endif( WIN32 )

add_executable( ${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} )
target_link_libraries( ${PROJECT_NAME} ${OPENGL_LIBRARY} ${GLEW_LIBRARY} ${OpenCV_LIBS} ${GLFW_LIBRARY} )


########################################################################################################################
# make_error_image : make error image (visualize difference between reference and rendering result)
########################################################################################################################
project( imgui_app )
include_directories( /usr/local/include/ ${OpenCV_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/include/ )
link_directories( /usr/local/lib/ ${OpenCV_LIB_DIRS} )
file( GLOB SOURCE_HEADER_FILES app/imgui/${PROJECT_NAME}/*.hpp src/imgui/imgui/*.hpp)
file( GLOB SOURCE_FILES src/imgui/imgui/*.cpp src/imgui/*.cpp )
add_executable( ${PROJECT_NAME} ${SOURCE_HEADER_FILES} ${SOURCE_FILES} src/imgui/${PROJECT_NAME}.cpp )
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} ${GLFW_LIBRARY} ${GLEW_LIBRARY} ${OPENGL_LIBRARY} )


########################################################################################################################
project( bspvt )



