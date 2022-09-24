file(GLOB examples LIST_FILES ON "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_custom_target(${PROJECT_NAME}-examples-resources ALL
  COMMAND
  ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/resources"
  "${CMAKE_CURRENT_BINARY_DIR}/resources"
  COMMENT "Copying resources directory"
  VERBATIM)

# Add an executable for each example file in examples/
foreach(example IN LISTS examples)
  get_filename_component(name "${example}" NAME_WLE)
  set(BUILD_SHARED_LIBS OFF)
  add_executable(${name})
  target_sources(${name} PRIVATE ${example})
  target_link_libraries(${name} PRIVATE ${PROJECT_NAME})
endforeach()
