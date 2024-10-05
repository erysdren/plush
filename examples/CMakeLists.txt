add_custom_target(examples)
add_custom_target(runtime DEPENDS $<TARGET_PROPERTY:runtime,TIMESTAMPS>)
add_custom_target(assets DEPENDS $<TARGET_PROPERTY:assets,TIMESTAMPS>)
add_dependencies(examples runtime assets)

file(GLOB examples CONFIGURE_DEPENDS "*.c")
file(GLOB assets CONFIGURE_DEPENDS "*.3ds" "*.pcx")
foreach (source IN LISTS examples)
  cmake_path(GET source STEM example)
  add_executable(${example})
  target_sources(${example} PRIVATE ${source})
  target_compile_definitions(${example}
    PRIVATE
      $<$<C_COMPILER_ID:TinyCC>:SDL_DISABLE_IMMINTRIN_H>)
  target_link_libraries(${example}
    PRIVATE
      plush::plush
      SDL2::SDL2
      $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>)
  set_property(TARGET ${example} PROPERTY EXPORT_COMPILE_COMMANDS YES)
  add_dependencies(examples ${example})
  # Small shortcut to run target as a 
  add_custom_target(run.${example}
    COMMAND ${example}
    WORKING_DIRECTORY $<TARGET_PROPERTY:${example},BINARY_DIR>
    USES_TERMINAL
  )
  add_dependencies(run.${example} runtime assets ${example})
endforeach()

add_custom_command(OUTPUT assets.timestamp
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${assets} "${CMAKE_CURRENT_BINARY_DIR}"
  COMMAND "${CMAKE_COMMAND}" -E touch assets.timestamp
  DEPENDS ${assets}
  COMMENT "Copying assets to examples build directory"
  JOB_SERVER_AWARE YES
  VERBATIM
)

add_custom_command(OUTPUT SDL2.timestamp
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different $<TARGET_PROPERTY:SDL2::SDL2,IMPORTED_LOCATION> "${CMAKE_CURRENT_BINARY_DIR}"
  COMMAND "${CMAKE_COMMAND}" -E touch SDL2.timestamp
  COMMENT "Copying SDL2 to examples build directory"
  JOB_SERVER_AWARE YES
  VERBATIM)

set_property(TARGET runtime APPEND PROPERTY TIMESTAMPS "${CMAKE_CURRENT_BINARY_DIR}/SDL2.timestamp")
set_property(TARGET assets APPEND PROPERTY TIMESTAMPS "${CMAKE_CURRENT_BINARY_DIR}/assets.timestamp")
