# RPATH
set(CMAKE_MACOSX_RPATH      TRUE)
set(CMAKE_SKIP_BUILD_RPATH  FALSE)
set(CMAKE_BUILD_RPATH       "${CMAKE_BINARY_DIR}/third_party/lib")

add_subdirectory(libnin64)
add_subdirectory(NinEmu64)
