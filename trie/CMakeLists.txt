cmake_minimum_required(VERSION 3.17)
project(01_trie_tree)

set(CMAKE_CXX_STANDARD 14)

add_executable(trie main.cpp trie.h trie.cpp sbc_convert.h sbc_convert.cpp)