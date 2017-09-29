add_library(StrictInt STATIC strict_int.hpp strict_int.cpp)
target_include_directories(StrictInt PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>  # <prefix>/include/mylib
)