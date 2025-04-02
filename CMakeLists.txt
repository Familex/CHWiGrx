# 3.11 for GLOB_RECURSE CONFIGURE_DEPENDS
# 3.20 for CXX_STANDARD 23
cmake_minimum_required (VERSION "3.20.0" FATAL_ERROR)

project (
  CHWiGrx
  LANGUAGES CXX
)

file (
  GLOB_RECURSE sources CONFIGURE_DEPENDS
  "./ChessCore/*.cpp"
  "./CHWiGrx/*.cpp"
)

# Compile winapi resource file
set (resource_file "${CMAKE_CURRENT_SOURCE_DIR}/CHWiGrx/src/winapi/CHWiGrx.rc")
set (resource_file_utf8 "${resource_file}.utf8")
set (resource_res "${CMAKE_CURRENT_BINARY_DIR}/CHWiGrx.o")
add_custom_command (
    OUTPUT ${resource_res}
    COMMAND iconv -f UTF-16LE -t UTF-8 ${resource_file} -o ${resource_file_utf8}
    COMMAND ${CMAKE_RC_COMPILER} -J rc -o ${resource_res} ${resource_file_utf8}
    COMMAND rm ${resource_file_utf8}
    DEPENDS ${resource_file}
)

add_executable (
  CHWiGrx WIN32
  ${sources}
  ${resource_res}
)

set_property (
  TARGET CHWiGrx
  PROPERTY CXX_STANDARD 23
)

target_include_directories (
  CHWiGrx PUBLIC
  "./ChessCore/src"
  "./CHWiGrx/src"
)

target_compile_definitions (
  CHWiGrx PUBLIC
  # https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt
  "_WIN32_WINNT=0x0A00" # FIXME downgrade from _WIN32_WINNT_WIN10 to _WIN32_WINNT_WIN7?
  "_UNICODE" "UNICODE"
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions (
    CHWiGrx PRIVATE
    "_DEBUG"
  )
endif ()

target_link_libraries (
  CHWiGrx PUBLIC
  msimg32 # for TransparentBlt
  comctl32 # for listview and something else
)

target_link_options (
  CHWiGrx PUBLIC
  -municode # enable wide-chars
  -mwindows # GUI mode
  -static -static-libgcc -static-libstdc++
)
