cmake_minimum_required(VERSION 3.26)
project(libs C)

set(CMAKE_C_STANDARD 23)

add_library(libs tree.c jlib.h)
