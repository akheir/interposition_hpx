//  Copyright (c) 2017 Alireza Kheirkhahang
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <memory>


#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
} //extern "C" {
#endif

int main()
{
    constexpr char * test_file {"test_file"};
    int fd_;

    fd_ = creat(test_file, 0644);
    if (fd_ < 0)
        return 1;
    if (close(fd_) != 0)
        return 2;
    if (unlink(test_file) !=0)
        return 3;

    fd_ = open(test_file, O_WRONLY | O_APPEND |O_CREAT, 0644);
    if(fd_ < 0)
        return 4;

    if(write(fd_,"The quick brown fox jumps over the lazy dog.\n", 45) != 45)
    {
        return 5;
    }
    close(fd_);

    fd_ = ::open(test_file, O_RDONLY);
    std::string result;
    constexpr int count = 9;
    std::unique_ptr<char> sp(new char[count]);

    ssize_t len = read(fd_, sp.get(), count);
    result.assign(sp.get(), sp.get() + len);
    std::cout << result;
    return 0;
}