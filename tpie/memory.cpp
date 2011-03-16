#include "memory.h"
#include <iostream>
namespace tpie {

memory_manager mm;

void memory_manager::register_allocation(size_t bytes) {
  std::cout << "allocate " << bytes << std::endl;
}

void memory_manager::register_deallocation(size_t bytes) {
  std::cout << "deallocate " << bytes << std::endl;
}

  size_t memory_manager::used() const throw(){
  return 0;
}


memory_manager & get_memory_manager() {return mm;}

} //namespace tpieg
