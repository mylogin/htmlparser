add_executable("${PROJECT_NAME}_example" example.cpp)
target_compile_features("${PROJECT_NAME}_example" PUBLIC cxx_std_11)
target_include_directories("${PROJECT_NAME}_example" PRIVATE ..)
target_link_libraries("${PROJECT_NAME}_example" PRIVATE ${PROJECT_NAME})