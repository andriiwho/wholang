file(GLOB_RECURSE main_files *.cppm LIST_DIRECTORIES ON)
add_executable(who)

target_sources(who PRIVATE FILE_SET main_files TYPE CXX_MODULES FILES ${main_files})

target_compile_features(who PUBLIC cxx_std_23)
target_include_directories(who PUBLIC ${CMAKE_CURRENT_LIST_DIR})