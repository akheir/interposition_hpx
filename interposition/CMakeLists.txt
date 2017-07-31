#  Copyright (c) 2017 Alireza Kheirkhahang
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

SET(SOURCE_FILES interposition.cpp)
add_library(interposition SHARED ${SOURCE_FILES})
hpx_setup_target(interposition)
target_link_libraries (interposition dl hpx_base_file)
