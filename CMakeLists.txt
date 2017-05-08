cmake_minimum_required(VERSION 3.7)
project(word2vec)

set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} -lm -pthread -O3 -march=native -Wall -funroll-loops -Wno-unused-result")

add_executable(word2vec word2vec.c)
add_executable(compute-accuracy compute-accuracy.c)
add_executable(distance distance.c)
add_executable(word-analogy word-analogy.c)
add_executable(word2phrase word2phrase.c)