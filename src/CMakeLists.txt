cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(polygonalmesh)
add_subdirectory(alpha_combine)
add_subdirectory(proxyutil)

add_subdirectory(test)
