#  Copyright (c) 2017 Alireza Kheirkhahang
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.7)

set(SOURCE_FILES posix_tests.cpp)

add_executable(posix_tests posix_tests.cpp)
target_link_libraries (posix_tests LINK_PUBLIC interposition)

