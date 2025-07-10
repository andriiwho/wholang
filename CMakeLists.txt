cmake_minimum_required(VERSION 3.30)
project(WhoLang)

if(MSVC)
    add_compile_options(/EHsc)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "")
add_subdirectory(main)