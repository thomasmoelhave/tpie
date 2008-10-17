//
//  Description:     source code for program buildtree
//  Created:         09.02.1999
//  Author:          Jan Vahrenhold
//  mail:            jan.vahrenhold@math.uni-muenster.de
//  $Id: build_rtree.cpp,v 1.2 2004-08-12 12:37:24 jan Exp $
//
//  Copyright (C) 1999-2001 by  
// 
//  Jan Vahrenhold
//  Westfaelische Wilhelms-Universitaet Muenster
//  Institut fuer Informatik
//  Einsteinstr. 62
//  D-48149 Muenster
//  GERMANY
//
#include <float.h>
// Quick hack.
#define INFINITY DBL_MAX
#define MINUSINFINITY -(DBL_MAX-1)

#include "common.h"
#include <tpie/portability.h>
#include "bulkloader.h"

int main(int argc, char** argv) {
    
    MM_manager.ignore_memory_limit();	
    //  i.e., TPIE is not in control of memory allocation and does not
    //  complain if more than  test_mm_size is allocated.
    
    // Set the main memory size. 
    MM_manager.set_memory_limit(25*1024*1024);    

    if (argc < 4) {
        cerr << "Missing command parameter." << "\n";
        cerr << "Usage: buildtree <input_stream> <fanout> <R|H>" << "\n";
    }
    else {

	RStarTree<double>* tree = NULL;
	
	cerr << "\n";
	cerr << "----------------------------------------------------------------------" << "\n";
        cerr << "\n" << "Creating ";
	if (!strcmp(argv[3], "H")) {
	    cerr << "Hilbert";
	}
	else {
	    cerr << "R*";
	}
	cerr << "-Tree (fanout=" << atol(argv[2]) << ") for " << argv[1] << "...";

	BulkLoader<double> bl(argv[1], (unsigned short)atol(argv[2]));
	err result = NO_ERROR;

	if (!strcmp(argv[3], "H")) {
	    result = bl.createHilbertRTree(&tree);
	}
	else {
	    result = bl.createRStarTree(&tree);
	}

	if (result != NO_ERROR) {
	    cerr << "Error " << hex << result;
	}

	cerr << "...done (" << tree->totalObjects() << " objects)." << "\n";
	tree->show_stats();

//
//  Uncomment the next few lines to count all nodes/objects in the tree.
//

// 	RStarNode<double>* n=NULL;
// 	list<AMI_bid> l;
// 	int nodes = 0;
// 	int objects = 0;

// 	l.push_back(tree->rootPosition());
	
// 	while (!l.empty()) {
// 	    AMI_bid next = l.front();
// 	    l.pop_front();
// 	    n = tree->readNode(next);
// 	    ++nodes;
// 	    if (!n->isLeaf()) {
// 		for(int i=0; i<n->numberOfChildren(); ++i) {
// 		    l.push_back(n->getChild(i).getID());		    
// 		}
// 	    }
// 	    else {
// 		objects += n->numberOfChildren();
// 	    }
// 	    delete n;
// 	}
// 	cout << nodes << " nodes" << "\n";
// 	cout << objects << " objects" << "\n";

	delete tree;
    }

    return 0;
}
