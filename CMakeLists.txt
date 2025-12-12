cmake_minimum_required(VERSION 3.30)

project(re LANGUAGES CXX)

add_compile_options(-pedantic -Wall -Wextra -Weffc++ -Wconversion -Wsign-conversion -Werror)

add_library(re re.cpp re.hpp)
target_compile_features(re PUBLIC cxx_std_23)
set_target_properties(re PROPERTIES CXX_EXTENSIONS OFF)

add_executable(match match.cpp)
target_link_libraries(match PUBLIC re)
target_compile_features(match PUBLIC cxx_std_23)
set_target_properties(match PROPERTIES CXX_EXTENSIONS OFF)

add_executable(test test.cpp)
target_link_libraries(test PUBLIC re)
target_compile_features(test PUBLIC cxx_std_23)
set_target_properties(test PROPERTIES CXX_EXTENSIONS OFF)
