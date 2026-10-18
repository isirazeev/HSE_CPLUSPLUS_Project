cmake_minimum_required(VERSION 3.20)
project(image_processor CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(
    image_processor
    image_processor.cpp
    factory.cpp
    image.cpp
    parse_args.cpp
)
target_compile_options(image_processor PRIVATE -std=c++20 -Wall -Wextra -Wpedantic)
