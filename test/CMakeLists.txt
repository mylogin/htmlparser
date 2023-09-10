find_package(GTest)
if(GTest_FOUND)
	if(NOT TARGET GTest::gtest_main AND TARGET GTest::Main)
		# CMake <3.20
		add_library(GTest::gtest_main INTERFACE IMPORTED)
		target_link_libraries(GTest::gtest_main INTERFACE GTest::Main)
	endif()
else()
	if(POLICY CMP0135)
		cmake_policy(SET CMP0135 NEW)
	endif()
	include(FetchContent)
	# Avoids mismatch between GTest's static CRT and our dynamic.
	set(gtest_force_shared_crt ON)
	set(BUILD_GMOCK OFF)
	set(BUILD_GTEST ON)
	set(INSTALL_GTEST OFF)
	FetchContent_Declare(
		googletest
		URL https://github.com/google/googletest/archive/main.tar.gz
	)
	FetchContent_MakeAvailable(googletest)
endif()

add_executable("${PROJECT_NAME}_test_sel" selectors.cpp)
# GoogleTest requires at least C++14
target_compile_features("${PROJECT_NAME}_test_sel" PUBLIC cxx_std_14)
target_include_directories("${PROJECT_NAME}_test_sel" PRIVATE ..)
target_link_libraries("${PROJECT_NAME}_test_sel" PRIVATE ${PROJECT_NAME} GTest::gtest_main)
gtest_discover_tests("${PROJECT_NAME}_test_sel")
