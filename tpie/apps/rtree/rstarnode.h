// -*- C++ -*-
//
//  Description:     declarations for class RStarNode
//  Created:         05.11.1998
//  Author:          Jan Vahrenhold
//  mail:            jan@math.uni-muenster.de
//  $Id: rstarnode.h,v 1.3 2004-08-12 12:37:24 jan Exp $
//  Copyright (C) 1997-2001 by  
// 
//  Jan Vahrenhold
//  Westfaelische Wilhelms-Universitaet Muenster
//  Institut fuer Informatik
//  Einsteinstr. 62
//  D-48149 Muenster
//  GERMANY
//

//  Prevent multiple includes.
#ifndef RSTARNODE_H
#define RSTARNODE_H

#include <portability.h>

//  Include <iostream.h> for output operator.
#include <iostream>

//  Include STL templates pair, list and vector.
#include <utility>
#include <list>
#include <vector>

#include <assert.h>

//  Include TPIE AMI declarations.
#include <ami_stream.h>
#include <ami_block.h>
#include <ami_coll.h>
#include <ami_stack.h>

//  Include declaration of bounding boxes.
#include "rectangle.h"

//  Define constants for describing the kind of node we are looking at.
const unsigned short RNodeTypeInternal = 1;
const unsigned short RNodeTypeLeafNode = 2;
const unsigned short RNodeTypeRoot     = 4;
const unsigned short RNodeTypeLeaf     = 8;

const double MIN_FANOUT_FACTOR = 2.5;

//  Forward declaration of R-Tree base class.
template<class coord_t, class BTECOLL> class RStarTree;

typedef TPIE_OS_SIZE_T children_count_t;

struct _RStarNode_info {
  AMI_bid parent;
  children_count_t children;
  unsigned short flag;
};

//- RStarNode
template<class coord_t, class BTECOLL = BTE_COLLECTION>
class RStarNode: public AMI_block<rectangle<coord_t, AMI_bid>, _RStarNode_info, BTECOLL> {
//. R-tree node
public:

  //- constructor, destructor
  RStarNode();
  RStarNode(AMI_collection_single<BTECOLL>* pcoll, 
	    RStarTree<coord_t, BTECOLL>*          tree, 
	    AMI_bid            parent, 
	    AMI_bid            ID, 
	    children_count_t     maxChildren);

  RStarNode(const RStarNode<coord_t, BTECOLL>& other);
  RStarNode<coord_t, BTECOLL>& operator=(const RStarNode<coord_t, BTECOLL>& other);
  //. The constructor expects a pointer to the tree (this tree is in
  //. main memory all the time), the ID of the parent node (i.e. the
  //. ID of the block that stores the parent node), the ID of the block 
  //. where the leaf is to be stored, and the maximum number of children 
  //. for this node. 
    
  //- operator==, operator!=
  //  bool operator==(const RStarNode<coord_t, BTECOLL>& other) 
  //{ return bid() == other.bid(); }
  //bool operator!=(const RStarNode<coord_t, BTECOLL>& other)
  //. Two R-tree nodes are identical iff their IDs are identical.
  //. The assigment operator is not defined to avoid several nodes
  //. refering to the same block.

  //- ID, tree
  //  AMI_bid ID() const;
  RStarTree<coord_t, BTECOLL>* tree() const { return tree_; }
  //. The block ID of the node and the tree the node bekongs to
  //. can be inquired.

  //- setParent, getParent
  void setParent(AMI_bid parent) { info()->parent = parent; }
  AMI_bid getParent() const { return info()->parent; }
  //. The parent node (given by its block ID) can be set and inquired.

  //- addChild, setChild, getChild, isParent, findChild, removeChild
  void setChild(children_count_t index, const rectangle<coord_t, AMI_bid>& bb) {
    assert(index < maxChildren_+1);  
    // The node is allowed to _temporarily_ overflow.
    el[index] = bb;    
  }
  void addChild(const rectangle<coord_t, AMI_bid>& bb) {
    setChild(numberOfChildren(), bb);
    ++info()->children;
  }
  const rectangle<coord_t, AMI_bid>& getChild(children_count_t index) const {
    assert(index < numberOfChildren());
    return el[index]; 
  }
  bool isParent(const RStarNode<coord_t, BTECOLL>* other) const;
  children_count_t findChild(AMI_bid ID) const;
  children_count_t removeChild(const rectangle<coord_t, AMI_bid>& r);
  children_count_t removeChild(children_count_t ID);
    //. A bounding box can be added as a child of the actual node. 
    //. Precondition: There must be room for the new child.
    //. The bounding box associated with an index can be set and inquired.
    //. Given the ID of a node, it is possible to inquire whether the
    //. actual node is parent of this one. If so, the index of the 
    //. child, otherwise the branching factor of the tree is returned by
    //. the method "findChild".
    //. Using the polymorphism of "removeChild" one can remove a child
    //. given by either its bounding box or its ID. The first method is
    //. used to delete an object from a leaf, the second method is
    //. used to delete an object from an internal node.

    //- setFlag, getFlag
  void setFlag(unsigned short flag) { info()->flag = flag; }
  unsigned short getFlag() const { return info()->flag; }
    //. The flag used to determine the kind of node represented by
    //. the object can be set and inquired.

    //- setAttribute, getAttribute
    //void setAttribute(AMI_bid attribute);
    //AMI_bid getAttribute() const;
    //. The interpretation of the attribute depends of the kind of node.

    //- route
    children_count_t route(const rectangle<coord_t, AMI_bid>& bb);
    //. Find the index of the child whose bounding box needs least enlargement
    //. to include the given bounding box and adjust that child to include
    //. the given box as well.

    //- adjustBoundingRectangle
    void adjustBoundingRectangle(
	AMI_bid         ID, 
	const rectangle<coord_t, AMI_bid>& bb);
    //. Given the ID (not the index!) of a child node and a bounding box 
    //. adjust the bounding box of the child to include the given object.

    //- numberOfChildren, showChildren, updateChildrenParent
  children_count_t numberOfChildren() const { return info()->children; }
  void showChildren() const;
    void updateChildrenParent() const;
    //. The number of children can be inquired. A debugging output of the
    //. children can be sent to 'cerr' by calling the method "showChildren". 
    //. The parent-pointer of all children can be set to the actual node by 
    //. a call to the method "updateChildrenParent".

    //- setCoveringRectangle, getCoveringRectangle
  void setCoveringRectangle(const rectangle<coord_t, AMI_bid>& bb) {
    coveringRectangle_ = bb;
  }
  rectangle<coord_t, AMI_bid> getCoveringRectangle() {
    if (coveringRectangle_.getID() != bid()) {
      coveringRectangle_ = getChild(0);
      for(children_count_t c = 1; c < numberOfChildren(); ++c) {
	coveringRectangle_.extend(getChild(c));
      }
      coveringRectangle_.setID(bid());
    }
    return coveringRectangle_;
  }
  //. Set and inquire the covering rectangle of the node. The
  //. method "getCoveringRectangle" computes the bounding 
  //. rectangle, only if no rectangle has been set/computed before.

    //- isFull
  bool isFull() const { return (numberOfChildren() + 2 >= maxChildren_); }
    //. Do we have to split the node if we want to add another child?

    //- isRoot, isInternalNode, isLeafNode, isLeaf
  bool isRoot() const { return ((getFlag() & RNodeTypeRoot) == RNodeTypeRoot); }
  bool isInternalNode() const { return ((getFlag() & RNodeTypeInternal) == RNodeTypeInternal); }
  bool isLeafNode() const { return ((getFlag() & RNodeTypeLeafNode) == RNodeTypeLeafNode); }
  bool isLeaf() const { return ((getFlag() & RNodeTypeLeaf) == RNodeTypeLeaf); }
    //. The type of the node can be inquired. This is done via the
    //. 'attribute' data member. 

    //- query
    void query(
	const rectangle<coord_t, AMI_bid>&       bb, 
	AMI_stack<AMI_bid>*   candidates, 
	AMI_STREAM<rectangle<coord_t, AMI_bid> >* matches,
	off_t&                 candidatesCounter,
	off_t&                 leafCounter,
	bool                   bruteForce = false) const;
    //. This method realizes the "single shot" query algorithm for R-trees.
    //. Given a query rectangle we look for all subtrees whose bounding
    //. rectangle overlaps the query rectangle and proceed recursively
    //. for each such subtree. If the current node is a leaf, all overlapping
    //. rectangles are written to the stream 'matches', otherwise all
    //. overlapping bounding rectangles of subtrees are pushed on
    //. the stack 'candidates'. The variable 'candiatesCounter' stores 
    //. the length of the stack to avoid one I/O for each size request.
    //. The variable 'leafCounter' is used for statistical purposes.

    //- findNode
    void findNode(
	AMI_bid             nodeID,
	AMI_stack<AMI_bid>* candidates, 
	TPIE_OS_OFFSET&     candidatesCounter) const;
    //. This method realized a depth-first search looking for a node
    //. with a given ID and prints all its contents. For a more detailed
    //. description see "findOverlappingChildren". 
    //. This method is used for debugging purposes.

    //- findLeaf
    AMI_bid findLeaf(const rectangle<coord_t, AMI_bid>& r, 
	                 list<AMI_bid>*  candidates) const;
    //. This method realized a depth-first search looking for a leaf
    //. containing a given object and returns this leaf's ID (or 0 if
    //. the search was unsuccessful. For a more detailed
    //. description see "findOverlappingChildren". Note that this 
    //. algorithm keeps track of all node to be visited in INTERNAL
    //. memory.
    //. This method is used for debugging purposes.

    //- checkChildren
    void checkChildren(
	const rectangle<coord_t, AMI_bid>&                  bb, 
	list<pair<AMI_bid, rectangle<coord_t, AMI_bid> > >& l, 
	TPIE_OS_OFFSET&                                     objectCounter);
    //. This method is called as a subroutine from the tree-checking
    //. procedure and appends the ID and the bounding box of the
    //. current node to the given list (in main-memory).
    //. This method is used for debugging purposes.

    //- chooseSplitAxisAndIndex
    pair<vector<rectangle<coord_t, AMI_bid> >*, children_count_t> chooseSplitAxisAndIndex() const;
    //. This method returns the axis perpendicular to which the split
    //. will be performed and the index of the distribution (according to 
    //. [BKSS90] (p.326).)

  //  friend ostream& operator<<(ostream& os, const RStarNode<coord_t, BTECOLL>& r);
protected:

  RStarTree<coord_t, BTECOLL>* tree_;               
  //  Pointer to the tree.

  children_count_t               maxChildren_;        
  //  Maximum number of children allowed ofr this node.

  rectangle<coord_t, AMI_bid>  coveringRectangle_;  
  //  Can be set by user.

private:
  //  No private members.
};

template<class coord_t, class BTECOLL>
ostream& operator<<(ostream& os, const RStarNode<coord_t, BTECOLL>& r) {
    os << "ID: " << r.bid() << ", parent: ";
    os << r.getParent() << ", flag: ";
    os << r.getFlag();
    if (r.isRoot()) {
	os << " (root)";
    }
    if (r.isInternalNode()) {
	os << " (internal node)";
    }
    if (r.isLeafNode()) {
	os << " (leaf node)";
    }
    if (r.isLeaf()) {
	os << " (leaf)";
    }
    return os;
}

template<class coord_t, class BTECOLL>
RStarNode<coord_t, BTECOLL>::RStarNode() {
    cerr << "You are not suppposed to create an R-tree node ";
    cerr << "without proper parameters. The constructor is:" << "\n"; 
    cerr << "RStarNode<coord_t, BTECOLL>::RStarNode(" << "\n";
    cerr << "     BTE_collection_mmb* storageArea," << "\n";
    cerr << "     RStarTree*          tree," << "\n";
    cerr << "     AMI_bid            parent," << "\n";
    cerr << "     AMI_bid            ID," << "\n";
    cerr << "     children_count_t      maxChildren)" << "\n";
    cerr << "\n";
    abort();
}


template<class coord_t, class BTECOLL>
RStarNode<coord_t, BTECOLL>::RStarNode(AMI_collection_single<BTECOLL>* pcoll, 
					      RStarTree<coord_t, BTECOLL>* tree, 
					      AMI_bid parent, 
					      AMI_bid ID, 
					      children_count_t maxChildren): 
  tree_(tree), AMI_block<rectangle<coord_t, AMI_bid>, _RStarNode_info, BTECOLL>(pcoll, 0, ID) {
    
  //  Initialize the four info fields.
  if (ID == 0) {
    info()->parent = parent;
    info()->children = 0;
    info()->flag = 0;
  }

  //  If a user-defined maximum size of children is given, check whether
  //  this number fits into the block and is not equal to zero.
  maxChildren_ = el.capacity();
  if (maxChildren <= maxChildren_) {
    maxChildren_ = max((children_count_t) 2, maxChildren);
  }
}


template<class coord_t, class BTECOLL>
RStarNode<coord_t, BTECOLL>& RStarNode<coord_t, BTECOLL>::operator=(const RStarNode<coord_t, BTECOLL>& other) {
  if (this != &other) {
    cerr << "Using RStarNode::operator= is not supported.";
    abort();   
  }
  return (*this);
}

template<class coord_t, class BTECOLL>
children_count_t RStarNode<coord_t, BTECOLL>::findChild(AMI_bid ID) const {
  children_count_t counter;
  children_count_t returnValue_ = maxChildren_;
  
  //  Check all children to find the index of the child with 
  //  given ID.
  for(counter = 0; counter < numberOfChildren(); ++counter) {
    if (el[counter].getID() == ID)
      returnValue_ = counter;
  }

  // [tavi] this assert should not be here.
  assert(returnValue_ < maxChildren_);
  return returnValue_;
}

template<class coord_t, class BTECOLL>
bool RStarNode<coord_t, BTECOLL>::isParent(const RStarNode<coord_t, BTECOLL>* other) const {
  children_count_t counter;
  children_count_t returnValue_ = maxChildren_;
  AMI_bid  ID = other->bid();
  
  //  Check all children to find the index of the child with 
  //  the given ID. If there is no such child, the predicate 
  //  returns false.
  for(counter = 0; counter < numberOfChildren(); ++counter) {
    if (el[counter].getID() == ID)
      returnValue_ = counter;
  }

  return (returnValue_ < maxChildren_);
}

template<class coord_t, class BTECOLL>
void RStarNode<coord_t, BTECOLL>::adjustBoundingRectangle(AMI_bid ID, const rectangle<coord_t, AMI_bid>& bb) {
  // [tavi] commented out the temporary.
  //    rectangle<coord_t, AMI_bid> tempBox;
  
  //  Search the child with the matching ID and adjust its bounding
  //  rectangle to include the given bounding box.
  for(children_count_t c = 0; c < numberOfChildren(); ++c) {
    // [tavi] commented out the temporary.
    //	tempBox = el[c];
    if (el[c].getID() == ID) {
      el[c].extend(bb);
      //    el[c] = tempBox;
    }
  }
}

template<class coord_t, class BTECOLL>
void RStarNode<coord_t, BTECOLL>::showChildren() const {
  //  Print all children.
  for(children_count_t c = 0; c < numberOfChildren(); ++c)
    cout << "  " << el[c] << "\n";
}

template<class coord_t, class BTECOLL>
children_count_t RStarNode<coord_t, BTECOLL>::removeChild(const rectangle<coord_t, AMI_bid>& r) {
  children_count_t childFound = numberOfChildren();

  //  Find the child to be deleted. 
  for(children_count_t c = 0; c < numberOfChildren(); ++c) {
    if (el[c] == r) {
      childFound = c;
      break;
    }
  }

  // If the child has been found replace it by the last child.
  if (childFound < numberOfChildren()) {
    if (childFound < numberOfChildren() - 1)
      el[childFound] = el[numberOfChildren()-1];
    el[numberOfChildren()-1] = rectangle<coord_t, AMI_bid>();
    --info()->children;
  }

  return numberOfChildren();
}

template<class coord_t, class BTECOLL>
children_count_t RStarNode<coord_t, BTECOLL>::removeChild(children_count_t idx) {

  if (idx < numberOfChildren()) {
    if (idx < numberOfChildren() -1)
      el[idx] = el[numberOfChildren()-1];
    el[numberOfChildren()-1] = rectangle<coord_t, AMI_bid>();
    --info()->children;
  }

  return numberOfChildren();
}

template<class coord_t, class BTECOLL>
void RStarNode<coord_t, BTECOLL>::checkChildren(const rectangle<coord_t, AMI_bid>& bb, list<pair<AMI_bid, rectangle<coord_t, AMI_bid> > >& l, TPIE_OS_OFFSET& objectCounter) {
  rectangle<coord_t, AMI_bid> toCompare = el[0];
  toCompare.setID(bid());
  
  //  Compute the bounding box of all children. If the child is the root
  //  of a non-trivial subtree, push its ID on the stack.
  for( children_count_t c = 0; c < numberOfChildren(); ++c) {
    toCompare.extend(el[c]);
    if (!isLeaf()) {
      l.push_back(pair<AMI_bid, rectangle<coord_t, AMI_bid> >(el[c].getID(), el[c]));
    } else {
      ++objectCounter;
    }
  }

  //  Compare the computed bounding box against the bounding box 'bb'
  //  that has been stored this node's bounding box its father.
  if (!(toCompare == bb)) {
    if (!isRoot()) {
      cout << "Test failed for "
		<< bid() << ", parent: " << getParent() << ", flag: " << getFlag() << "\n"
		<< toCompare << " should be " << "\n" << bb << "\n";
      showChildren();
      abort();
    }
  }
}

template<class coord_t, class BTECOLL>
void RStarNode<coord_t, BTECOLL>::updateChildrenParent() const {
  AMI_bid       childID;
  RStarNode<coord_t, BTECOLL>* n = NULL;
  
  //  Updating is not possible on leaf-node level.
  if (isLeaf() == false) {
    for(children_count_t c = 0; c < numberOfChildren(); ++c) {
      childID = el[c].getID();
      
      //  Read the child 'counter'.
      n = tree()->readNode(childID);
      
      //  Check whether the parent pointer needs to be updated.
      if (n->getParent() != bid()) {
	//  If so, set the parent pointer the actual ID...
	n->setParent(bid());		
      }

      //  Write the node to disk.
      delete n;
      n = NULL;
    }
  }
}

template<class coord_t, class BTECOLL>
void RStarNode<coord_t, BTECOLL>::query(const rectangle<coord_t, AMI_bid>& bb, AMI_stack<AMI_bid>* candidates, AMI_STREAM<rectangle<coord_t, AMI_bid> >* matches, off_t& candidatesCounter, off_t& leafCounter, bool bruteForce) const {
  rectangle<coord_t, AMI_bid> rb;
  
  if (isLeaf()) {
    //  If the current node is a leaf, write all children that overlap
    //  the given rectangle 'bb' to the output stream 'matches'.
    for(children_count_t c = 0; c < numberOfChildren(); ++c) {
      rb = el[c];
      ++leafCounter;
      if (bb.intersects(rb)) {
	matches->write_item(rb);
      }
    }
  } else {
    //  If the current node is not a leaf, push all children that 
    //  overlap the given rectangle 'bb' onto the stack 'candidates'.
    //  Increment the size counter of the stack accordingly.
    //  If the flag 'bruteForce' is true, push all children onto the
    //  stack, i.e. perform a depth-first traversal of the tree.
    for(children_count_t c = 0; c < numberOfChildren(); ++c) {
      rb = el[c];
      if ((bruteForce) || (bb.intersects(rb))) {
	candidates->push(rb.getID());
	++candidatesCounter;
      }
    }
  }
}

template<class coord_t, class BTECOLL>
AMI_bid RStarNode<coord_t, BTECOLL>::findLeaf(const rectangle<coord_t, AMI_bid>& r, list<AMI_bid>* candidates) const {
  rectangle<coord_t, AMI_bid> rb;

  if (isLeaf()) {
    //  If the current node is a leaf, write all children that overlap
    //  the given rectangle 'bb' to the output stream 'matches'.
    for(children_count_t c = 0; c < numberOfChildren(); ++c) {
      if (el[c] == r) {
	return bid();
      }
    }
  } else {
    //  If the current node is not a leaf, push all children that 
    //  overlap the given rectangle 'bb' onto the stack 'candidates'.
    //  Increment the size counter of the stack accordingly.
    //  If the flag 'bruteForce' is true, push all children onto the
    //  stack, i.e. perform a depth-first traversal of the tree.
    for(children_count_t c = 0; c < numberOfChildren(); ++c) {
      rb = el[c];
      if (r.intersects(rb)) {
	candidates->push_front(rb.getID());
      }
    }  
  }

  return (AMI_bid) 0;
}


template<class coord_t, class BTECOLL>
void RStarNode<coord_t, BTECOLL>::findNode(AMI_bid nodeID, AMI_stack<AMI_bid>* candidates, TPIE_OS_OFFSET& candidatesCounter) const {

  if (isLeaf()) {
    //  If the current node is a leaf, check whether its ID matches
    //  the ID in question. If so, print the leaf and its children.
    if (bid() == nodeID) {
      cout << bid() << ", parent: " << getParent() << ", flag: " << getFlag() << "\n";
      showChildren();
    }
  } else {
    //  If the current node is not a leaf, check whether the ID of
    //  one of its children matches the ID in question. Push all 
    //  children onto the stack and adjust the size counter of the
    //  stack accordingly (perform a depth-first traversal of the tree).
    for(children_count_t c = 0; c < numberOfChildren(); ++c) {
      rectangle<coord_t, AMI_bid> rb = el[c];
      if (rb.getID() == nodeID) {
	cout << rb << "\n";
      }
      candidates->push(rb.getID());
      ++candidatesCounter;
    }
  }
}


template<class coord_t, class BTECOLL>
children_count_t RStarNode<coord_t, BTECOLL>::route(const rectangle<coord_t, AMI_bid>& bb) {
    children_count_t returnValue_ = 0;
    coord_t        area;
    coord_t        perimeter;
    coord_t        otherArea;
    coord_t        otherPerimeter;
    //    rectangle<coord_t, AMI_bid> tempBox;
 
    assert (numberOfChildren() > 0);

    //  For each child compute the area of its bounding rectangle
    //  extended so that it includes the given bounding box.
    area      = el[0].extendedArea(bb) - el[0].area();
    perimeter = el[0].width() + el[0].height();
    
    for(children_count_t c = 1; c < numberOfChildren(); ++c) {
      otherArea      = el[c].extendedArea(bb) - el[c].area();
      otherPerimeter = el[c].width() + el[c].height();
      
      //  Check for which child the difference between the enlarged
      //  area and the original area is minimal. If the difference
      //  is the same for two children, select the child whose 
      //  bounding rectangle has smaller perimeter.	
      if ((otherArea < area) ||
	  ((otherArea == area) && (otherPerimeter < perimeter))){
	area         = otherArea;
	perimeter    = otherPerimeter;
	returnValue_ = c;
      }
    }

    el[returnValue_].extend(bb);

    return returnValue_;
}

template<class coord_t>
struct sortBoxesAlongXAxis {
    inline bool operator()(const rectangle<coord_t, AMI_bid>& t1, 
			   const rectangle<coord_t, AMI_bid>& t2) const {
	if (t1.left() == t2.left()) {
	    return (t1.right() < t2.right());
	}
	else {
	    return (t1.left() < t2.left());
	}
    }
};

template<class coord_t>
struct sortBoxesAlongYAxis {
    inline bool operator()(const rectangle<coord_t, AMI_bid>& t1, 
			   const rectangle<coord_t, AMI_bid>& t2) const {
	if (t1.lower() == t2.lower()) {
	    return (t1.upper() < t2.upper());
	}
	else {
	    return (t1.lower() < t2.lower());
	}
    }
};

template<class coord_t, class BTECOLL>
pair<vector<rectangle<coord_t, AMI_bid> >*, children_count_t> 
RStarNode<coord_t, BTECOLL>::chooseSplitAxisAndIndex() const {

    const unsigned short dim = 2;
    vector<rectangle<coord_t, AMI_bid> >* toSort[dim];
    children_count_t     c;
    children_count_t     c2;
    unsigned short     dimC;

    coord_t S[dim];
    
    for (dimC = 0; dimC < dim; ++dimC) {

	toSort[dimC] = new vector<rectangle<coord_t, AMI_bid> >;

	for (c = 0; c < numberOfChildren(); ++c) {
	    toSort[dimC]->push_back(el[c]);
	}
    }


    children_count_t firstGroupMinSize = (children_count_t)(maxChildren_ / MIN_FANOUT_FACTOR);
    children_count_t distributions = maxChildren_ - 2*firstGroupMinSize + 1;
//    children_count_t distributions = numberOfChildren() - (children_count_t)(maxChildren_ / MIN_FANOUT_FACTOR) + 2;
//    children_count_t firstGroupMinSize = (children_count_t)(maxChildren_ / MIN_FANOUT_FACTOR);

    //  area-value:    area[bb(first group)] +
    //                 area[bb(second group)]
    //  margin-value:  margin[bb(first group)] +
    //                 margin[bb(second group)]
    //  overlap-value: area[bb(first group) $\cap$ bb(second group)]

    VarArray2D<coord_t> areaValue(dim,distributions);
    VarArray2D<coord_t> marginValue(dim,distributions);
    VarArray2D<coord_t> overlapValue(dim,distributions);

    //  "For each axis 
    //     Sort the entries by their lower then by their upper 
    //     value of their rectangles and determine all 
    //     distributions as described above. Compute S, the
    //     sum of all margin-values of the different 
    //     distributions.
    //   end."

    rectangle<coord_t, AMI_bid> group[dim];
    rectangle<coord_t, AMI_bid> firstGroup;
    rectangle<coord_t, AMI_bid> secondGroup;

    for (dimC = 0; dimC < dim; ++ dimC) {
	
	if (dimC == 0) {
	    //  Process x-axis.
	    sort(toSort[0]->begin(), toSort[0]->end(), sortBoxesAlongXAxis<coord_t>());
	} 
	else {
	    //  Process y-axis.
	    sort(toSort[1]->begin(), toSort[1]->end(), sortBoxesAlongYAxis<coord_t>());
	}

	S[dimC] = 0.0;

	firstGroup = (*toSort[dimC])[0];
	secondGroup = (*toSort[dimC])[numberOfChildren()];
    
	//  The first group contains at least the first "firstGroupMinSize" 
	//  boxes while the second group contains at least the last
	//  "firstGroupMinSize" boxes. This is true for all distributions.
	for (c = 1; c < firstGroupMinSize; ++c) {
	    firstGroup.extend((*toSort[dimC])[c]);
	    secondGroup.extend((*toSort[dimC])[numberOfChildren()-c]);
	}
    
	//  Iterate over all possible distributions.
	for (c = 0; c < distributions; ++c) {
	    
	    //  Initialize groups.
	    group[0] = firstGroup;
	    group[1] = secondGroup;
	    
	    //  Update first group.
	    for (c2 = firstGroupMinSize; c2 < firstGroupMinSize+c; ++c2) {
		group[0].extend((*toSort[dimC])[c2]);
	    } 
	    
	    //  Update second group.
	    for (c2 = (firstGroupMinSize + c); c2 < numberOfChildren(); ++c2) {
		group[1].extend((*toSort[dimC])[c2]);
	    }
	    
	    //  Compute area-value, margin-value and overlap-value.
	    areaValue(dimC,c)    = group[0].area() + group[1].area();
	    marginValue(dimC,c)  = group[0].width() + 
		group[0].height() + group[1].width() + group[1].height();
	    overlapValue(dimC,c) = group[0].overlapArea(group[1]);

	    //  Update S.
	    S[dimC] += marginValue(dimC,c);
	}
    }
    
    
    //  "Choose the axis with the minimum S as split axis."
    unsigned short splitAxis = 0;
    children_count_t bestSoFar = 0;
    coord_t minS = S[0];

    for (dimC = 1; dimC < dim; ++dimC) {
	if (S[dimC] < minS) {
	    minS      = S[dimC];
	    splitAxis = dimC;
	}
    }

    for (dimC = 0; dimC < dim; ++dimC) {
	if (dimC != splitAxis) {
	    delete toSort[dimC];
	    toSort[dimC] = NULL;
	}
    }

    //  "Along the chosen split axis, choose the
    //   distribution with the minimum overlap-value.
    //   resolve ties by choosing the distribution with
    //   minimum area-value."

    for(c = 1; c < distributions; ++c) {
	if ((overlapValue(splitAxis,c) < overlapValue(splitAxis,bestSoFar)) ||
	    ((overlapValue(splitAxis,c) == overlapValue(splitAxis,bestSoFar)) && 
	     (areaValue(splitAxis,c) < areaValue(splitAxis,bestSoFar)))) {
	    bestSoFar = c;
	}
    }

    return pair<vector<rectangle<coord_t, AMI_bid> >*, children_count_t>(toSort[splitAxis], bestSoFar);
}


#endif

