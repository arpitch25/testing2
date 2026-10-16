cmake_minimum_required(VERSION 3.10)
project(SecureMarksheetVerifier CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(verifier
    main.cpp
    MerkleTree.cpp
    MarksheetVerifier.cpp
)

target_include_directories(verifier PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
