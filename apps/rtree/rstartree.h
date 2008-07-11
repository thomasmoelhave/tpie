// -*- C++ -*-
//
//  Description:     declarations for class RStarTree
//  Created:         05.11.1998
//  Author:          Jan Vahrenhold
//  mail:            jan.vahrenhold@math.uni-muenster.de
//  $Id: rstartree.h,v 1.3 2004-08-12 12:37:24 jan Exp $
//  Copyright (C) 1997-2001 by  
// 
//  Jan Vahrenhold
//  Westfaelische Wilhelms-Universitaet Muenster
//  Institut fuer Informatik
//  Einsteinstr. 62
//  D-48149 Muenster
//  GERMANY
//

//  Prevent multiple #includes.
#ifndef RSTARTREE_H
#define RSTARTREE_H

#include <portability.h>

//  Include <iostream> for output operator.
//#include <iostream>

//  Include <algorithm> for STL algorithms (min/max/sorting).
#include <algorithm>

//  Include STL templates pair, list, and vector.
#include <utility>
#include <list>
#include <vector>

//  Include TPIE AMI declarations and template AMI_stack.
#include <ami_stream.h>
#include <ami_coll.h>
#include <ami_block.h>
#include <ami_stack.h>

//  Include class rectangle.
#include "rectangle.h"

//  Include class RStarNode
#include "rstarnode.h"

//  Define a meaningfull symbolic constant used for indicating
//  that the block to be read is a new one.
const AMI_bid nextFreeBlock = 0;

//- RStarTree
template<class coord_t, class BTECOLL = BTE_COLLECTION>
class RStarTree {
//. R*-tree as described by Beckmann et al. [BKSS90].
public:
    //- RStarTree
    RStarTree();
    RStarTree(
	const char*    name,
	children_count_t fanOut);
    RStarTree(const RStarTree<coord_t, BTECOLL>& other);
    ~RStarTree();
    //. The R*-tree is defined by its fan-out, and a name for
    //. the disk file it shall be resident in. 
   
    //- operator=, operator==, operator!=
    RStarTree<coord_t, BTECOLL>& operator=(const RStarTree<coord_t, BTECOLL>& other);
    bool operator==(const RStarTree<coord_t, BTECOLL>& other);
    bool operator!=(const RStarTree<coord_t, BTECOLL>& other);
    //. Two R*-trees are identical iff they use the same storage area.
    //. It is not intended to use the assignment operator to
    //. copy the tree (i.e. the assignment operator is incomplete :-( )
 
    //- blockSize, name, fanOut, treeHeight, totalObjects, rootPosition
    unsigned short blockSize() const;
    const char* name() const;
    unsigned short fanOut() const;
    unsigned short treeHeight() const;
	TPIE_OS_OFFSET totalObjects() const;
    AMI_bid rootPosition() const;
    //. These are several methods to inquire tree metadata.

    //- readNode
    RStarNode<coord_t, BTECOLL>* readNode(AMI_bid position);
    //. The method "readNode" loads the block with ID "position" and
    //. creates a new node object from the block's contents. Note that
    //. it is the responsibility of the programmer to delete this
    //. object explicitly. 
    //. There is NO corresponding 'writeNode' method, as writing a node
    //. is be done automatically upon calling the node's destructor.
 
    //- insert
    AMI_err insert(const rectangle<coord_t, AMI_bid>& r);
    //. This method provides the standard way to insert an object
    //. into the tree. [BKSS90]
 
    //- delete
    AMI_err remove(const rectangle<coord_t, AMI_bid>& r);
    //. This method provides the standard way to delete an object
    //. from the tree. [BKSS90]

    //- query
    AMI_err query(
	const rectangle<coord_t, AMI_bid>&       r, 
	AMI_STREAM<rectangle<coord_t, AMI_bid> >* matches,
	AMI_stack<AMI_bid>*   candidates,
	bool                   bruteForce = false);
    //. This method realizes the "single shot" query algorithm for R-trees.
    //. Given a query rectangle "r" we look for all subtrees whose bounding
    //. rectangle overlaps the query rectangle and proceed recursively
    //. for each such subtree. If the current node is a leaf, all overlapping
    //. rectangles are written to the stream "matches", otherwise all
    //. overlapping bounding rectangles of subtrees are pushed on
    //. the stack "candidates". If the "bruteForce" flag is set, the complete
    //. tree is traversed.

    //- findNode
    AMI_err findNode(
	AMI_bid             nodeID,  
	AMI_stack<AMI_bid>* candidates);
    //. This method realized a depth-first search looking for a node
    //. with a given ID and prints all its contents. For a more detailed
    //. description see "findOverlappingChildren".

    //- checkTree
    void checkTree();
    //. This method traverses to complete tree and checks for each node
    //. whether the bounding box stored with a subtree is the same as the
    //. exact bounding box obtained when computing it "manually". This
    //. method is for debugging purposes only.

    //- show_stats
    void show_stats();
    //. This method shows several statistical information about the
    //. including the BTE statistics (if available).

    //- setTreeInformation
    void setTreeInformation(
	AMI_bid        root, 
	unsigned short height,
	TPIE_OS_OFFSET objects);
    //. After bulk loading by bottom-up construction, this method needs
    //. to be called to set the information about the root's block ID,
    //. the height of the tree, and the number of objects. There is
    //. no validitation of these values, however, so be sure you know
    //. what you are doing.

    //- writeTreeInfo, readTreeInfo
    void writeTreeInfo();
    bool readTreeInfo();
    //.  Write and read tree metadata so that a tree can be reused.
    //.  A read attempt returns a success flag.

protected:
  AMI_collection_single<BTECOLL> *storageArea_;      //  Pointer to the disk.
    children_count_t              fanOut_;           //  Fan-out of each node.
    children_count_t              minFanOut_;        //  Minimum fan-out.
    TPIE_OS_SIZE_T                blockSize_;        //  Block size in bytes.
    unsigned short                treeHeight_;       //  Height of the tree.
    AMI_bid                       rootPosition_;     //  ID of the root.
    TPIE_OS_OFFSET                totalObjects_;     //  Objects in the tree.
    char*                         name_;
    list<pair<rectangle<coord_t, AMI_bid>, unsigned short> > reinsertObjects_; 
    vector<bool>                           overflowOnLevel_;

    //  This method splits a given node "toSplit" into two new nodes.
    //  If necessary, a new root is created.
    //  The first node returned has the same block ID as the original node.
    pair<RStarNode<coord_t, BTECOLL>*, RStarNode<coord_t, BTECOLL>*> splitNode(RStarNode<coord_t, BTECOLL>* toSplit);

    //  This method inserts a given rectangle making sure that 
    //  the level of the node where the rectangle is placed is 'level'.
    AMI_err insertOnLevel(
	const rectangle<coord_t, AMI_bid>& r, 
	unsigned short   level);

    //  This method selects a node on level 'level' suitable to insert
    //  the given rectangle into. See "Algorithm ChooseSubtree" [BKSS90].
    RStarNode<coord_t, BTECOLL>* chooseNodeOnLevel(
	const rectangle<coord_t, AMI_bid>& r, 
	unsigned short   level);

    //  This method selects a leaf node suitable to insert
    //  the given rectangle into. This corrensponds to calling
    //  chooseNodeOnLevel(r, 0).
    RStarNode<coord_t, BTECOLL>* chooseLeaf(const rectangle<coord_t, AMI_bid>& r);

    //  This method handles underflow treatment after deletions.
    //  See "Algorithm CondenseTree" [Gutt84].
    AMI_err condenseTree(RStarNode<coord_t, BTECOLL>* n, unsigned short level);


    //  This method is called upon returning from a deletion
    //  if there are objects to be reinserted.
    AMI_err handleReinsertions();

    //  Given a (pointer to a) node, its bounding rectangle, this method
    //  reinserts the node on level 'level' making sure that any
    //  overflow is treated properly.
    AMI_err reinsert(
	RStarNode<coord_t, BTECOLL>*       n, 
	const rectangle<coord_t, AMI_bid>& r, 
	unsigned short   level);

    // This method realized a depth-first search looking for a leaf
    // containing a given object and returns this leaf's ID (or 0 if
    // the search was unsuccessful. For a more detailed
    // description see "findOverlappingChildren". Note that this 
    // algorithm keeps track of all node to be visited in internal
    // memory.
    AMI_bid findLeaf(const rectangle<coord_t, AMI_bid>& r);

    //  Install an old node 'node1' and a new node 'node2' to 'node1's 
    //  parent node. If necessary, split the parent node and propagate the
    //  split upwards. The ID of the child to be replaced in the
    //  parent node has to be passed to this method.
    void adjustTreeOnLevel(
	RStarNode<coord_t, BTECOLL>*     node1, 
	RStarNode<coord_t, BTECOLL>*     node2,
	AMI_bid       childToBeReplaced,
	unsigned short level);

private:
    //  No private members.
};

template<class coord_t, class BTECOLL>
unsigned short RStarTree<coord_t, BTECOLL>::blockSize() const {
    return blockSize_;
}

template<class coord_t, class BTECOLL>
unsigned short RStarTree<coord_t, BTECOLL>::treeHeight() const {
    return treeHeight_;
}

template<class coord_t, class BTECOLL>
TPIE_OS_OFFSET RStarTree<coord_t, BTECOLL>::totalObjects() const {
    return totalObjects_;
}

template<class coord_t, class BTECOLL>
AMI_bid RStarTree<coord_t, BTECOLL>::rootPosition() const {
    return rootPosition_;
}

template<class coord_t, class BTECOLL>
unsigned short RStarTree<coord_t, BTECOLL>::fanOut() const {
    return fanOut_;
}

template<class coord_t, class BTECOLL>
const char* RStarTree<coord_t, BTECOLL>::name() const {
//    char* namePtr = NULL;
//    storageArea_->name(&namePtr);
//    return (const char*)namePtr;
//    cerr << "name() currently not supported by BTE_collection_base." << "\n";
    return name_;
}

template<class coord_t, class BTECOLL>
RStarNode<coord_t, BTECOLL>* RStarTree<coord_t, BTECOLL>::readNode(AMI_bid position) {

    //  Try to fetch the node from the buffer.
    RStarNode<coord_t, BTECOLL>* node = new RStarNode<coord_t, BTECOLL>(storageArea_,
 				    this, 
 				    position, position, 
 				    fanOut_);
    
    assert((position==nextFreeBlock) || (node->bid() == position));

    return node;
}

template<class coord_t, class BTECOLL>
void RStarTree<coord_t, BTECOLL>::setTreeInformation(AMI_bid root, unsigned short height, TPIE_OS_OFFSET objects) {
    rootPosition_ = root;
    treeHeight_   = height;
    totalObjects_ = objects;
}

template<class coord_t, class BTECOLL>
void RStarTree<coord_t, BTECOLL>::show_stats() {
    TPIE_OS_OFFSET nodes = storageArea_->size();
    cout << "\n";
    cout << "R*-tree statistics for file: " << name() << "\n";
    cout << "root position     : " << rootPosition_ << "\n";
    cout << "fan-out           : " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(fanOut_) << "\n";
    cout << "height            : " << treeHeight_ << "\n";
    cout << "nodes             : " << nodes << "\n";
    cout << "objects           : " << totalObjects_ << "\n";
    cout << "space utilization : " << (double) (totalObjects_ + nodes -1) / (double)(nodes*fanOut_) << "\n";
    cout << "BTE statistics    : ";
    cout << "\n";
}


template<class coord_t, class BTECOLL>
RStarTree<coord_t, BTECOLL>::RStarTree() {
    storageArea_  = NULL;
    blockSize_    = 0;
    treeHeight_   = 0;
    totalObjects_ = 0;
    fanOut_       = 0;
    minFanOut_    = 0;
    rootPosition_ = 0;
}

template<class coord_t, class BTECOLL>
RStarTree<coord_t, BTECOLL>::RStarTree(const char* name, children_count_t fanOut) {

    name_ = new char[strlen(name)+1];
    strcpy(name_, name);

    storageArea_ = new AMI_collection_single<BTECOLL>(name_);
    storageArea_->persist(PERSIST_PERSISTENT);

    //  Set all attributes to the initial values.
    blockSize_     = storageArea_->block_size();
    treeHeight_    = 0;
    totalObjects_  = 0;

    //  Compute the maximal number of children that can be packed
    //  into a disk block.
    const unsigned short nodeInfoSize = 
	sizeof(fanOut) +
	sizeof(blockSize_) +
	sizeof(rootPosition_);
    const unsigned short childInfoSize = sizeof(rectangle<coord_t, AMI_bid>);

    fanOut_   = static_cast<children_count_t>((blockSize_ - nodeInfoSize) / childInfoSize);

    //  If a user-defined branching factor is given, check whether
    //  this number fits into the block and is not equal to zero.
    if ((fanOut <= fanOut_) &&
	(fanOut != 0)) {
	fanOut_ = fanOut;
    }

    minFanOut_ = (unsigned short) ((double)fanOut_ / MIN_FANOUT_FACTOR);
    
    overflowOnLevel_.push_back(false);

    //  Try to read tree information from meta file.
    readTreeInfo();

    if (!totalObjects_) {

	//  Create a "fresh" block collection.
	storageArea_->persist(PERSIST_DELETE);
	delete storageArea_;
	storageArea_  = new AMI_collection_single<BTECOLL>(name_);
	
	//  Create an empty root.
	RStarNode<coord_t, BTECOLL>* root = new RStarNode<coord_t, BTECOLL>(
	    storageArea_,
	    this, 
	    nextFreeBlock, nextFreeBlock,
	    fanOut_);
	root->setFlag(RNodeTypeLeaf | RNodeTypeRoot );
	root->setParent(root->bid());
	rootPosition_ = root->bid();

	//  Write the root to disk.
	delete root;
	root = NULL;
    }
    else {
	cerr << "Existing block collection.\n";
    }

}

template<class coord_t, class BTECOLL>
RStarTree<coord_t, BTECOLL>::RStarTree(const RStarTree<coord_t, BTECOLL>& other) {
    //  Use assignment-operator.
    (*this) = other;
}

template<class coord_t, class BTECOLL>
RStarTree<coord_t, BTECOLL>& RStarTree<coord_t, BTECOLL>::operator=(const RStarTree<coord_t, BTECOLL>& other) {
    if (this != &other) {

	cerr << "Using RStarTree::operator= is not supported.\n";
	abort();

    }
    return (*this);
}

template<class coord_t, class BTECOLL>
RStarTree<coord_t, BTECOLL>::~RStarTree() {

    writeTreeInfo();

    //  Free the storage area.
    delete storageArea_;
}

template<class coord_t, class BTECOLL>
bool RStarTree<coord_t, BTECOLL>::operator==(const RStarTree<coord_t, BTECOLL>& other) {
    return (storageArea_ == other.storageArea_);
}

template<class coord_t, class BTECOLL>
bool RStarTree<coord_t, BTECOLL>::operator!=(const RStarTree<coord_t, BTECOLL>& other) {
    return !(*this == other);
}

template<class coord_t, class BTECOLL>
AMI_err RStarTree<coord_t, BTECOLL>::query(const rectangle<coord_t, AMI_bid>& bb, AMI_STREAM<rectangle<coord_t, AMI_bid> >* matches, AMI_stack<AMI_bid>* candidates, bool bruteForce) {
     RStarNode<coord_t, BTECOLL>* n = NULL;
     AMI_bid*  current = NULL;
     AMI_err    result = AMI_ERROR_NO_ERROR;
     off_t      candidatesCounter = 0;
     off_t      leafCounter = 0;

     //  Initialize the process by pushing the root's ID onto the stack.
     candidates->push(rootPosition_);
     ++candidatesCounter;

     //  Explore the tree by (restricted) depth-first traversal.
     while (candidatesCounter > 0) {

	 //  Pop the topmost node ID from the stack.
	 result = candidates->pop(&current);
	 --candidatesCounter;
	 
	 if (result != AMI_ERROR_NO_ERROR) {
	     break;
	 }
	 
	 //  Read the current node and find eventually overlapping children.
	 n = readNode(*current);    
	 n->query(bb, candidates, matches, candidatesCounter, leafCounter, bruteForce);
	 delete n;
	 n = NULL;
     }
     
     if (bruteForce) {
	 cout << leafCounter << " objects." << "\n";
     }

     return result;
}

template<class coord_t, class BTECOLL>
AMI_err RStarTree<coord_t, BTECOLL>::findNode(AMI_bid nodeID,  AMI_stack<AMI_bid>* candidates) {
     RStarNode<coord_t, BTECOLL>* n = NULL;
     AMI_bid*  current = NULL;
     AMI_err    result = AMI_ERROR_NO_ERROR;
     off_t      candidatesCounter = 0;

     cout << "Looking for : " << nodeID << "\n";

     //  Initialize the search by pushing the root's ID onto the stack.
     candidates->push(rootPosition_);
     ++candidatesCounter;

     //  Explore the tree by depth-first traversal.
     while (candidatesCounter > 0) {

	 //  Pop the topmost node ID from the stack.
	 result = candidates->pop(&current);
	 --candidatesCounter;
	 
	 if (result != AMI_ERROR_NO_ERROR) {
	     break;
	 }
	 
	 //  Read the current node and find the node in question.
	 n = readNode(*current);    
	 n->findNode(nodeID, candidates, candidatesCounter);
	 delete n;
	 n = NULL;
     }

     return result;
}

template<class coord_t, class BTECOLL>
void RStarTree<coord_t, BTECOLL>::checkTree() {
    list<pair<AMI_bid, rectangle<coord_t, AMI_bid> > > l;
    rectangle<coord_t, AMI_bid>                        checkNode;
    RStarNode<coord_t, BTECOLL>*                       n = NULL;
    TPIE_OS_OFFSET                                     objectCounter = 0;

    cerr << "Checking tree " << name() << "\n";

    //  Initialize the process by pushing the root's ID into the queue.
    l.push_back(pair<AMI_bid, rectangle<coord_t, AMI_bid> >(rootPosition_, checkNode));

    //  Explore the tree by breadth-first traversal.
    while (!l.empty()) {

	//  Remove the frontmost element from the queue.
	n         = readNode(l.front().first);
	checkNode = l.front().second;
	l.pop_front();

	//  Check all children of the current node.
	n->checkChildren(checkNode, l, objectCounter);
	delete n;
	n = NULL;
    }

    cerr << objectCounter << " objects stored in this tree." << "\n";
}

template<class coord_t, class BTECOLL>
RStarNode<coord_t, BTECOLL>* RStarTree<coord_t, BTECOLL>::chooseLeaf(const rectangle<coord_t, AMI_bid>& r) { 
    return chooseNodeOnLevel(r, 0);
}

template<class coord_t, class BTECOLL>
RStarNode<coord_t, BTECOLL>* RStarTree<coord_t, BTECOLL>::chooseNodeOnLevel(const rectangle<coord_t, AMI_bid>& r, unsigned short level) {
    RStarNode<coord_t, BTECOLL>*     N  = readNode(rootPosition_);
    AMI_bid       ID;
    unsigned short lookingAtLevel = treeHeight();
    
    //  Proceed on a root-to-leaf path.
    while(lookingAtLevel > level) {

	--lookingAtLevel;

	//  Select the child whose bounding rectangle need least
	//  enlargement to include "r".
	//  Adjust the bounding rectangle to include "r".
	ID = N->getChild(N->route(r)).getID();

	//  Write the node to disk.
	delete N;
	N = NULL;
	//  Load the child node selected above.
	N = readNode(ID);
    }

    return N;
}

template<class coord_t, class BTECOLL>
AMI_bid RStarTree<coord_t, BTECOLL>::findLeaf(const rectangle<coord_t, AMI_bid>& r) {
     RStarNode<coord_t, BTECOLL>*     n = NULL;
     AMI_bid       current = (AMI_bid) 0;     
     list<AMI_bid> candidateList;

     //  Initialize the process by pushing the root's ID onto the stack.
     candidateList.push_back(rootPosition_);

     //  Explore the tree by (restricted) depth-first traversal.
     while ((candidateList.size() > 0) && (current == 0)) {

	 //  Pop the topmost node ID from the stack.
	 current = candidateList.front();
	 candidateList.pop_front();
	 
	 //  Read the current node and find eventually overlapping children.
	 n = readNode(current);    
	 current = n->findLeaf(r, &candidateList);
	 delete n;
	 n = NULL;
     }
     
     return current;
}

template<class coord_t, class BTECOLL>
AMI_err RStarTree<coord_t, BTECOLL>::condenseTree(RStarNode<coord_t, BTECOLL>* n, unsigned short level) {

    //  The return value is always AMI_ERROR_NO_ERROR.
    //  You might want to check for I/O errors every time
    //  an I/O has been performed.
    AMI_err result = AMI_ERROR_NO_ERROR;
    
    while ((!n->isRoot()) && (result == AMI_ERROR_NO_ERROR)) {
	RStarNode<coord_t, BTECOLL>*     parent = readNode(n->getParent());
	unsigned short sonID = parent->findChild(n->bid());
	unsigned short counter = 0;	

	//  Check whether there is an underflow in node n.
	if (n->numberOfChildren() < minFanOut_) {
	    
	    //  Move all children of node n to the list of objects to
	    //  be reinserted.
	    for (counter = 0; counter < n->numberOfChildren(); ++counter) {
		reinsertObjects_.push_back(pair<rectangle<coord_t, AMI_bid>, unsigned short>(n->getChild(counter), level));
	    }
	    
	    //  Remove n from its parent.
	    parent->removeChild(sonID);
	    
	    //  Delete the (block containing the) old node.
	    n->persist(PERSIST_DELETE);
	    delete n;
	    n = NULL;

	}
	else {
	    
	    //  Compute the exact boundingbox of n.
	    rectangle<coord_t, AMI_bid> cover = n->getChild(0);
	    for (counter = 1; counter < n->numberOfChildren(); ++counter) {
		cover.extend(n->getChild(counter));
	    }
	    cover.setID(n->bid());
	    
	    //  Update n's bounding box stored in n's parent.
	    parent->setChild(sonID, cover);

	    //  Write the node to disk.
	    delete n;
	    n = NULL;

	}
	
	n = parent;

	++level;
    }

    //  Uncomment the following line, if you don't trust
    //  the algorithm and want to check the tree every time
    //  the height decreases.
//    unsigned short th = treeHeight();

    if (result == AMI_ERROR_NO_ERROR) {

	//  Node n is the root of the tree.
	if (!n->isLeaf()) {
	    if (n->numberOfChildren() == 1) {
		
		//  Check whether there is an underflow in node n.
		AMI_bid newRootPosition = n->getChild(0).getID();
		
		//  If the only child of the root is a node make it
		//  the new root.
		if (newRootPosition > 0) {
		    
		    rootPosition_ = newRootPosition;
		    
		    //  Delete the old node.
		    n->persist(PERSIST_DELETE);
		    delete n;
		    n = readNode(rootPosition_);
		    n->setFlag(n->getFlag() | RNodeTypeRoot);
		    n->setParent(n->bid());
		    
		    //  Decrease the height of the tree by one.
		    --treeHeight_;
		}
	    //  Otherwise nothing is to be done.
	    }
	}
    }

    //  Write the node to disk.
    delete n;
    n = NULL;

    //  Uncomment the following three lines, if you don't trust
    //  the algorithm and want to check the tree every time
    //  the height decreases.
//    if (treeHeight() != th) {
//	checkTree();
//    }

    return result;
}

template<class coord_t, class BTECOLL>
AMI_err RStarTree<coord_t, BTECOLL>::remove(const rectangle<coord_t, AMI_bid>& r) {
    AMI_err   result = AMI_ERROR_NO_ERROR;
    AMI_bid  nodeID = findLeaf(r);

    if (nodeID > 0) {
	//  If the node contained the objects proceed as
	//  described in Guttman's delete-algorithm.
	RStarNode<coord_t, BTECOLL>* n = readNode(nodeID);

	//  Remove the object to be deleted.
	n->removeChild(r);

	--totalObjects_;

	//  Node n will be deleted within this method.
	result = condenseTree(n, 0);  
	
	if (result == AMI_ERROR_NO_ERROR) {

	    //  Reinsert orphaned entries.
	    result = handleReinsertions();
	}

	if (result != AMI_ERROR_NO_ERROR) {
	    cerr << "After deletion of " << r.getID() << "\n";
	    cerr << "AMI_ERROR " << result << " occurred." << "\n";
	}

    }
    else {
	cerr << "Object to be deleted (ID=" << r.getID() << ") not found." << "\n";
	checkTree();
	result = AMI_ERROR_END_OF_STREAM;
    }

    return result;    
}

template<class coord_t, class BTECOLL>
AMI_err RStarTree<coord_t, BTECOLL>::handleReinsertions() {
    AMI_err        result = AMI_ERROR_NO_ERROR;
    rectangle<coord_t, AMI_bid>      r;
    unsigned short level = 0;

    while ((result == AMI_ERROR_NO_ERROR) && (!reinsertObjects_.empty())) { 

	r     = (*reinsertObjects_.begin()).first;
	level = (*reinsertObjects_.begin()).second;

	reinsertObjects_.pop_front();

	result = insertOnLevel(r, level);
	
    }

    return result;
}

template<class coord_t, class BTECOLL>
AMI_err RStarTree<coord_t, BTECOLL>::insert(const rectangle<coord_t, AMI_bid>& r) {

    AMI_err result = AMI_ERROR_NO_ERROR;

    //  Initialize the overflow array
    unsigned short counter;
    for (counter = 0; counter < overflowOnLevel_.size(); ++counter) {
	overflowOnLevel_[counter] = false;
    }

    ++totalObjects_;

    result = insertOnLevel(r, 0);

    if (result == AMI_ERROR_NO_ERROR) {
	handleReinsertions();
    }
    // [tavi] added return statement.
    return result;
}

template<class coord_t, class BTECOLL>
AMI_err RStarTree<coord_t, BTECOLL>::insertOnLevel(const rectangle<coord_t, AMI_bid>& r, unsigned short level) {

    //  The return value is always AMI_ERROR_NO_ERROR.
    //  You might want to check for I/O errors every time
    //  an I/O has been performed.
    AMI_err    result        = AMI_ERROR_NO_ERROR;

    RStarNode<coord_t, BTECOLL>* insertionNode = chooseNodeOnLevel(r, level);

    if (insertionNode->isFull()) {

	//  "If the level is not the root level and this is the first
	//   call of OverflowTreatment in the given level during the
        //   insertion of one data rectangle then invoke reinsert
        //   else invoke split." [BKS93], 327.
	
	if ((overflowOnLevel_[level]) || (insertionNode->isRoot())) {

	    insertionNode->addChild(r);
	    
	    //  If we are inserting on other than leaf-level, the 
	    //  (re-)inserted node needs to know its new parent.
	    if (!insertionNode->isLeaf()) {
		RStarNode<coord_t, BTECOLL>* tempNode = readNode(r.getID());
		tempNode->setParent(insertionNode->bid());

		//  Write the node to disk.
		delete tempNode;
		tempNode = NULL;
	    }

	    //  If the insertion node is full, split it to obtain two new 
	    //  node. (The special case of splitting the root is handled by 
	    //  the split-method.)

	    pair<RStarNode<coord_t, BTECOLL>*, RStarNode<coord_t, BTECOLL>*> nodeTupel = splitNode(insertionNode);

	    AMI_bid childToBeReplaced = insertionNode->bid();

	    //  Delete the node that has been split.
	    insertionNode->persist(PERSIST_DELETE);
	    delete insertionNode;
	    insertionNode = NULL;
	    
	    //  Propagate the split upwards passing the new nodes.
	    if (nodeTupel.first) {
		adjustTreeOnLevel(nodeTupel.first, nodeTupel.second, childToBeReplaced, level+1);
	    }
	}
	else {
	    

	    //  Mark the level.
	    overflowOnLevel_[level] = true;

	    //  Force reinsert.
	    reinsert(insertionNode, r, level);

	    //  Delete the old node (it has been substituted in 'reinsert').
	    insertionNode->persist(PERSIST_DELETE);
	    delete insertionNode;
	    insertionNode = NULL;

	}
    }
    else {

	//  If the insertion node still has enough room, insert the
	//  given bounding box...
	insertionNode->addChild(r);

	if (!insertionNode->isLeaf()) {
	    //  Notify the node of its new parent, i.e., "insertionNode".
	    RStarNode<coord_t, BTECOLL>* tempNode = readNode(r.getID());
	    tempNode->setParent(insertionNode->bid());

	    //  Write the node to disk.
	    delete tempNode;
	    tempNode = NULL;
	    insertionNode->updateChildrenParent();
	}

	//  ...and save the touched node.
	delete insertionNode;
	insertionNode = NULL;
    }

    return result;
}

template<class coord_t>
struct sortByCenterDistance {
    bool operator()(const pair<unsigned short, coord_t>& t1, const pair<unsigned short, coord_t>& t2) {
	return (t1. second < t2.second);
    }
};

template<class coord_t, class BTECOLL>
AMI_err RStarTree<coord_t, BTECOLL>::reinsert(RStarNode<coord_t, BTECOLL>* n, const rectangle<coord_t, AMI_bid>& r, unsigned short level) {
    rectangle<coord_t, AMI_bid>                              cover = n->getChild(0);
    vector<pair<unsigned short, coord_t> > sortVector;
    unsigned short                         counter;

    AMI_err result = AMI_ERROR_NO_ERROR;

    for (counter=1; counter < n->numberOfChildren(); ++counter) {
	cover.extend(n->getChild(counter));
    }

    coord_t midX = (cover.left()+cover.right()) / 2.0;
    coord_t midY = (cover.lower()+cover.upper()) / 2.0;

    //  Copy all children (pointers to them) along with their MBR's bounding
    //  rectangle's distance to the centerpoint into an array.
    for(counter=0; counter < n->numberOfChildren(); ++counter) {
	cover = n->getChild(counter);
	sortVector.push_back(pair<unsigned short, coord_t>(
	    counter, 
	    (((cover.left()+cover.right()) / 2.0) - midX)*
	    (((cover.left()+cover.right()) / 2.0) - midX) +
	    (((cover.lower()+cover.upper()) / 2.0) - midY)*
	    (((cover.lower()+cover.upper()) / 2.0) - midY)));
    }
    sortVector.push_back(pair<unsigned short, coord_t>(
	counter, 
	(((r.left()+r.right()) / 2.0) - midX)*
	(((r.left()+r.right()) / 2.0) - midX) +
	(((r.lower()+r.upper()) / 2.0) - midY)*
	(((r.lower()+r.upper()) / 2.0) - midY)));    

    //  Sort by increasing distance to n's centerpoint.
    sort(sortVector.begin(), sortVector.end(), sortByCenterDistance<coord_t>());

    //  Create a new node that recieves the first 70% of the (sorted)
    //  sequence of children.
    //  "The experiments have shown that p=30% of M for leaf nodes as
    //   well as for non-leaf nodes yields the best performance" [BKS93], 327.

    RStarNode<coord_t, BTECOLL>* newNode = new RStarNode<coord_t, BTECOLL>(storageArea_,
				       this, 
				       n->getParent(), 
				       nextFreeBlock,
				       fanOut_);
    newNode->setFlag(n->getFlag());

    typename std::vector<pair<unsigned short, coord_t> >::iterator vi = sortVector.begin();

    //  Copy the entries into the new node.
    for(counter = 0; counter < (n->numberOfChildren() * 70) / 100; ++counter, ++vi) {
	//  Check to see whether we are moving around entries
	//  from the old node or are inserting rectangle r.
	if ((*vi).first == n->numberOfChildren()) {
	    newNode->addChild(r);
	    if (!newNode->isLeaf()) {
		//  Notify each child of its new parent.
		RStarNode<coord_t, BTECOLL>* tempNode = readNode(r.getID());
		tempNode->setParent(newNode->bid());

		//  Write the node to disk.
		delete tempNode;
		tempNode = NULL;
	    }
	}
	else {
	    newNode->addChild(n->getChild((*vi).first));	
	    if (!newNode->isLeaf()) {
		//  Notify each child of its new parent.
		RStarNode<coord_t, BTECOLL>* tempNode = readNode(n->getChild((*vi).first).getID());
		tempNode->setParent(newNode->bid());

		//  Write the node to disk.
		delete tempNode;
		tempNode = NULL;
	    }
	}
    }    

    //  Delete the old node.
    n->persist(PERSIST_DELETE);

    RStarNode<coord_t, BTECOLL>* currentNode = newNode;
    AMI_bid   lastID      = newNode->bid();
    AMI_bid   newParent   = newNode->getParent();
    //  Adjust bounding rectangles along the path to the root.
    //  Note: At this point the first node in question cannot be the root.
    do {
	
	//  Compute the parent's new bounding rectangle.
	cover = currentNode->getChild(0);
	cover.setID(currentNode->bid());
	for (counter = 1; counter < currentNode->numberOfChildren(); ++counter) {
	    cover.extend(currentNode->getChild(counter));
	}	    

	newParent = currentNode->getParent();

	//  Save the AMI_bid necessary for the next iteration.
	if (currentNode == newNode) {
	    lastID    = n->bid();  // We are replacing "n" by "newNode".
	    delete newNode;
	    newNode     = NULL;
	    currentNode = NULL;
	}
	else {
	    lastID    = currentNode->bid();
	}

	//  Delete the old parent node and load the parent's parent.
	delete currentNode;
	currentNode = NULL;
       
	currentNode = readNode(newParent);

	//  Update the bounding rectangle of the last node in its
	//  parents child array.
	currentNode->setChild(currentNode->findChild(lastID), cover);

    } while (!currentNode->isRoot()); 

    delete currentNode;
    currentNode = NULL;

    //  Reinsert the remaining 30% of the entries.
    while ((result == AMI_ERROR_NO_ERROR) && (vi != sortVector.end())) {

	if ((*vi).first == n->numberOfChildren()) {
	    reinsertObjects_.push_back(pair<rectangle<coord_t, AMI_bid>, unsigned short>(r, level));
	}
	else {
	    reinsertObjects_.push_back(pair<rectangle<coord_t, AMI_bid>, unsigned short>(n->getChild((*vi).first), level));
	}
	++vi;
    }

    return result;

    //  Don't forget to delete n after returning from this method!
}


template<class coord_t, class BTECOLL>
void RStarTree<coord_t, BTECOLL>::adjustTreeOnLevel(RStarNode<coord_t, BTECOLL>* node1, RStarNode<coord_t, BTECOLL>* node2, AMI_bid childToBeReplaced, unsigned short level) {

    RStarNode<coord_t, BTECOLL>*     parent = NULL;
    unsigned short index;

    parent = readNode(node1->getParent());
    
    //  Update the covering rectangle of the actual node in
    //  its parent node.
    index  = parent->findChild(childToBeReplaced);     
    parent->setChild(index, node1->getCoveringRectangle());
    
    if (parent->isFull()) {
	
	if ((overflowOnLevel_[level]) || (parent->isRoot())) {

	    parent->addChild(node2->getCoveringRectangle());
	    node2->setParent(parent->bid());
	    
	    assert(parent->isParent(node2));

	    //  Split the parent node.
	    pair<RStarNode<coord_t, BTECOLL>*, RStarNode<coord_t, BTECOLL>*> nodeTupel = splitNode(parent);

	    AMI_bid childToBeReplaced = parent->bid();

	    //  Delete the node that has been split.
	    parent->persist(PERSIST_DELETE);
	    delete parent;
	    parent = NULL;
	    
	    RStarNode<coord_t, BTECOLL>* newNode1 = nodeTupel.first;
	    RStarNode<coord_t, BTECOLL>* newNode2 = nodeTupel.second;
	    
	    if (newNode1) {
		//  Check which node is the actual parent of node1.
		if (newNode1->isParent(node1)) {
		    node1->setParent(newNode1->bid());
		}
		else {
		    node1->setParent(newNode2->bid());
		}
		//  Check which node is the actual parent of node2.
		if (newNode1->isParent(node2)) {
		    node2->setParent(newNode1->bid());
		}
		else {
		    node2->setParent(newNode2->bid());
		}		
	    }
	    
	    //  Write the nodes to disk.
	    delete node1;
	    delete node2;
	    node1 = NULL;
	    node2 = NULL;
	    
	    //  The touched new nodes do not have to be saved at this point.
	    //  This is done after the next modifications in 'adjustTree'.
	    
	    //  Propagate the split upwards passing the new nodes.
	    if (newNode1) {
		adjustTreeOnLevel(newNode1, newNode2, childToBeReplaced, level + 1);
	    }
	} 
	else {
	    //  Mark the level.
	    overflowOnLevel_[level] = true;

	    rectangle<coord_t, AMI_bid> r = node2->getCoveringRectangle();
	    r.setID(node2->bid());

	    //  Write the nodes to disk.
	    delete node1;
	    delete node2;
	    node1 = NULL;
	    node2 = NULL;

	    //  Force reinsert.
	    reinsert(parent, r, level);

	    //  Delete the old node (it has been substituted in 'reinsert').
	    parent->persist(PERSIST_DELETE);
	    delete parent;
	    parent = NULL;

	}
    }
    else {
	
	node2->setParent(parent->bid());
	parent->addChild(node2->getCoveringRectangle());
	
	assert(parent->isParent(node1));
	assert(parent->isParent(node2));

	//  Write all nodes to disk.
	delete parent;
	delete node1;
	delete node2;
	parent = NULL;
	node1 = NULL;
	node2 = NULL;
    }	

    handleReinsertions();
}

//  R*-tree split heuristic from [BKSS90].
template<class coord_t, class BTECOLL>
pair<RStarNode<coord_t, BTECOLL>*, RStarNode<coord_t, BTECOLL>*>
RStarTree<coord_t, BTECOLL>::splitNode(RStarNode<coord_t, BTECOLL>* toSplit) {
    RStarNode<coord_t, BTECOLL>* newNode1 = new RStarNode<coord_t, BTECOLL>(storageArea_,
					this, 
					toSplit->getParent(), 
					nextFreeBlock, 
					fanOut_);
    RStarNode<coord_t, BTECOLL>* newNode2 = new RStarNode<coord_t, BTECOLL>(storageArea_,
					this, 
					toSplit->getParent(), 
					nextFreeBlock,
					fanOut_);
    RStarNode<coord_t, BTECOLL>*     newRoot         = NULL;
    AMI_bid       newRootPosition = 0;
    unsigned short counter         = 0;

    //  Determine split axis and distribution.
    pair<vector<rectangle<coord_t, AMI_bid> >*, unsigned short> seeds = toSplit->chooseSplitAxisAndIndex();

    unsigned short firstGroupNumber = (unsigned short)(fanOut_/ MIN_FANOUT_FACTOR) + seeds.second;

    rectangle<coord_t, AMI_bid> b1 = (*(seeds.first))[0];
    rectangle<coord_t, AMI_bid> b2 = (*(seeds.first))[firstGroupNumber];

    for(counter = 0; counter < firstGroupNumber; ++counter) {
	newNode1->addChild((*(seeds.first))[counter]);
	b1.extend((*(seeds.first))[counter]);
    }

    for(counter = firstGroupNumber; counter < (seeds.first)->size(); ++counter) {
	newNode2->addChild((*(seeds.first))[counter]);
	b2.extend((*(seeds.first))[counter]);
    }

    delete seeds.first;
    seeds.first = NULL;

    b1.setID(newNode1->bid());
    b2.setID(newNode2->bid());
	
    //  Check whether the old node was the root. If so, create a new root.
    if (toSplit->isRoot()) {


	//  Create a new root whose child is the first node.
	newRoot       = new RStarNode<coord_t, BTECOLL>(storageArea_,
				      this, 
				      nextFreeBlock, 
				      nextFreeBlock, 
				      fanOut_);
	newRoot->setFlag(RNodeTypeRoot);
	newRoot->setParent(newRoot->bid());
	newRootPosition = newRoot->bid();
	rootPosition_   = newRoot->bid();

	++treeHeight_;

	//  Extend overflow array
	overflowOnLevel_.push_back(false);

	newRoot->addChild(b1);
	newRoot->addChild(b2);
	newNode1->setParent(newRootPosition);
	newNode2->setParent(newRootPosition);

	assert(newRoot->isParent(newNode1));
	assert(newRoot->isParent(newNode2));

	//  Write the new root to disk.
	delete newRoot;
	newRoot = NULL;

	//  If the old node was a leaf node, the new nodes are leaf nodes, too.
	//  Otherwise the new ones are internal nodes.
	if (toSplit->isLeaf()) {
	    newNode1->setFlag(RNodeTypeLeaf);
	    newNode2->setFlag(RNodeTypeLeaf);
	}
	else {
	    newNode1->setFlag(RNodeTypeInternal);
	    newNode2->setFlag(RNodeTypeInternal);
	}
    } 
    else {

	//  The new nodes are of the same kind as the old node.
	newNode1->setFlag(toSplit->getFlag());
	newNode2->setFlag(toSplit->getFlag());
    }

    //  Set the correct covering rectangles.
    newNode1->setCoveringRectangle(b1);
    newNode2->setCoveringRectangle(b2);

    //  The children of the new nodes refer to the ID of the old node.
    //  Therefore these pointers have to be updated.
    newNode1->updateChildrenParent();
    newNode2->updateChildrenParent();

    if (newRootPosition) {
	delete newNode1;
	delete newNode2;
	newNode1 = NULL;
	newNode2 = NULL;
    }

    //  Return pointers to the new nodes.
    return pair<RStarNode<coord_t, BTECOLL>*, RStarNode<coord_t, BTECOLL>*>(newNode1,newNode2);

}

// Try to get tree information using the ".info" meta file.
template<class coord_t, class BTECOLL>
bool RStarTree<coord_t, BTECOLL>::readTreeInfo() {

    bool returnValue;

    //  Add suffix ".info" to the input file name.
    char *treeinfo_filename = new char[strlen(name_)+5];
    strcpy(treeinfo_filename, name_);
    strcat(treeinfo_filename, ".info");
    
    cerr << "Looking for info file " << treeinfo_filename << "\n";
    
    //  Try to read some info.
    ifstream *treeinfo_file_stream = new ifstream(treeinfo_filename);
    if (!(*treeinfo_file_stream)) {
	rootPosition_ = 0;
	treeHeight_   = 0;
	totalObjects_ = 0;
	cerr << "No tree information found." << "\n";
	returnValue = false;
    }
    else {
	treeinfo_file_stream->read((char *) &rootPosition_, sizeof(AMI_bid));
	treeinfo_file_stream->read((char *) &treeHeight_, sizeof(unsigned short));
	treeinfo_file_stream->read((char *) &totalObjects_, sizeof(off_t));
	treeinfo_file_stream->read((char *) &fanOut_, sizeof(off_t));
	returnValue = true;
    }
    
    delete treeinfo_file_stream;
    delete [] treeinfo_filename;

    return returnValue;

}

template<class coord_t, class BTECOLL>
void RStarTree<coord_t, BTECOLL>::writeTreeInfo() {

    //  Add suffix ".info" to the input file name.
    char *treeinfo_filename = new char[strlen(name_)+5];
    strcpy(treeinfo_filename, name_);
    strcat(treeinfo_filename, ".info");
    //  Write some info.
    ofstream *treeinfo_file_stream = new ofstream(treeinfo_filename);
    treeinfo_file_stream->write((char *) &rootPosition_, sizeof(AMI_bid));
    treeinfo_file_stream->write((char *) &treeHeight_, sizeof(unsigned short));
    treeinfo_file_stream->write((char *) &totalObjects_, sizeof(off_t));  
    treeinfo_file_stream->write((char *) &fanOut_, sizeof(unsigned short));  
    
    delete treeinfo_file_stream;
    delete [] treeinfo_filename;
}



#endif

//  References:
//  
//  @inproceedings{BKSS90
//  , author = 	     "Norbert Beckmann and Hans-Peter Kriegel and Ralf
//                    Schneider and Bernhard Seeger" 
//  , title = 	     "The {R$^\ast$}-tree: {A}n Efficient and Robust
//                    Access Method for Points and Rectangles"
//  , pages = 	     "322--331"
//  , booktitle =    "Proceedings of the 1990 {ACM} {SIGMOD} International
//                    Conference on Management of Data"
//  , year = 	     1990
//  , editor = 	     "Hector Garcia-Molina and H. V. Jagadish"
//  , volume = 	     "19.2"
//  , series = 	     "{SIGMOD} Record"
//  , month = 	     "June"
//  , publisher =    "{ACM} Press"
//  }
//  
//  @inproceedings{Gutt84
//  , author = 	     "Antonin Guttman"
//  , title = 	     "{R}-trees: {A} Dynamic Index Structure for Spatial 
//                    Searching"
//  , pages = 	     "47--57" 
//  , booktitle =    "{SIGMOD} '84, Proceedings of Annual Meeting"
//  , year = 	     1984
//  , editor = 	     "Beatrice Yormark"
//  , volume = 	     "14.2"
//  , series = 	     "{SIGMOD} Record"
//  , month = 	     "June"
//  , publisher =    "{ACM} Press"
//  }


//
//   End of File.
//

