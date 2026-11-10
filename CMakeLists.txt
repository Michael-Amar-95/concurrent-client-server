cmake_minimum_required(VERSION 3.17)

project(client_server C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(server server.c)
add_executable(client client.c)