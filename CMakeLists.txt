cmake_minimum_required(VERSION 3.16)
project(audio VERSION 0.0.1 LANGUAGES C)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON CACHE BOOL "Prevents cmake -S. -B.")
set(CMAKE_DISABLE_SOURCE_CHANGES ON CACHE BOOL "Prevent writing files to CMAKE_SOURCE_DIR under configure")

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0075 NEW)

set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_FIND_FRAMEWORK FIRST)

include(CMakeDependentOption)
include(GenerateExportHeader)
include(FetchContent)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/dep/cmake")

# Use OpenAL-Soft's binaries for Windows
if (WIN32)
  FetchContent_Declare(openalwin
    URL "https://openal-soft.org/openal-binaries/openal-soft-1.20.1-bin.zip")
  if (NOT openalwin_POPULATED)
    FetchContent_Populate(openalwin)
  endif()
  list(APPEND CMAKE_PREFIX_PATH "${openalwin_SOURCE_DIR}")
endif()

find_package(OpenGL REQUIRED)

# OpenAL must be installed on the system, env "OPENALDIR" must be set
find_package(OpenALSoft)
find_package(OpenAL)

if(OpenALSoft_FOUND)
  if(OPENALSOFT_FOUND AND NOT TARGET OpenAL::AL)
    message("OpenALSoft FOUND ${OPENAL-SOFTFOUND}")
    add_library(OpenAL::AL UNKNOWN IMPORTED)
    set_target_properties(OpenAL::AL PROPERTIES
      IMPORTED_LOCATION "${OPENALSOFT_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${OPENALSOFT_INCLUDE_DIR}")
   endif()
   set(AL_INCLUDE ${OPENALSOFT_INCLUDE_DIR})
elseif(OpenAL_FOUND)
  if(OPENAL_FOUND AND NOT TARGET OpenAL::AL)
    message("OpenAL FOUND ${OPENAL-SOFTFOUND}")
    add_library(OpenAL::AL UNKNOWN IMPORTED)
    set_target_properties(OpenAL::AL PROPERTIES
      IMPORTED_LOCATION "${OPENAL_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}")
  endif()
   set(AL_INCLUDE ${OPENAL_INCLUDE_DIR})
else()
  message(FATAL_ERROR "Could not find OpenAL or OpenALSoft")
endif()

# Require just OpenALSoft if on apple
if(APPLE AND NOT OpenALSoft_FOUND)
  message(FATAL_ERROR "Could not find OpenALSoft")
endif()

# If to build the `examples/` folder
cmake_dependent_option(ASTERA_BUILD_EXAMPLES 
  "Build astera's examples" ON
  "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" ON)

# Build out the utility tools
cmake_dependent_option(ASTERA_BUILD_TOOLS 
  "Build astera's tools" ON
  "CMAKE_BUILD_TYPE STREQUAL Debug" ON)

# Enables output using the ASTERA_DBG macro
cmake_dependent_option(ASTERA_DEBUG_OUTPUT
  "Enable Astera's internal debug output" ON 
  "CMAKE_BUILD_TYPE STREQUAL Debug" ON)

# Enable the inclusion of pak writing in the library
cmake_dependent_option(ASTERA_PAK_WRITE
  "Include pak writing functions in asset.c/h" ON
  "CMAKE_BUILD_TYPE STREQUAL Debug;CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" ON)

# Enables ASAN & Pedantic output
option(ASTERA_DEBUG_ENGINE OFF)

# Disable OpenAL Audio Effects from being compiled in
# NOTE: Mac OSX OpenAL Effects are currently disabled by default
option(ASTERA_DISABLE_AUDIO_FX OFF)

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
    $<BUILD_INTERFACE:${AL_INCLUDE}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INCLUDE_INSTALLDIR}>)

# Add all the preprocessor definitions for headers
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<BOOL:${ASTERA_DEBUG_ENGINE}>:ASTERA_DEBUG_OUTPUT>
    $<$<BOOL:${ASTERA_DEBUG_OUTPUT}>:ASTERA_DEBUG_OUTPUT>
    $<$<PLATFORM_ID:FreeBSD>:FreeBSD>
    $<$<PLATFORM_ID:OpenBSD>:OpenBSD>
    $<$<PLATFORM_ID:NetBSD>:NetBSD>
    $<$<PLATFORM_ID:Darwin>:OSX>
  PUBLIC
    $<$<BOOL:${ASTERA_DISABLE_AUDIO_FX}>:ASTERA_AL_NO_FX>
    $<$<BOOL:${ASTERA_PAK_WRITE}>:ASTERA_PAK_WRITE>)

# Set the C Standard to C99
target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)

# Add warning output for the different compilers
target_compile_options(${PROJECT_NAME}
  PRIVATE
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
  $<$<C_COMPILER_ID:MSVC>:/W4>)

# Enables ASAN & Pedantics
if(ASTERA_DEBUG_ENGINE)
  target_compile_options(${PROJECT_NAME}
    PUBLIC
    $<$<C_COMPILER_ID:MSVC>:/fsanitize=address>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>: -g -fsanitize=address -fno-omit-frame-pointer -O2 -g>)
  
  target_link_options(${PROJECT_NAME}
    PUBLIC
    $<$<C_COMPILER_ID:MSVC>:/fsanitize=address>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-g -fsanitize=address>)
endif()

# Link against the various libraries we need
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    $<$<NOT:$<PLATFORM_ID:Windows>>:m>
  PRIVATE
  $<$<NOT:$<PLATFORM_ID:Darwin>>:OpenAL::AL>)


# set_property(TARGET ${PROJECT_NAME} PROPERTY C_VISIBILITY_PRESET "hidden")
# set_property(TARGET ${PROJECT_NAME} PROPERTY VERSION ${PROJECT_VERSION})

install(DIRECTORY include/ "${PROJECT_BINARY_DIR}/include/"
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h")

if(ASTERA_BUILD_EXAMPLES)
  if(EXISTS "${PROJECT_SOURCE_DIR}/examples")
    add_subdirectory("${PROJECT_SOURCE_DIR}/examples")
  else()
    message(WARNING "Unable to find examples directory, disabling ASTERA_BUILD_EXAMPLES")
    set(ASTERA_BUILD_EXAMPLES OFF)
  endif()
endif()

if(ASTERA_BUILD_TOOLS)
  if(EXISTS "${PROJECT_SOURCE_DIR}/tools")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools")
  else()
    message(WARNING "Unable to find tools directory, disabling ASTERA_BUILD_TOOLS")
    set(ASTERA_BUILD_TOOLS OFF)
  endif()
endif()
