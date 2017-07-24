
//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <hpx/hpx_main.hpp>
#include <hpx/include/iostreams.hpp>

#include <hpxio/server/local_file.hpp>
#include <hpxio/base_file.hpp>

int main()
{
    hpx::io::base_file f =
            hpx::new_<hpx::io::server::local_file>(hpx::find_here());

    f.open(hpx::launch::sync, "test.txt", O_WRONLY | O_APPEND | O_CREAT);

    hpx::serialization::serialize_buffer<char> data ("test\n", 5);
    std::cout << f.write(hpx::launch::sync, data) << " bytes written in test.txt" << std::endl;

    f.close();

    // Say hello to the world!
    hpx::cout << "Hello World!\n" << hpx::flush;
    return 0;
}


