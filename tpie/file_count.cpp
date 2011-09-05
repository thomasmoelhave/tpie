#include <tpie/file_count.h>
#include <tpie/portability.h>
namespace tpie {

memory_size_type file_count=0;
void increment_open_file_count() {++file_count;}
void decrement_open_file_count() {--file_count;}
memory_size_type open_file_count() {return file_count;}
memory_size_type available_files() {return get_maximum_open_files() - file_count;}

}

