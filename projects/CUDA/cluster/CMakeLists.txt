
project(cluster)

cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)


target_sources(zcuda PRIVATE
  cluster.cpp
  kdsearch.h
  kdsearch.cpp
)
