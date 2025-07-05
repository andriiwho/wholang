file(GLOB_RECURSE main_files *.hh *.cc LIST_DIRECTORIES ON)
add_executable(who ${main_files})

target_compile_features(who PUBLIC cxx_std_23)
target_include_directories(who PUBLIC ${CMAKE_CURRENT_LIST_DIR})