cmake_minimum_required(VERSION 3.10)
project(AdeptSearch VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SRC
    src/main.cpp
    src/tokenizer.cpp
    src/indexer.cpp
    src/searcher.cpp
)

add_executable(adeptsearch ${SRC})

target_include_directories(adeptsearch PRIVATE src)
