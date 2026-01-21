cmake_minimum_required(VERSION 3.10)
project(TernaryGCCPlugin)

set(CMAKE_CXX_STANDARD 14)

# Find GCC plugin includes
find_path(GCC_PLUGIN_INCLUDE_DIR
    NAMES gcc-plugin.h
    PATHS /opt/homebrew/Cellar/gcc/15.2.0/lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include
          /usr/lib/gcc/x86_64-linux-gnu/9/plugin/include
          /usr/local/lib/gcc/x86_64-linux-gnu/9/plugin/include
)

if(NOT GCC_PLUGIN_INCLUDE_DIR)
    message(FATAL_ERROR "GCC plugin include directory not found")
endif()

# Find GMP
find_path(GMP_INCLUDE_DIR
    NAMES gmp.h
    PATHS /opt/homebrew/include
          /usr/include
          /usr/local/include
)

if(NOT GMP_INCLUDE_DIR)
    message(FATAL_ERROR "GMP include directory not found")
endif()

find_library(GMP_LIBRARY
    NAMES gmp
    PATHS /opt/homebrew/lib
          /usr/lib
          /usr/local/lib
)

if(NOT GMP_LIBRARY)
    message(FATAL_ERROR "GMP library not found")
endif()

include_directories(${GCC_PLUGIN_INCLUDE_DIR})
include_directories(${GMP_INCLUDE_DIR})
include_directories(include)

# Plugin
add_library(ternary_plugin SHARED src/ternary_plugin.cpp)
target_compile_definitions(ternary_plugin PRIVATE PIC)
target_link_libraries(ternary_plugin ${GMP_LIBRARY} -Wl,-undefined,dynamic_lookup)

# Runtime
add_library(ternary_runtime STATIC runtime/ternary_runtime.c)
target_include_directories(ternary_runtime PUBLIC include)

# Install
install(TARGETS ternary_plugin ternary_runtime
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include/ternary FILES_MATCHING PATTERN "*.h")