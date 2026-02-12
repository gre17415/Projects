cmake_minimum_required(VERSION 3.14)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)

project(philosophers)

add_compile_options(-Wall -Werror)

find_package(userver REQUIRED COMPONENTS core universal)

add_executable(${PROJECT_NAME} main.cpp philosopher.cpp)
add_google_tests(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC userver::universal)
target_link_libraries(${PROJECT_NAME} PUBLIC userver::utest)
