cmake_minimum_required(VERSION 3.30)

project(re LANGUAGES CXX)

add_executable(re re.cpp)
target_compile_features(re PUBLIC cxx_std_23)
set_target_properties(re PROPERTIES CXX_EXTENSIONS OFF)

add_compile_options(-pedantic -Wall -Wextra -Weffc++ -Wconversion -Wsign-conversion -Werror)
