cmake_minimum_required(VERSION 3.15.0)

project(daemon VERSION 1.0.0 DESCRIPTION "Знакомство с демонами")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

add_executable(daemon
               main.cpp
               daemon.cpp
               daemon.hpp
               )       