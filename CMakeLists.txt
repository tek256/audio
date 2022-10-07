cmake_minimum_required(VERSION 3.16)
project(audio VERSION 0.0.1 LANGUAGES C)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON CACHE BOOL "Prevents cmake -S. -B.")
set(CMAKE_DISABLE_SOURCE_CHANGES ON CACHE BOOL "Prevent writing files to CMAKE_SOURCE_DIR under configure")

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0075 NEW)

set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_FIND_FRAMEWORK LAST)

set(CMAKE_C_EXTENSIONS ON)

include(CMakeDependentOption)
include(GenerateExportHeader)
include(FetchContent)

FetchContent_Declare(openal-windows
  DOWNLOAD_EXTRACT_TIMESTAMP YES
  URL "https://openal-soft.org/openal-binaries/openal-soft-1.20.1-bin.zip")
if (WIN32)
  FetchContent_MakeAvailable(openal-windows)
  set(ENV{OPENALDIR} "${openal-windows_SOURCE_DIR}")
endif()

find_package(OpenAL REQUIRED)

if (OPENAL_FOUND AND NOT TARGET OpenAL::OpenAL)
  add_library(OpenAL::OpenAL UNKNOWN IMPORTED)
  set_target_properties(OpenAL::OpenAL
    PROPERTIES
      IMPORTED_LOCATION "${OPENAL_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}")
endif()

if(APPLE AND NOT ${OPENAL_SOFTFOUND})
  message(FATAL_ERROR "Could not find OpenALSoft")
endif()

# If to build the `examples/` folder
cmake_dependent_option(AUDIO_BUILD_EXAMPLES 
  "Build AUDIO's examples" ON
  "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" ON)

# Enables output using the AUDIO_DBG macro
cmake_dependent_option(AUDIO_DEBUG_OUTPUT
  "Enable AUDIO's internal debug output" ON 
  "CMAKE_BUILD_TYPE STREQUAL Debug" ON)

# Enable the inclusion of pak writing in the library
cmake_dependent_option(AUDIO_PAK_WRITE
  "Include pak writing functions in asset.c/h" ON
  "CMAKE_BUILD_TYPE STREQUAL Debug;CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" ON)

# Enables ASAN & Pedantic output
option(AUDIO_DEBUG_ENGINE OFF)

# Disable OpenAL Audio Effects from being included
option(AUDIO_DISABLE_AUDIO_FX OFF)

# Get all of our source files into a list
file(GLOB sources CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.c)
add_library(${PROJECT_NAME} STATIC)

# Add our source files 
target_sources(${PROJECT_NAME} PRIVATE ${sources})

# Include headers
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INCLUDE_INSTALLDIR}>)

# Add all the preprocessor definitions for headers
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<BOOL:${AUDIO_DEBUG_ENGINE}>:AUDIO_DEBUG_OUTPUT>
    $<$<BOOL:${AUDIO_DEBUG_OUTPUT}>:AUDIO_DEBUG_OUTPUT>
    $<$<PLATFORM_ID:FreeBSD>:FreeBSD>
    $<$<PLATFORM_ID:OpenBSD>:OpenBSD>
    $<$<PLATFORM_ID:NetBSD>:NetBSD>
    $<$<PLATFORM_ID:Darwin>:OSX>
  PUBLIC
    $<$<BOOL:${AUDIO_DISABLE_AUDIO_FX}>:AUDIO_AL_NO_FX>
    $<$<BOOL:${AUDIO_PAK_WRITE}>:AUDIO_PAK_WRITE>)

# Set the C Standard to C99
target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)

# Add warning output for the different compilers
target_compile_options(${PROJECT_NAME}
  PRIVATE
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
  $<$<C_COMPILER_ID:MSVC>:/W4>)

# Enables ASAN & Pedantics
if(AUDIO_DEBUG_ENGINE)
  target_compile_options(${PROJECT_NAME}
    PUBLIC
    $<$<C_COMPILER_ID:MSVC>:/fsanitize=address>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>: -g -fsanitize=address -fno-omit-frame-pointer -O2 -g>)
  
  target_link_options(${PROJECT_NAME}
    PUBLIC
    $<$<C_COMPILER_ID:MSVC>:/fsanitize=address>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-g -fsanitize=address>)
endif()

# Link against the libraries we need
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    $<$<NOT:$<PLATFORM_ID:Windows>>:m>
    OpenAL::OpenAL)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_VISIBILITY_PRESET "hidden")
set_property(TARGET ${PROJECT_NAME} PROPERTY VERSION ${PROJECT_VERSION})

# Include our headers in projects including us
install(DIRECTORY include/ "${PROJECT_BINARY_DIR}/include/"
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h")

# Build examples if we're told to do so and the folder exists!
if(AUDIO_BUILD_EXAMPLES)
  if(EXISTS "${PROJECT_SOURCE_DIR}/examples")
    add_subdirectory("${PROJECT_SOURCE_DIR}/examples")
  else()
    message(WARNING "Unable to find examples directory, disabling AUDIO_BUILD_EXAMPLES")
    set(AUDIO_BUILD_EXAMPLES OFF)
  endif()
endif()

