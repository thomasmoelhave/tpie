//
// File: test_ami_queue.cpp
// Created: May 10th 2006
//

#include <portability.h>

#include <versions.h>
#include <progress_indicator_arrow.h>

VERSION(test_ami_stack_cpp,"$Id: test_ami_queue.cpp,v 1.1 2006-05-10 11:39:19 aveng Exp $");

#include "app_config.h"        
#include "parse_args.h"

// Get the AMI_stack definition.
#include <ami_queue.h>

struct options app_opts[] = {
  { 0, NULL, NULL, NULL, 0 }
};

void parse_app_opts(int idx, char *opt_arg) {
}

int main(int argc, char **argv)
{
    AMI_err ae;
    progress_indicator_arrow* pi = new progress_indicator_arrow("Title","Desc",0,100,1);

    parse_args(argc, argv, app_opts, parse_app_opts);

    if (verbose) {
      cout << "test_size = " << test_size << "." << endl;
      cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << "." << endl;
      cout << "random_seed = " << random_seed << "." << endl;
    } else {
      cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed;
    }
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit(test_mm_size);

    AMI_queue<TPIE_OS_OFFSET> queue;

    pi->set_percentage_range(0,test_size);
    pi->set_description("Enqueue");

    // Push values.
    TPIE_OS_OFFSET ii;
    for (ii = 0; ii < test_size; ii++) {
      pi->step_percentage();
      queue.enqueue(ii);
    }
    pi->done("Done");

    if (verbose) {
      cout << "Queue size = " << queue.size() << endl;
    }
    
    TPIE_OS_OFFSET *jj;
    TPIE_OS_OFFSET last;
    TPIE_OS_OFFSET read = 0;
    queue.dequeue(&jj);
    last = *jj;
    
    pi->set_description("Dequeue");
    pi->reset();
    read++;
    pi->step_percentage();
    while(!queue.is_empty()) {
      AMI_err ae = queue.dequeue(&jj);
      if(ae != AMI_ERROR_NO_ERROR) {
        cout << "Error from queue received" << endl;
      }
      read++;
      pi->step_percentage();
      if(*jj != ++last) {
        cout << endl << "Error in output: " << *jj << "!=" << last  << endl;
      }
    }
    pi->done("Done");
    if(read != test_size) {
      cout << "Error: Wrong amount of elements read, got: " << read << " expected: "<<test_size << endl;
    }

    if(verbose) {
      cout << "Queue size = " << queue.size() << endl;
    }
    
    return 0;
}
