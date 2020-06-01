file(GLOB_RECURSE SOURCES "*.cpp" "*.h")
add_executable(nin64 ${SOURCES})
target_link_libraries(nin64 libnin64 OpenAL::AL)
target_include_directories(nin64 PRIVATE "${CMAKE_SOURCE_DIR}/src")
