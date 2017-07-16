cmake_minimum_required( VERSION 2.8.4 )
project( tesseract-layout )
set( tool_EXE tesseract-layout )
include( FindPkgConfig )
pkg_check_modules( lept REQUIRED lept )
pkg_check_modules( tesseract REQUIRED tesseract )

file( GLOB tool_SRC "*.cc" )
add_executable( ${tool_EXE} ${tool_SRC} )
set_property( TARGET ${tool_EXE} PROPERTY CXX_STANDARD 11 )

include_directories( SYSTEM ${tesseract_INCLUDEDIR} )
if( EXISTS "${CMAKE_HOME_DIRECTORY}/tesseract-ocr" )
  include_directories( tesseract-ocr/ccstruct tesseract-ocr/ccutil tesseract-ocr/cutil tesseract-ocr/image tesseract-ocr/viewer )
  add_definitions( -D__TESSERACT_SOURCE__ )
  message( "-- Found tesseract source" )
endif()

add_definitions( -D__PAGEXML_LEPT__ )

string( REPLACE ";" " " CFLAGS_STR "-Wall -W ${lept_CFLAGS} ${tesseract_CFLAGS}" )
set_target_properties( ${tool_EXE} PROPERTIES COMPILE_FLAGS "${CFLAGS_STR}" )

target_link_libraries( ${tool_EXE} ${lept_LDFLAGS} ${tesseract_LDFLAGS} )

install( TARGETS ${tool_EXE} DESTINATION bin )

add_custom_target( realclean cd ${CMAKE_HOME_DIRECTORY} COMMAND rm -fr ${tool_EXE} ${tool_EXE}.exe ${tool_EXE}.dSYM CMakeFiles CMakeCache.txt cmake_install.cmake install_manifest.txt Makefile )
