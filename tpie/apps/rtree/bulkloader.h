// -*- C++ -*-
//
//  Description:     declarations for class BulkLoader
//  Created:         27.01.1999
//  Author:          Jan Vahrenhold
//  mail:            jan@math.uni-muenster.de
//  $Id: bulkloader.h,v 1.2 2004-02-05 17:53:35 jan Exp $
//  Copyright (C) 1999-2001 by  
// 
//  Jan Vahrenhold
//  Westfaelische Wilhelms-Universitaet Muenster
//  Institut fuer Informatik
//  Einsteinstr. 62
//  D-48149 Muenster
//  GERMANY
//

//  Prevent multiple #includes.
#ifndef BULKLOADER_H
#define BULKLOADER_H

#include <assert.h>        //  Debugging.
#include <math.h>          //  For log, pow, etc.
#include <iostream>        //  Debugging output.
#include <fstream>         //  Reading the MBR metafile.
#include <stdlib.h>        //  strlen / strcpy
#include <ami_scan.h>      //  for AMI_scan 
#include <ami_sort.h>      //  for AMI_sort
#include "rectangle.h"     //  Data.
#include "rstartree.h"     //  Output data.
#include "rstarnode.h"     //  Needed while bulk loading.
#include "hilbert.h"       //  Computing Hilbert values.

//  Compare by increasing Hilbert value. Note that the order has to be
//  "reversed" as the priority queue sorts by "decreasing" order.
template<class coord_t, class BTECOLL>
struct hilbertPriority {
    inline bool operator()(const pair<RStarNode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG>& t1, 
		                   const pair<RStarNode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG>& t2) const {
	//return (static_cast<TPIE_OS_LONGLONG>(t1.second) > static_cast<TPIE_OS_LONGLONG>(t2.second));
		return t1.second > t2.second;
    }
};

//  Scan all data in the input stream and compute the mininum bounding box.
template<class coord_t>
class scan_computeBoundingBox : public AMI_scan_object {
private:
    coord_t* xMin_;
    coord_t* yMin_;
    coord_t* xMax_;
    coord_t* yMax_;
public:
    
    scan_computeBoundingBox(coord_t* xMin, coord_t* yMin, coord_t* xMax, coord_t* yMax) : xMin_(xMin), yMin_(yMin), xMax_(xMax), yMax_(yMax) {};
    
    AMI_err initialize() {

	*xMin_ = INFINITY;
	*yMin_ = INFINITY;
	*xMax_ = MINUSINFINITY;
	*yMax_ = MINUSINFINITY;
	return AMI_ERROR_NO_ERROR;
    };
    
    AMI_err operate(const rectangle<coord_t, AMI_bid>& in, AMI_SCAN_FLAG* sfin,
                    pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG>* out, AMI_SCAN_FLAG* sfout) {
	//  Write nothing to the output stream.
	*sfout = false;
        if (*sfin) {
	    //  Check whether the new data item extends the bounding box.
	    *xMin_ = min(*xMin_, in.xlo);
	    *yMin_ = min(*yMin_, in.ylo);
	    *xMax_ = max(*xMax_, in.xhi);
	    *yMax_ = max(*yMax_, in.yhi);
            return AMI_SCAN_CONTINUE;
        } 
	else {
            return AMI_SCAN_DONE;
        }
    };
};

//  Scan all data and compute the Hilbert value of each bounding box
//  w.r.t. a "size = 2**k" grid that encloses the data translated to the 
//  origin and scaled by "factor". The translation is determined by
//  "xOffset" and "yOffset".
template<class coord_t>
class scan_scaleAndComputeHilbertValue : public AMI_scan_object {
private:
    coord_t   xOffset_;
    coord_t   yOffset_;
    coord_t   factor_;
    TPIE_OS_LONGLONG  side_;

public:
    
    scan_scaleAndComputeHilbertValue(coord_t xOffset, coord_t yOffset, coord_t factor, TPIE_OS_LONGLONG side) : xOffset_(xOffset), yOffset_(yOffset), factor_(factor), side_(side) {};
    
    AMI_err initialize() {
	return AMI_ERROR_NO_ERROR;
    }
    
    AMI_err operate(const rectangle<coord_t, AMI_bid>& in, AMI_SCAN_FLAG* sfin,
                    pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG>* out, AMI_SCAN_FLAG* sfout) {
        if (*sfout = *sfin) {

	    //  Translate the rectangle by the given offset and
	    //  compute the midpoint in scaled integer coordinates.
	    TPIE_OS_LONGLONG x = (LONGLONG)(factor_ * (LONGLONG)((in.left() + in.right()) / 2.0 - xOffset_));
	    TPIE_OS_LONGLONG y = (LONGLONG)(factor_ * (LONGLONG)((in.lower() + in.upper()) / 2.0 - yOffset_));

	    *out = pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG>(in, computeHilbertValue(x, y, side_));

            return AMI_SCAN_CONTINUE;
        } 
	else {
            return AMI_SCAN_DONE;
        }
    }
};


//- BulkLoader
template<class coord_t, class BTECOLL = BTE_COLLECTION>
class BulkLoader {
//.  Various mechanism for bulk loading R-trees from a given stream 
//.  of rectangles.
public:
    //- BulkLoader
    BulkLoader(
	const char*    inputStream = NULL, 
	unsigned short fanOut = 50);
    BulkLoader(const BulkLoader& other);
    BulkLoader& operator=(const BulkLoader& other);
    ~BulkLoader();
    //. The constructor can handle the (optional) name of the input
    //. stream of rectangles.

    //- setInputStream, getInputStream
    void setInputStream(const char* inputStream);
    const char* getInputStream() const;
    //. The name of the input stream of rectangles can be set and inquired.

    //- setFanOut, getFanOut
    void setFanOut(unsigned short fanOut);
    unsigned short getFanOut() const;
    //. The fan-out for the R-tree to be constructed can be set and inquired.


    //- show_stats
    void show_stats();
    //.  Toggle, reset, and print statistics.

    //- createHilbertRTree, createRStarTree
    AMI_err createHilbertRTree(RStarTree<coord_t, BTECOLL>** tree);
    AMI_err createRStarTree(RStarTree<coord_t, BTECOLL>** tree);
    //.  Create a Hilbert tree by sorting all rectangles according to the
    //.  Hilbert value of the center and the building bottom-up.
    //.  The method "createRStarTree" does an additional caching and
    //.  repacking as suggested by DeWitt et al. for the "Paradise"
    //.  system [DKL+94].

protected:   

    char*          inputStream_;  //  Name of the input stream of rectangles.
    unsigned short fanOut_;       //  Fan-out for the R-tree.

    coord_t        xOffset_;      //  Minimun x-coordinate of data set.
    coord_t        yOffset_;      //  Minimun y-coordinate of data set.
    TPIE_OS_LONGLONG       factor_;       //  Scaling factor for coordinates.
    TPIE_OS_LONGLONG       size_;         //  Size of the integer grid.

    RStarTree<coord_t, BTECOLL>*     tree_;         //  Output object.
    bool           statistics_;   //  Whether to record stats or not.

    priority_queue< pair<RStarNode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG>, 
	vector<pair<RStarNode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG> >, 
	hilbertPriority<coord_t, BTECOLL> >   cachedNodes_; //  Cache

    //  Choosing the best split axis and index for R*-tree node 
    //  splitting [BKSS90]
    pair<vector<rectangle<coord_t, AMI_bid> >*, unsigned short>  chooseSplitAxisAndIndex(
	vector<rectangle<coord_t, AMI_bid> >* sortedVector0, 
	vector<rectangle<coord_t, AMI_bid> >* sortedVector1);

    //  Caching and repacking as proposed by DeWitt et al. [DKL+94]
    void repackCachedNodes(RStarNode<coord_t, BTECOLL>** lastNode);

    // Try to get the mbr of a stream of rectangles
    // using the ".mbr" meta file.
    void getMbr(const char *input_filename, rectangle<coord_t, AMI_bid> **mbr);

private:
    //  No private members.
};

template<class coord_t, class BTECOLL>
inline void BulkLoader<coord_t, BTECOLL>::setInputStream(const char* inputStream) {

    //  Delete ols string if necessary.
    if (inputStream_) {
	delete[] inputStream_;
    }

    //  Copy the argument into a new string.
    if (inputStream) {
	inputStream_ = new char[strlen(inputStream)+1];
	strcpy(inputStream_, inputStream);
    } else {
	inputStream_ = NULL;
    }
}

template<class coord_t, class BTECOLL>
inline const char* BulkLoader<coord_t, BTECOLL>::getInputStream() const {
    return inputStream_;
}

template<class coord_t, class BTECOLL>
inline void BulkLoader<coord_t, BTECOLL>::setFanOut(unsigned short fanOut) {
    if (fanOut > ((LOG_BLK_SZ - (2*sizeof(AMI_bid) + 2*sizeof(unsigned short)))/ sizeof(rectangle<coord_t, AMI_bid>))) {
	cerr << "Warning: fan-out too big (" << fanOut << " > " << (LOG_BLK_SZ - (2*sizeof(AMI_bid) + 2*sizeof(unsigned short)))/ sizeof(rectangle<coord_t, AMI_bid>) <<  ") !" << endl;
    }
    fanOut_ = fanOut;    
}

template<class coord_t, class BTECOLL>
inline unsigned short BulkLoader<coord_t, BTECOLL>::getFanOut() const {
    return fanOut_;
}


template<class coord_t, class BTECOLL>
inline void BulkLoader<coord_t, BTECOLL>::show_stats() {
    if (tree_) {
	tree_->show_stats();
    }
}


template<class coord_t, class BTECOLL>
BulkLoader<coord_t, BTECOLL>::BulkLoader(const char* inputStream, unsigned short fanOut) : inputStream_(NULL), fanOut_(0), cachedNodes_(), xOffset_((coord_t) 0), yOffset_((coord_t) 0), factor_(0), size_(0), statistics_(false), tree_(NULL) {
    setInputStream(inputStream);   
    setFanOut(fanOut);
}

template<class coord_t, class BTECOLL>
BulkLoader<coord_t, BTECOLL>::BulkLoader(const BulkLoader<coord_t, BTECOLL>& other) {
    *this = other;
}

template<class coord_t, class BTECOLL>
BulkLoader<coord_t, BTECOLL>& BulkLoader<coord_t, BTECOLL>::operator=(const BulkLoader<coord_t, BTECOLL>& other) {
    if (this != &other) {
	setInputStream(other.getInputStream());
	setFanOut(other.getFanOut());

	statistics_ = other.statistics_;
	xOffset_    = other.xOffset_;
	yOffset_    = other.yOffset_;
	factor_     = other.factor_;
	size_       = other.size_;	

	tree_       = NULL;
    }
    return (*this);
}

template<class coord_t, class BTECOLL>
BulkLoader<coord_t, BTECOLL>::~BulkLoader() {
    if (inputStream_) {
	delete[] inputStream_;
    }
}

// Try to get the mbr of a stream of rectangles
// using the ".mbr" meta file.
template<class coord_t, class BTECOLL>
void BulkLoader<coord_t, BTECOLL>::getMbr(const char *input_filename, rectangle<coord_t, AMI_bid> **mbr) {

  // Add suffix ".mbr" to the input file name.
  char *mbr_filename = new char[strlen(input_filename)+5];
  strcpy(mbr_filename, input_filename);
  mbr_filename = strcat(mbr_filename, ".mbr");
  // Read the mbr.
  ifstream *mbr_file_stream = new ifstream(mbr_filename);
  if (!(*mbr_file_stream)) {
      *mbr = NULL;
  }
  else {
      *mbr = new rectangle<coord_t, AMI_bid>;
      mbr_file_stream->read((char *) *mbr, sizeof(rectangle<coord_t, AMI_bid>));
  }

  delete mbr_file_stream;
  delete [] mbr_filename;
}

/* [tavi] commented out these two definitions since they appear in rstarnode.H as well.
//  This struct is needed for sorting while R*-tree node splitting.
template<class coord_t>
struct sortBoxesAlongXAxis {
    bool operator()(const rectangle<coord_t, AMI_bid>& t1, const rectangle<coord_t, AMI_bid>& t2) const {
	if (t1.left() == t2.left()) {
	    return (t1.right() < t2.right());
	}
	else {
	    return (t1.left() < t2.left());
	}
    }
};

//  This struct is needed for sorting while R*-tree node splitting.
template<class coord_t, class BTECOLL>
struct sortBoxesAlongYAxis {
    bool operator()(const rectangle<coord_t, AMI_bid>& t1, const rectangle<coord_t, AMI_bid>& t2) const {
	if (t1.lower() == t2.lower()) {
	    return (t1.upper() < t2.upper());
	}
	else {
	    return (t1.lower() < t2.lower());
	}
    }
};
*/

//  Choosing the best split axis and index for R*-tree node splitting
template<class coord_t, class BTECOLL>
pair<vector<rectangle<coord_t, AMI_bid> >*, unsigned short>  BulkLoader<coord_t, BTECOLL>::chooseSplitAxisAndIndex(vector<rectangle<coord_t, AMI_bid> >* sortedVector0, vector<rectangle<coord_t, AMI_bid> >* sortedVector1) {

    vector<rectangle<coord_t, AMI_bid> >* sortedVector[2];
    unsigned short counter;
    unsigned short counter2;

    coord_t S[2];
    
    sortedVector[0] = sortedVector0;
    sortedVector[1] = sortedVector1;

    //  Sort vectors according to the coordinates.
	sort(sortedVector[0]->begin(), sortedVector[0]->end(), sortBoxesAlongXAxis<coord_t>());
	sort(sortedVector[1]->begin(), sortedVector[1]->end(), sortBoxesAlongYAxis<coord_t>());
    
    unsigned short children_ = sortedVector[1]->size();

    unsigned short firstGroupMinSize = (unsigned short)(fanOut_ / MIN_FANOUT_FACTOR);
    unsigned short distributions = fanOut_ - 2*firstGroupMinSize + 1;

    //  area-value:    area[bb(first group)] +
    //                 area[bb(second group)]
    //  margin-value:  margin[bb(first group)] +
    //                 margin[bb(second group)]
    //  overlap-value: area[bb(first group) \cap bb(second group)]

    VarArray2D<coord_t> areaValue(2,distributions);
    VarArray2D<coord_t> marginValue(2,distributions);
    VarArray2D<coord_t> overlapValue(2,distributions);

    //  "For each axis 
    //     Sort the entries by their lower then by their upper 
    //     value of their rectangles and determine all 
    //     distributions as described above. Compute S, the
    //     sum of all margin-values of the different 
    //     distributions.
    //   end."

    rectangle<coord_t, AMI_bid> group[2];
    rectangle<coord_t, AMI_bid> firstGroup;
    rectangle<coord_t, AMI_bid> secondGroup;

    //  Process x-axis.
    S[0] = (coord_t) 0;

    firstGroup = (*sortedVector[0])[0];
    secondGroup = (*sortedVector[0])[children_-1];
    
    //  The first group contains at least the first "firstGroupMinSize" 
    //  boxes while the second group contains at least the last
    //  "firstGroupMinSize" boxes. This is true for all distributions.
    for (counter = 1; counter < firstGroupMinSize; ++counter) {
	firstGroup.extend((*sortedVector[0])[counter]);	
	secondGroup.extend((*sortedVector[0])[(children_-1)-counter]);
    }
    
    //  Iterate over all possible distributions.
    for (counter = 0; counter < distributions; ++counter) {

	//  Initialize groups.
	group[0] = firstGroup;
	group[1] = secondGroup;

	//  Update first group.
	for (counter2 = firstGroupMinSize; counter2 < firstGroupMinSize+counter; ++counter2) {
	    group[0].extend((*sortedVector[0])[counter2]);
	} 

	//  Update second group.
	for (counter2 = (firstGroupMinSize + counter); counter2 < children_; ++counter2) {
	    group[1].extend((*sortedVector[0])[counter2]);
	}
	
	//  Compute area-value, margin-value and overlap-value.
	areaValue(0,counter) = group[0].area() + group[1].area();
	marginValue(0,counter) = group[0].width() + group[0].height()+
	    group[1].width() + group[1].height();
	overlapValue(0,counter) = group[0].overlapArea(group[1]);

	//  Update S.
	S[0] += marginValue(0,counter);
    }
    
    //  Process y-axis.
    S[1] = (coord_t) 0;

    firstGroup = (*sortedVector[1])[0];
    secondGroup = (*sortedVector[1])[children_-1];
    
    //  The first group contains at least the first "firstGroupMinSize" 
    //  boxes while the second group contains at least the last
    //  "firstGroupMinSize" boxes. This is true for all distributions.
    for (counter = 1; counter < firstGroupMinSize; ++counter) {
	firstGroup.extend((*sortedVector[1])[counter]);	
	secondGroup.extend((*sortedVector[1])[(children_-1)-counter]);
    }
    
    //  Iterate over all possible distributions.
    for (counter = 0; counter < distributions; ++counter) {

	//  Initialize groups.
	group[0] = firstGroup;
	group[1] = secondGroup;

	//  Update first group.
	for (counter2 = firstGroupMinSize; counter2 < firstGroupMinSize+counter; ++counter2) {
	    group[0].extend((*sortedVector[1])[counter2]);
	} 

	//  Update second group.
	for (counter2 = (firstGroupMinSize + counter); counter2 < children_; ++counter2) {
	    group[1].extend((*sortedVector[1])[counter2]);
	}
	

	//  Compute area-value, margin-value and overlap-value.
	areaValue(1,counter) = group[0].area() + group[1].area();
	marginValue(1,counter) = group[0].width() + group[0].height()+
	    group[1].width() + group[1].height();
	overlapValue(1,counter) = group[0].overlapArea(group[1]);

	//  Update S.
	S[1] += marginValue(1,counter);
    }

    
    //  "Choose the axis with the minimum S as split axis."
    unsigned short splitAxis = 0;
    unsigned short bestSoFar = 0;

    if (S[1] < S[0]) {
	splitAxis = 1;
    }

    //  "Along the chosen split axis, choose the
    //   distribution with the minimum overlap-value.
    //   resolve ties by choosing the distribution with
    //   minimum area-value."

    for(counter = 1; counter < distributions; ++counter) {
	if ((overlapValue(splitAxis,counter) < overlapValue(splitAxis,bestSoFar)) ||
	    ((overlapValue(splitAxis,counter) == overlapValue(splitAxis,bestSoFar)) && 
	     (areaValue(splitAxis,counter) < areaValue(splitAxis,bestSoFar)))) {
	    bestSoFar = counter;
	}
    }

    return pair<vector<rectangle<coord_t, AMI_bid> >*, unsigned short>(sortedVector[splitAxis], bestSoFar);
}

//  Caching and repacking as proposed by DeWitt et al.
template<class coord_t, class BTECOLL>
void BulkLoader<coord_t, BTECOLL>::repackCachedNodes(RStarNode<coord_t, BTECOLL>** lastNode) {

    unsigned short counter  = 0;

    RStarNode<coord_t, BTECOLL>*  newNode = NULL;
    rectangle<coord_t, AMI_bid> newBB = (*lastNode)->getChild(0);

    for(counter = 1; counter < (*lastNode)->numberOfChildren(); ++counter) {
	newBB.extend((*lastNode)->getChild(counter));
    }

    //  Translate the rectangle by the given offset and
    //  compute the midpoint in scaled integer coordinates.
    TPIE_OS_LONGLONG x = factor_ * (LONGLONG)((newBB.left() + newBB.right()) / 2.0 - xOffset_);
    TPIE_OS_LONGLONG y = factor_ * (LONGLONG)((newBB.lower() + newBB.upper()) / 2.0 - yOffset_);
    
    //  Compute and set the Hilbert value.
    TPIE_OS_LONGLONG hv = computeHilbertValue(x, y, size_);

    //  Add the last node to the stream.
    cachedNodes_.push(pair<RStarNode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG>(*lastNode, hv));
    *lastNode = NULL;

    //  Wait until there are three or more entries in the cache.
    if (cachedNodes_.size() < 3) {
	return;
    }
    
    vector<rectangle<coord_t, AMI_bid> >* toSort[2];
    typename std::vector<rectangle<coord_t, AMI_bid> >::iterator vi;

    toSort[0] = new vector<rectangle<coord_t, AMI_bid> >;
    toSort[1] = new vector<rectangle<coord_t, AMI_bid> >;

    //  Create two arrays containing the boxes to be distributed,
    //  one will be sorted according to the x-values, the other
    //  will be sorted according to the y-values.
    while(!cachedNodes_.empty()) {
	RStarNode<coord_t, BTECOLL>* cachedNode_ = cachedNodes_.top().first;
	for (counter = 0; counter < cachedNode_->numberOfChildren(); ++counter) {
	    toSort[0]->push_back(cachedNode_->getChild(counter));
	    toSort[1]->push_back(cachedNode_->getChild(counter));
	}
	cachedNode_->persist(PERSIST_DELETE);
	delete cachedNode_;
	cachedNodes_.pop();
    }

    vector<rectangle<coord_t, AMI_bid> >* backup = NULL;

    unsigned short exitLoop = 0;

    //  Repeat the splitting of the nodes as long as there is no
    //  proper distribution, i.e., as long as there is no small enough
    //  node.
    do {

	pair<vector<rectangle<coord_t, AMI_bid> >*, unsigned short> seeds;
	unsigned short toDelete         = 0;
	unsigned short firstGroupNumber = 0;

	//  Initialize backup vector.
	backup   = NULL;
	exitLoop = 0;

	do {
	
	    //  Determine best split axis and distribution.
	    seeds = chooseSplitAxisAndIndex(toSort[0], toSort[1]);

	    if (seeds.first == toSort[0]) {
		toDelete = 1;
	    } 
	    else {
		toDelete = 0;
	    }
	    
	    delete toSort[toDelete];
	    toSort[toDelete] = new vector<rectangle<coord_t, AMI_bid> >;

	    //  Compute the index for splitting the vector.
 	    firstGroupNumber = (unsigned short)(fanOut_ / MIN_FANOUT_FACTOR) + seeds.second;

	    //  Check if (a) the first section or (b) the second section
	    //  is small enough to for a node.
	    if (firstGroupNumber < fanOut_) {                           //  (a)
		exitLoop += 1;
	    }
	    if ((seeds.first)->size() - firstGroupNumber < fanOut_) {   //  (b)
		exitLoop += 2;
	    }

	    if (!exitLoop) {

		assert(backup == NULL);

		//  Create new backup vector.
		backup = new vector<rectangle<coord_t, AMI_bid> >;

		//  Proceed with the smaller part of the distribution.
		//  This is to ensure that we will have a proper node
		//  after the next iteration.
		if (firstGroupNumber < (seeds.first)->size() - firstGroupNumber) {
		    //  Copy the smaller part of the distribution.
		    for(counter = 0; counter < firstGroupNumber; ++counter) {
			toSort[toDelete]->push_back((*(seeds.first))[counter]);
		    }

		    //  Create a backup vector for remaining entries.
		    for(counter = firstGroupNumber; counter < (seeds.first)->size(); ++counter) {
			backup->push_back((*(seeds.first))[counter]);
		    }
		    
		    //  Create new vector.
		    delete toSort[1-toDelete];
		    toSort[1-toDelete] = new vector<rectangle<coord_t, AMI_bid> >;

		    //  Copy entries from first vector (smaller part).
		    for(vi = toSort[toDelete]->begin(); vi != toSort[toDelete]->end(); ++vi) {
			toSort[1-toDelete]->push_back(*vi);
		    }		    
		}
		else {
		    //  Copy the smaller part of the distribution.
		    for(counter = firstGroupNumber; counter < (seeds.first)->size(); ++counter) {
			toSort[toDelete]->push_back((*(seeds.first))[counter]);
		    }

		    //  Create a backup vector for remaining entries.
		    for(counter = 0; counter < firstGroupNumber; ++counter) {
			backup->push_back((*(seeds.first))[counter]);
		    }
		    
		    //  Create new vector.
		    delete toSort[1-toDelete];
		    toSort[1-toDelete] = new vector<rectangle<coord_t, AMI_bid> >;

		    //  Copy entries from first vector (smaller part).
		    for(vi = toSort[toDelete]->begin(); vi != toSort[toDelete]->end(); ++vi) {
			toSort[1-toDelete]->push_back(*vi);
		    }		    
		} // else
	    }

	} while (!exitLoop);

	//  Check if the first part of the distribution creates a new node.
	if (exitLoop & 1) {

	    //  Create node for the first part of the distribution.
	    newNode = tree_->readNode(nextFreeBlock);
	    newBB   = (*(seeds.first))[0];

	    //  Copy entries to the new node.
	    for(counter = 0; counter < firstGroupNumber; ++counter) {
		newNode->addChild((*(seeds.first))[counter]);
		newBB.extend((*(seeds.first))[counter]);
	    }

	    //  Translate the rectangle by the given offset and
	    //  compute the midpoint in scaled integer coordinates.
	    x = factor_ * (LONGLONG)((newBB.left() + newBB.right()) / 2.0 - xOffset_);
	    y = factor_ * (LONGLONG)((newBB.lower() + newBB.upper()) / 2.0 - yOffset_);

	    //  Compute and set the Hilbert value.
	    hv = computeHilbertValue(x, y, size_);

	    //  Cache the new node.
	    cachedNodes_.push(pair<RStarNode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG>(newNode, hv));

	    //  Do not delete this node.
	    newNode = NULL;	    

	    //  If the second part of the distribution is too large,
	    //  copy to 'toSort' and repeat.
	    if (!(exitLoop & 2)) {

		//  Initialize temporary vector.
		vector<rectangle<coord_t, AMI_bid> >* tempVector = new vector<rectangle<coord_t, AMI_bid> >;

		//  Copy to the temporary and the other vector.
		for(counter = firstGroupNumber; counter < (seeds.first)->size(); ++counter) {
		    toSort[toDelete]->push_back((*(seeds.first))[counter]);
		    tempVector->push_back((*(seeds.first))[counter]);
		}

		//  Delete the old vector and make the temporary vector
		//  active.
		delete toSort[1-toDelete];
		toSort[1-toDelete] = tempVector;		
	    }
	}

	//  Check if the second part of the distribution creates a new node.
	if (exitLoop & 2) {

	    //  Create node for the second part of the distribution.
	    newNode = tree_->readNode(nextFreeBlock);
	    newBB = (*(seeds.first))[firstGroupNumber];

	    //  Copy entries to the new node.
	    for(counter = firstGroupNumber; counter < (seeds.first)->size(); ++counter) {
		newNode->addChild((*(seeds.first))[counter]);
		newBB.extend((*(seeds.first))[counter]);
	    }

	    //  Translate the rectangle by the given offset and
	    //  compute the midpoint in scaled integer coordinates.
	    TPIE_OS_LONGLONG x = factor_ * (LONGLONG)((newBB.left() + newBB.right()) / 2.0 - xOffset_);
	    TPIE_OS_LONGLONG y = factor_ * (LONGLONG)((newBB.lower() + newBB.upper()) / 2.0 - yOffset_);

	    //  Compute and set the Hilbert value.
	    TPIE_OS_LONGLONG hv = computeHilbertValue(x, y, size_);

	    //  Cache the new node.
	    cachedNodes_.push(pair<RStarNode<coord_t, BTECOLL>*, TPIE_OS_LONGLONG>(newNode, hv));

	    //  Do not delete this node.
	    newNode = NULL;	    

	    //  If the first part of the distribution is to large,
	    //  copy to 'toSort' and repeat.
	    if (!(exitLoop & 1)) {

		//  Initialize temporary vector.
		vector<rectangle<coord_t, AMI_bid> >* tempVector = new vector<rectangle<coord_t, AMI_bid> >;

		//  Copy to the temporary and the other vector.
		for(counter = 0; counter < firstGroupNumber; ++counter) {
		    toSort[toDelete]->push_back((*(seeds.first))[counter]);
		    tempVector->push_back((*(seeds.first))[counter]);
		}

		//  Delete the old vector and make the temporary vector
		//  active.
		delete toSort[1-toDelete];
		toSort[1-toDelete] = tempVector;		
	    }
	}
	
	if (exitLoop == 3) {

	    delete toSort[1-toDelete];

	    if (backup != NULL) {
		toSort[0] = backup;
		toSort[1] = new vector<rectangle<coord_t, AMI_bid> >;

		for(vi = backup->begin(); vi != backup->end(); ++vi) {
		    toSort[1]->push_back(*vi);
		}
	    }
	}
    } while ((exitLoop != 3) || (backup != NULL));
}

template<class coord_t, class BTECOLL>
AMI_err BulkLoader<coord_t, BTECOLL>::createHilbertRTree(RStarTree<coord_t, BTECOLL>** tree) {

    if (getInputStream() == NULL) {
	*tree = NULL;
	return AMI_ERROR_END_OF_STREAM;
    }
    
    AMI_err result = AMI_ERROR_NO_ERROR;

    char* treeName = new char[strlen(getInputStream()) + strlen(".hrtree") + 1];
    strcpy(treeName, getInputStream());
    strcat(treeName, ".hrtree");

    //  Create a new tree object.
    tree_ = new RStarTree<coord_t, BTECOLL>(treeName, fanOut_);
	
	delete[] treeName;

    if (tree_->readTreeInfo()) {

	//  Copy the pointer to the tree.
	*tree = tree_;
	tree_ = NULL;

	return AMI_ERROR_NO_ERROR;
    }


    AMI_STREAM<rectangle<coord_t, AMI_bid> > amis(getInputStream());
    AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >* boxUnsorted = new AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >("unsorted.boxes");
    AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >* boxSorted = new AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >("sorted.boxes");

    amis.persist(PERSIST_PERSISTENT);
    boxUnsorted->persist(PERSIST_PERSISTENT);
    boxSorted->persist(PERSIST_PERSISTENT);

    //  Find the minimum bounding box of all rectangles in the stream.
    xOffset_ = (coord_t) 0;
    yOffset_ = (coord_t) 0;

    coord_t xMax = (coord_t) 0;
    coord_t yMax = (coord_t) 0;

    rectangle<coord_t, AMI_bid>* mbr = NULL;

    getMbr(getInputStream(), &mbr);

    if (mbr == NULL) {
	scan_computeBoundingBox<coord_t> scb(&xOffset_, &yOffset_, &xMax, &yMax);

	//  Scan data to compute minimum bounding box.
	AMI_scan(&amis, &scb, boxUnsorted);
    }
    else {
	xOffset_ = mbr->xlo;
	xMax     = mbr->xhi;
	yOffset_ = mbr->ylo;
	yMax     = mbr->yhi;

	delete mbr;    
    }

#if (COORD_T==INT)
    coord_t scaleFactor = 1;
#elif
    coord_t scaleFactor = 1000000.0;
#endif

    coord_t maxExtent = max(xMax-xOffset_, yMax-yOffset_) * scaleFactor;
    size_ = (LONGLONG)(pow((float)2, (int)((log((float)maxExtent)/log((float)2)) + 1.0)));
    scan_scaleAndComputeHilbertValue<coord_t> ssb(xOffset_, yOffset_, scaleFactor, size_);

    //  Scan data to scale the midpoint of each MBR such that it fits
    //  in the grid. The Hilbert value of each scaled midpoint is stored 
    //  with the MBRs.
    AMI_scan(&amis, &ssb, boxUnsorted);

    //  Sort MBRs according to their Hilbert values.
    AMI_sort(boxUnsorted, boxSorted);

    boxUnsorted->persist(PERSIST_DELETE);
    boxSorted->persist(PERSIST_DELETE);

    delete boxUnsorted;
    boxUnsorted = NULL;

    off_t                       streamLength = 0;
    off_t                       streamCounter = 0;
    unsigned short              level = 0;
    unsigned short              counter = 0;
    unsigned short              childCounter = 0;
    unsigned short              nodesCreated = 0;
    pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG>* currentObject = NULL;
//    AMI_bid                       parentID;
    rectangle<coord_t, AMI_bid>                   bb;
    RStarNode<coord_t, BTECOLL>*                  currentNode = tree_->readNode(nextFreeBlock);

    AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >* currentLevel_ = boxSorted;
    AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >* nextLevel_;

    off_t minimumPacking = (fanOut_ * 3) / 4;
    double increaseRatio = 1.20;

    //  Bottom-up construction of the Hilbert R-Tree
    //  While there is more than one node on the current level...
    do {

	nodesCreated     = 1;
	streamLength     = currentLevel_->stream_len();

	if (streamLength == 0) {
	    result = AMI_ERROR_END_OF_STREAM;
	    break; // Exit loop.
	}

	//  Start at the beginning of the current level.
	currentLevel_->seek(0);

	//  Create a repository for the next level.
	nextLevel_   = new AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >;
	childCounter = 0;

	//  Scan the current level and group up to 'fanOut_' items 
	//  into one new node.
	for (streamCounter = 0; streamCounter < streamLength; ++streamCounter) {
	    currentLevel_->read_item(&currentObject);

	    if ((currentNode->isFull()) ||
		((currentNode->numberOfChildren() > minimumPacking) &&
		 ((coord_t)bb.extendedArea((*currentObject).first) / (coord_t)bb.area() > increaseRatio)))
	    {

		//  If the current node is full, label it with the
		//  correct flag (according to the level and
		//  compute the ID of the parent node.
		if (level == 0) {
		    currentNode->setFlag(RNodeTypeLeaf);
		}
		else {
		    currentNode->setFlag(RNodeTypeInternal);
		    currentNode->updateChildrenParent();
		}
		currentNode->setParent(currentNode->bid());

		//  Update the bounding box of the current node.
		bb.setID(currentNode->bid());
		currentNode->setCoveringRectangle(bb);
		
		//  Write the bounding box to the next level's stream.
		nextLevel_->write_item(pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG>(bb,0));

		//  Save the node in the tree.
		delete currentNode;

		//  Create the next node.
		currentNode = tree_->readNode(nextFreeBlock);
		++nodesCreated;
		childCounter = 0;
	    } 

	    //  Add the current object to the current node.

	    currentNode->addChild((*currentObject).first);

	    if (!childCounter) {
		bb = (*currentObject).first;
	    }
	    else {
		bb.extend((*currentObject).first);
	    }

	    ++childCounter;
	}

	if (nodesCreated > 1) {
	    //  Compute the flag and the parent ID of the current node.
	    if (level == 0) {
		currentNode->setFlag(RNodeTypeLeaf);
	    }
	    else {
		currentNode->setFlag(RNodeTypeInternal);
		currentNode->updateChildrenParent();
	    }
	    currentNode->setParent(currentNode->bid());

	    //  Update the bounding box of the current node.
	    bb.setID(currentNode->bid());
	    currentNode->setCoveringRectangle(bb);
	    
	    //  Write the bounding box to the next level's stream
	    nextLevel_->write_item(pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG>(bb,0));
	    
	    //  Save the node in the tree.
	    delete currentNode;
	    
	    //  Advance one level.
	    ++level;
	    delete currentLevel_;
	    currentLevel_ = nextLevel_;
	    
	    //  Create the next node.
	    currentNode = tree_->readNode(nextFreeBlock);
	    childCounter = 0;
	}
	else {
	    //  Update the root of the tree (i.e. set the correct flag and
	    //  let the parent ID be the ID of the node itself).
	    bb.setID(currentNode->bid());
	    currentNode->setParent(currentNode->bid());
	    currentNode->setFlag(RNodeTypeRoot);
	    currentNode->setCoveringRectangle(bb);

	    nextLevel_->persist(PERSIST_DELETE);
	    delete nextLevel_;
	}
    } while (nodesCreated > 1);

    //  Delete the stream for the current level.
    currentLevel_->persist(PERSIST_DELETE);
    delete currentLevel_;

    //  Set the tree's root ID, height, and number of objects.
    tree_->setTreeInformation(currentNode->bid(), level, amis.stream_len());

    delete currentNode;

    if (statistics_) {

	show_stats();
	cout << endl;
    }

    //  Save tree info.
    tree_->writeTreeInfo();

    //  Copy the pointer to the tree.
    *tree = tree_;
    tree_ = NULL;

    return result;
}

template<class coord_t, class BTECOLL>
AMI_err BulkLoader<coord_t, BTECOLL>::createRStarTree(RStarTree<coord_t, BTECOLL>** tree) {

    if (getInputStream() == NULL) {
	*tree = NULL;
	return AMI_ERROR_END_OF_STREAM;
    }
    
    AMI_err result = AMI_ERROR_NO_ERROR;

    char* treeName = new char[strlen(getInputStream()) + strlen(".rstree") + 1];
    strcpy(treeName, getInputStream());
    strcat(treeName, ".rstree");

    //  Create a new tree object.
    tree_ = new RStarTree<coord_t, BTECOLL>(treeName, fanOut_);	
    
	delete[] treeName;

    if (tree_->readTreeInfo()) {

	//  Copy the pointer to the tree.
	*tree = tree_;
	tree_ = NULL;
	
	return AMI_ERROR_NO_ERROR;
    }


    AMI_STREAM<rectangle<coord_t, AMI_bid> > amis(getInputStream());
    AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >* boxUnsorted = new AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >("unsorted.boxes");
    AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >* boxSorted = new AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >("sorted.boxes");

    amis.persist(PERSIST_PERSISTENT);
    boxUnsorted->persist(PERSIST_PERSISTENT);
    boxSorted->persist(PERSIST_PERSISTENT);

    //  Find the minimum bounding box of all rectangles in the stream.
    xOffset_ = (coord_t) 0;
    yOffset_ = (coord_t) 0;

    coord_t xMax = (coord_t) 0;
    coord_t yMax = (coord_t) 0;

    rectangle<coord_t, AMI_bid>* mbr = NULL;

    getMbr(getInputStream(), &mbr);

    if (mbr == NULL) {
	scan_computeBoundingBox<coord_t> scb(&xOffset_, &yOffset_, &xMax, &yMax);

	//  Scan data to compute minimum bounding box.
	AMI_scan(&amis, &scb, boxUnsorted);
    }
    else {
	xOffset_ = mbr->xlo;
	xMax     = mbr->xhi;
	yOffset_ = mbr->ylo;
	yMax     = mbr->yhi;

	delete mbr;
    }

#if (COORD_T==INT)
    coord_t scaleFactor = 1;
#elif
    coord_t scaleFactor = 1000000;
#endif

    coord_t maxExtent = max(xMax-xOffset_, yMax-yOffset_) * scaleFactor;
    size_ = (LONGLONG)(pow((float)2, (int)((log((float)maxExtent)/log((float)2)) + 1.0)));

    scan_scaleAndComputeHilbertValue<coord_t> ssb(xOffset_, yOffset_, scaleFactor, size_);

    //  Scan data to scale the midpoint of each MBR such that it fits
    //  in the grid. The Hilbert value of each scaled midpoint is stored 
    //  with the MBRs.
    AMI_scan(&amis, &ssb, boxUnsorted);

    //  Sort MBRs according to their Hilbert values.
    AMI_sort(boxUnsorted, boxSorted);

    boxUnsorted->persist(PERSIST_DELETE);
    boxSorted->persist(PERSIST_DELETE);

    delete boxUnsorted;
    boxUnsorted = NULL;

    off_t                   streamLength = 0;
    off_t                   streamCounter = 0;
    unsigned short          level = 0;
    unsigned short          counter = 0;
    unsigned short          childCounter = 0;
    unsigned short          nodesCreated = 0;
    pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG>* currentObject = NULL;
    AMI_bid                    parentID = 0;
    rectangle<coord_t, AMI_bid>               bb;
    RStarNode<coord_t, BTECOLL>*              currentNode = tree_->readNode(nextFreeBlock);

    AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >* currentLevel_ = boxSorted;
    AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >* nextLevel_;

    off_t minimumPacking = (fanOut_ * 3) / 4;
    double increaseRatio = 1.20;

    //  Bottom-up construction of the (Hilbert-)R*-Tree
    //  While there is more than one node on the current level...
    do {
	nodesCreated     = 1;
	streamLength     = currentLevel_->stream_len();

	if (streamLength == 0) {
	    result = AMI_ERROR_END_OF_STREAM;
	    break; // Exit loop.
	} 

	//  Start at the beginning of the current level.
	currentLevel_->seek(0);

	//  Create a repository for the next level.
	nextLevel_   = new AMI_STREAM<pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG> >;
	childCounter = 0;

	//  Scan the current level and group up to 'fanOut_' items 
	//  into one new node.
	for (streamCounter = 0; streamCounter < streamLength; ++streamCounter) {
	    currentLevel_->read_item(&currentObject);

	    if ((currentNode->isFull()) ||
		((currentNode->numberOfChildren() > minimumPacking) &&
		 ((coord_t)bb.extendedArea((*currentObject).first) / (coord_t)bb.area() > increaseRatio)))
	    {
		
		//  Add current node to the cache and repack nodes cached
		//  there. The object *currentNode will be deleted there.
		repackCachedNodes(&currentNode);

		while (cachedNodes_.size() > 2) {

		    RStarNode<coord_t, BTECOLL>* tempNode = cachedNodes_.top().first;
		    cachedNodes_.pop();

		    bb = tempNode->getChild(0);

		    for(counter = 0; counter < tempNode->numberOfChildren(); ++counter) {
			bb.extend(tempNode->getChild(counter));
		    }


		    //  If the current node is full, label it with the
		    //  correct flag (according to the level and
		    //  compute the ID of the parent node.
		    if (level == 0) {
			tempNode->setFlag(RNodeTypeLeaf);
		    }
		    else {
			tempNode->setFlag(RNodeTypeInternal);
		    }

		    tempNode->updateChildrenParent();
		    
		    //  Update the bounding box of the current node.
		    bb.setID(tempNode->bid());
		    tempNode->setCoveringRectangle(bb);
		    
		    //  Write the bounding box to the next level's stream.
		    nextLevel_->write_item(pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG>(bb,0));

		    //  Save the node in the tree.
		    tempNode->persist(PERSIST_PERSISTENT);
		    delete tempNode;
		}

		//  Create the next node.
		currentNode = tree_->readNode(nextFreeBlock);
		++nodesCreated;
		childCounter = 0;
	    } 

	    //  Add the current object to the current node.

	    currentNode->addChild((*currentObject).first);

	    if (!childCounter) {
		bb = (*currentObject).first;
	    }
	    else {
		bb.extend((*currentObject).first);
	    }

	    ++childCounter;
	}

	if (nodesCreated > 1) {

	    //  Add current node to the cache and repack nodes cached
	    //  there. The object *currentNode will be deleted there.
	    repackCachedNodes(&currentNode);

	    while (!cachedNodes_.empty()) {
		
		RStarNode<coord_t, BTECOLL>* tempNode = cachedNodes_.top().first;
		cachedNodes_.pop();
		
		bb = tempNode->getChild(0);

		for(counter = 0; counter < tempNode->numberOfChildren(); ++counter) {
		    bb.extend(tempNode->getChild(counter));
		}
		
		//  If the current node is full, label it with the
		//  correct flag (according to the level and
		//  compute the ID of the parent node.
		if (level == 0) {
		    tempNode->setFlag(RNodeTypeLeaf);
		}
		else {
		    tempNode->setFlag(RNodeTypeInternal);
		}
		
		tempNode->updateChildrenParent();
		
		//  Update the bounding box of the current node.
		bb.setID(tempNode->bid());
		tempNode->setCoveringRectangle(bb);
		
		//  Write the bounding box to the next level's stream.
		nextLevel_->write_item(pair<rectangle<coord_t, AMI_bid>, TPIE_OS_LONGLONG>(bb,0));
		
		//  Save the node in the tree.
		tempNode->persist(PERSIST_PERSISTENT);
		delete tempNode;
	    }
		
	    //  Delete the current level.
	    currentLevel_->persist(PERSIST_DELETE);
	    delete currentLevel_;

	    //  Advance one level.
	    currentLevel_ = nextLevel_;
	    ++level;
	    
	    //  Create the next node.
	    currentNode = tree_->readNode(nextFreeBlock);
	    childCounter = 0;
	}
	else {

	    //  Update the root of the tree (i.e. set the correct flag and
	    //  let the parent ID be the ID of the node itself).

	    //  Make the children aware of their parent.
	    currentNode->updateChildrenParent();

	    //  Make the actual node the root.
	    bb.setID(currentNode->bid());
	    currentNode->setParent(currentNode->bid());
	    currentNode->setFlag(RNodeTypeRoot);
	    currentNode->setCoveringRectangle(bb);

	    //  Delete the stream for the next level.
	    nextLevel_->persist(PERSIST_DELETE);
	    delete nextLevel_;
	}

	assert(cachedNodes_.empty());

    } while (nodesCreated > 1);


    //  Delete the stream for the current level.
    currentLevel_->persist(PERSIST_DELETE);
    delete currentLevel_;

    //  Set the tree's root ID, height, and number of objects.
    tree_->setTreeInformation(currentNode->bid(), level, amis.stream_len());

    delete currentNode;

    if (statistics_) {

	show_stats();
	cout << endl;
    }

    //  Save tree info.
    tree_->writeTreeInfo();

    //  Copy the pointer to the tree.
    *tree = tree_;
    tree_ = NULL;

    return result;
}


#endif

//
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
//  @inproceedings{DKL+94
//  , author = 	 "David J. DeWitt and Navin Kabra and Jun Luo and 
//                 Jignesh M. Patel and Jie-Bing Yu"
//  , title = 	 "Client-Server {P}aradise"
//  , pages = 	 "558--569"
//  , booktitle = "Proceedings of the 20th International
//                 Conference on Very Large Data Bases (VLDB'94)"
//  , year =       1994
//  , editor =    "Jorge B. Bocca and Matthias Jarke and Carlo Zaniolo" 
//  , publisher = "Morgan Kaufmann"
//  }
