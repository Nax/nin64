file(GLOB_RECURSE SOURCES "*.cpp" "*.h")
add_library(libnin64 SHARED ${SOURCES})
target_include_directories(libnin64 PUBLIC "${CMAKE_SOURCE_DIR}/include" PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_compile_definitions(libnin64 PRIVATE NIN64_DLL=1 _CRT_SECURE_NO_WARNINGS=1)

if (WIN32)
  set_target_properties(libnin64 PROPERTIES OUTPUT_NAME libnin64)
else()
  set_target_properties(libnin64 PROPERTIES OUTPUT_NAME nin64)
endif()
