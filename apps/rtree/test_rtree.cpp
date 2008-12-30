#include "app_config.h"

#include <tpie/portability.h>
#include <tpie/block.h>
#include "rstartree.h"

#include <iostream>
#include <list>

const unsigned short fanOut = 10;

using namespace tpie::ami;

void insertIntoTree(rstartree<double>* r, int numberOfObjects) {

    for(int i = 0; i < numberOfObjects; ++i) {
	if (i == 89) {
	    std::cerr << numberOfObjects << std::endl;
	}
 	r->insert(rectangle<double, bid_t>(i, i*5,i*5, i*5+2, i*5+2));
	std::cerr << i << " ";
    }

    r->show_stats();
    r->check_tree();
}

void deleteFromTree(rstartree<double>* r, int numberOfObjects) {

    for(int i = 0; i < numberOfObjects; ++i) {
 	r->remove(rectangle<double, bid_t>(i, i*5,i*5, i*5+2, i*5+2));
    }

    r->show_stats();
    r->check_tree();
}

void printTree(rstartree<double>* r) {
    
    rstarnode<double>* n=NULL;
    std::list<bid_t> l;
    
    l.push_back(r->root_position());
    
    while (!l.empty()) {

	bid_t next = l.front();
	l.pop_front();

	n = r->read_node(next);

	std::cout << "-------------------------------------------" << std::endl;
	std::cout << *n << std::endl;

	n->show_children();

	if (!n->is_leaf()) {
	    for(unsigned short i=0; i<n->children(); ++i) {
		l.push_back(n->get_child(i).get_id());
	    }
	}
	delete n;
    }
}

int main(int /* argc */, char** /* argv */) {

    rstartree<double>* r = new rstartree<double>("rectangles100.rtree", fanOut);        
    r->show_stats();
    r->check_tree();
    
    std::cerr << "inserting rectangles..." << std::endl;
    insertIntoTree(r, 100);
    std::cerr << "done..." << std::endl;

    r->show_stats();
    r->check_tree();
    printTree(r);
    delete r;

    return 0;
}
