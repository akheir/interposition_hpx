//  Copyright (c) 2017 Alireza Kheirkhahang
//  Copyright (c) 2016 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h> 

#ifdef __cplusplus
} //extern "C" {
#endif

#include <string>
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>

#include <hpx/hpx.hpp>
#include <hpx/include/run_as.hpp>
#include <hpx/hpx_start.hpp>

#define FORWARD_DECLARE(ret,name,args) \
	ret (*__real_ ## name)args = NULL;
#define MAP(func, ret) \
	if (!(__real_ ## func)) { \
		__real_ ## func = (ret) dlsym(RTLD_NEXT, #func); \
 		if (!(__real_ ## func)) \
 		std::cout  << "Failed to link symbol: " << #func << std::endl; \
	}
#if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
    #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
    #define DLL_PUBLIC
    #define DLL_LOCAL
#endif

///////////////////////////////////////////////////////////////////////////////
// Store the command line arguments in global variables to make them available
// to the startup code.

#if defined(linux) || defined(__linux) || defined(__linux__)

int __argc = 0;
char** __argv = nullptr;

void set_argv_argv(int argc, char* argv[], char* env[])
{
    __argc = argc;
    __argv = argv;
}

__attribute__((section(".init_array")))
void (*set_global_argc_argv)(int, char*[], char*[]) = &set_argv_argv;

#elif defined(__APPLE__)

#include <crt_externs.h>

inline int get_arraylen(char** argv)
{
    int count = 0;
    if (nullptr != argv)
    {
        while(nullptr != argv[count])
            ++count;
    }
    return count;
}

int __argc = get_arraylen(*_NSGetArgv());
char** __argv = *_NSGetArgv();

#endif


///////////////////////////////////////////////////////////////////////////////
// This class demonstrates how to initialize a console instance of HPX
// (locality 0). In order to create an HPX instance which connects to a running
// HPX application two changes have to be made:
//
//  - replace hpx::runtime_mode_console with hpx::runtime_mode_connect
//  - replace hpx::finalize() with hpx::disconnect()
//
struct manage_global_runtime
{
    manage_global_runtime()
            : running_(false), rts_(nullptr)
    {
#if defined(HPX_WINDOWS)
        hpx::detail::init_winsocket();
#endif

        std::vector<std::string> const cfg = {
                // make sure hpx_main is always executed
                "hpx.run_hpx_main!=1",
                // allow for unknown command line options
                "hpx.commandline.allow_unknown!=1",
                // disable HPX' short options
                "hpx.commandline.aliasing!=0"
        };

        using hpx::util::placeholders::_1;
        using hpx::util::placeholders::_2;
        hpx::util::function_nonser<int(int, char**)> start_function =
                hpx::util::bind(&manage_global_runtime::hpx_main, this, _1, _2);

        if (!hpx::start(start_function, __argc, __argv, cfg, hpx::runtime_mode_connect))
        {
            // Something went wrong while initializing the runtime.
            // This early we can't generate any output, just bail out.
            std::abort();
        }

        // Wait for the main HPX thread (hpx_main below) to have started running
        std::unique_lock<std::mutex> lk(startup_mtx_);
        while (!running_)
            startup_cond_.wait(lk);
    }

    ~manage_global_runtime()
    {
        // notify hpx_main above to tear down the runtime
        {
            std::lock_guard<hpx::lcos::local::spinlock> lk(mtx_);
            rts_ = nullptr;               // reset pointer
        }

        cond_.notify_one();     // signal exit

        // wait for the runtime to exit
        hpx::stop();
    }

    // registration of external (to HPX) threads
    void register_thread(char const* name)
    {
        hpx::register_thread(rts_, name);
    }
    void unregister_thread()
    {
        hpx::unregister_thread(rts_);
    }

protected:
    // Main HPX thread, does nothing but wait for the application to exit
    int hpx_main(int argc, char* argv[])
    {
        // Store a pointer to the runtime here.
        rts_ = hpx::get_runtime_ptr();

        // Signal to constructor that thread has started running.
        {
            std::lock_guard<std::mutex> lk(startup_mtx_);
            running_ = true;
        }

        startup_cond_.notify_one();

        // Here other HPX specific functionality could be invoked...

        // Now, wait for destructor to be called.
        {
            std::unique_lock<hpx::lcos::local::spinlock> lk(mtx_);
            if (rts_ != nullptr)
                cond_.wait(lk);
        }

        // tell the runtime it's ok to exit
        return hpx::disconnect();
    }

private:
    hpx::lcos::local::spinlock mtx_;
    hpx::lcos::local::condition_variable_any cond_;

    std::mutex startup_mtx_;
    std::condition_variable startup_cond_;
    bool running_;

    hpx::runtime* rts_;
};

// This global object will initialize HPX in its constructor and make sure HPX
// stops running in its destructor.
manage_global_runtime init;



/*
 * File manipulation functions
 *
 * open, close, creat, read, write, close, pread, pwrite
 *
*/
FORWARD_DECLARE(int, creat, (const char *path, mode_t mode));
FORWARD_DECLARE(int, open, (const char *path, int flags, ...));
FORWARD_DECLARE(int, close, (int fd));
FORWARD_DECLARE(int, unlink, (const char *pathname));
FORWARD_DECLARE(ssize_t, write, (int fd, const void *buf, size_t count));
FORWARD_DECLARE(ssize_t, read, (int fd, void *buf, size_t count));
FORWARD_DECLARE(ssize_t, pread, (int fd, void *buf, size_t count, off_t offset));
FORWARD_DECLARE(ssize_t, pwrite, (int fd, const void *buf, size_t count, off_t offset));

__attribute__ ((noreturn)) FORWARD_DECLARE(void, exit, (int status)) ;

FORWARD_DECLARE(FILE *, fopen, (const char *path, const char *mode));

#ifdef __cplusplus
extern "C" {
#endif

DLL_PUBLIC int open(const char *path, int flags, ...) {
	MAP(open,int (*)(const char*, int, ...));

    std::cout << "Intercepting a call to open \"" << path << '\"' << std::endl;

    int ret = 0;

	if ((flags & O_CREAT) == O_CREAT) {
		va_list argf;
		va_start(argf, flags);
		mode_t mode = va_arg(argf, mode_t);
		va_end(argf);
		ret = __real_open(path, flags, mode);
	} else {
		ret = __real_open(path, flags);
	}

    std::cout << path << " opened successfully with fd: \"" << ret << '\"' << std::endl;

	return ret;
}

DLL_PUBLIC int creat(const char* path, mode_t mode) {

    std::cout << "Intercepting a call to creat \"" << path << '\"' << std::endl;

    int ret = open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);

    std::cout << path << " created successfully with fd: \"" << ret << '\"' << std::endl;

    return ret;
}

DLL_PUBLIC int close(int fd) {
    MAP(close,int (*)(int));

    std::cout << "Intercepting a call to close \"" << fd << '\"' << std::endl;

    int ret = __real_close(fd);

    return ret;
}

DLL_PUBLIC int unlink(const char *pathname) {
    MAP(unlink, int (*)(const char *));

    std::cout << "Intercepting a call to unlink \"" << pathname << '\"' << std::endl;

    int ret = 0;
    ret = __real_unlink(pathname);

    return ret;
}

DLL_PUBLIC ssize_t write(int fd, const void *buf, size_t count) {
    MAP(write,ssize_t (*)(int, const void*, size_t));

    std::cout << "Intercepting a call to write "
              << count << " bytes on \"" << fd << '\"' << std::endl;

    ssize_t ret = 0;
    ret = __real_write(fd, buf, count);

    return ret;
}

DLL_PUBLIC ssize_t read(int fd, void *buf, size_t count) {
    MAP(read, ssize_t(*)(int, void * , size_t));

    std::cout << "Intercepting a call to read "
              << count << " bytes from \"" << fd << '\"' << std::endl;

    ssize_t ret = 0;
    ret = __real_read(fd, buf, count);

    return ret;
}

DLL_PUBLIC ssize_t pread(int fd, void *buf, size_t count, off_t offset) {
    MAP(pread,ssize_t (*)(int, void*, size_t, off_t));

    std::cout << "Intercepting a call to pread "
              << count << " bytes from \"" << fd << '\"' <<
              " on position " << offset << std::endl;

    ssize_t ret = 0;
    ret = __real_pread(fd, buf, count, offset);

    return ret;
}

DLL_PUBLIC ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) {
    MAP(pwrite,ssize_t (*)(int, const void*, size_t, off_t));

    std::cout << "Intercepting a call to pwrite "
              << count << " bytes on \"" << fd << '\"' <<
              " on position " << offset << std::endl;

    ssize_t ret = 0;
    ret = __real_pwrite(fd, buf, count, offset);

    return ret;
}

DLL_PUBLIC void exit(int status) {
    MAP(exit, void (*)(int));

    std::cout << "Intercepting a call to exit with status = \"" << status << '\"' << std::endl;
    __real_exit(status);

//    return;
}

DLL_PUBLIC FILE * fopen(const char *path, const char *mode) {
	MAP(fopen, FILE *(*)(const char *, const char *));

	std::cout << "fopen: " << path << std::endl;

	return __real_fopen(path, mode);
}

#ifdef __cplusplus
} //extern "C" {
#endif