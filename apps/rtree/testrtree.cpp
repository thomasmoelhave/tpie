#include <iostream>
#include <list>
#include "app_config.h"
#include <ami_block.h>
#include "rstartree.h"

const unsigned short fanOut = 10;


void insertIntoTree(RStarTree<double>* r, int numberOfObjects) {

    for(int i = 0; i < numberOfObjects; ++i) {
	if (i == 89) {
	    cerr << numberOfObjects << endl;
	}
 	r->insert(rectangle<double, AMI_bid>(i, i*5,i*5, i*5+2, i*5+2));
	cerr << i << " ";
    }

    r->show_stats();
    r->checkTree();
}

void deleteFromTree(RStarTree<double>* r, int numberOfObjects) {

    for(int i = 0; i < numberOfObjects; ++i) {
 	r->remove(rectangle<double, AMI_bid>(i, i*5,i*5, i*5+2, i*5+2));
    }

    r->show_stats();
    r->checkTree();
}

void printTree(RStarTree<double>* r) {
    
    RStarNode<double>* n=NULL;
    list<AMI_bid> l;
    
    l.push_back(r->rootPosition());
    
    while (!l.empty()) {
	AMI_bid next = l.front();
	l.pop_front();
	n = r->readNode(next);
	cout << "-------------------------------------------"<<endl;
	cout << *n << endl;
	n->showChildren();
	if (!n->isLeaf()) {
	    for(int i=0; i<n->numberOfChildren(); ++i) {
		l.push_back(n->getChild(i).getID());
	    }
	}
	delete n;
    }
}

int main(int argc, char** argv) {

    RStarTree<double>* r = new RStarTree<double>("rectangles100.rtree", fanOut);        
    r->show_stats();
    r->checkTree();
    cerr << "inserting rectangles..." << endl;
    insertIntoTree(r, 100);
    cerr << "done..." << endl;

    r->show_stats();
    r->checkTree();
    printTree(r);
    delete r;

    return 0;
}
