cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(htmlparser CXX)

add_library(${PROJECT_NAME} html.cpp html.hpp)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)

if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(HTML_BUILD_EXAMPLES "Build examples" ON)
option(HTML_BUILD_TESTS "Build tests" OFF)

if(HTML_BUILD_EXAMPLES)
	add_subdirectory(example)
endif()

if(HTML_BUILD_TESTS)
	include(CTest)
	add_subdirectory(test)
endif()