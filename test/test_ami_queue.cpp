// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#include <tpie/portability.h>


#include <tpie/progress_indicator_arrow.h>

#include "app_config.h"        
#include "parse_args.h"

// Get the AMI_stack definition.
#include <tpie/queue.h>

using namespace tpie;

struct options app_opts[] = {
    { 0, NULL, NULL, NULL, 0 }
};

void parse_app_opts(int /* idx */, char* /* opt_arg */) {
}

int main(int argc, char **argv)
{
    parse_args(argc, argv, app_opts, parse_app_opts);
    progress_indicator_arrow enqueue_p("Desc", test_size);
    progress_indicator_arrow dequeue_p("Desc", test_size);

    if (verbose) {
	std::cout << "test_size = " << test_size << "." << std::endl;
	std::cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << "." << std::endl;
	std::cout << "random_seed = " << random_seed << "." << std::endl;
    } else {
	std::cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed;
    }
    
    // Set the amount of main memory:
    get_memory_manager().set_limit(test_mm_size);

    ami::queue<TPIE_OS_OFFSET> queue;
    
	enqueue_p.init(test_size);
    // Push values.
    TPIE_OS_OFFSET ii;
    for (ii = 0; ii < test_size; ii++) {
		enqueue_p.step();
		queue.enqueue(ii);
    }
    enqueue_p.done();
    
    if (verbose) {
		std::cout << "Queue size = " << queue.size() << std::endl;
    }
    
    const TPIE_OS_OFFSET *jj;
    TPIE_OS_OFFSET last;
    TPIE_OS_OFFSET read = 0;

    queue.dequeue(&jj);
    last = *jj;
    
	dequeue_p.init(test_size);
    read++;
   
	dequeue_p.step();

    while(!queue.is_empty()) {

		ami::err ae = queue.dequeue(&jj);
		if(ae != ami::NO_ERROR) {
			std::cout << "Error from queue received" << std::endl;
		}
		read++;
		dequeue_p.step();
		if(*jj != ++last) {
			std::cout << std::endl << "Error in output: " << *jj << "!=" << last  << std::endl;
		}
    }
	dequeue_p.done();

    if(read != test_size) {
	std::cout << "Error: Wrong amount of elements read, got: " << read << " expected: "<<test_size << std::endl;
    }

    if(verbose) {
	std::cout << "Queue size = " << queue.size() << std::endl;
    }
    
    return 0;
}
