#include <tpie/file_count.h>
#include <tpie/portability.h>
#ifndef _WIN32
#include <sys/resource.h>
#endif

namespace tpie {

inline tpie::memory_size_type get_maximum_open_files() {
#ifdef _WIN32
	return 512;
#else
	struct rlimit limits;
	if (getrlimit(RLIMIT_NOFILE,&limits) == -1)
	  	return 255;
	return static_cast<tpie::memory_size_type>(limits.rlim_cur);
#endif
}

memory_size_type file_count=0;
void increment_open_file_count() {++file_count;}
void decrement_open_file_count() {--file_count;}
memory_size_type open_file_count() {return file_count;}
memory_size_type available_files() {return get_maximum_open_files() - file_count;}

}

