// Copyright (c) 1994 Darren Vengroff
//
// File: test_ami_stack.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/15/94
//

#include <portability.h>

#include <progress_indicator_arrow.h>

#include "app_config.h"        
#include "parse_args.h"

// Get the AMI_stack definition.
#include <stack.h>

using namespace tpie;

struct options app_opts[] = {
  { 0, NULL, NULL, NULL, 0 }
};

void parse_app_opts(int idx, char *opt_arg) {
}

int main(int argc, char **argv)
{
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

    AMI_stack<TPIE_OS_OFFSET> stack;

    pi->set_percentage_range(0,test_size);
    pi->set_description("Push");

    // Push values.
    TPIE_OS_OFFSET ii;
    for (ii = test_size; ii--; ) {
      pi->step_percentage();
      stack.push(ii);
    }
    pi->done("Done");

    if (verbose) {
      cout << "Stack size = " << stack.size() << endl;
    }
    
    // Pop them all off.
    TPIE_OS_OFFSET *jj;
    TPIE_OS_OFFSET last;
    TPIE_OS_OFFSET read = 0;
    stack.pop(&jj);
    last = *jj;
    
    pi->set_description("Pop ");
    pi->reset();
    read++;
    pi->step_percentage();
    while(!stack.is_empty()) {
      AMI_err ae = stack.pop(&jj);
      if(ae != AMI_ERROR_NO_ERROR) {
        cout << "Error from stack received" << endl;
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

    if (verbose) {
        cout << "Stack size = " << stack.size() << endl;
    }
    
    return 0;
}
