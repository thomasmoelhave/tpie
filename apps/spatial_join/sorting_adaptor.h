// Copyright (c) 1999 Jan Vahrenhold
//
// File:         sorting_adaptor.h
// Author:       Jan Vahrenhold <jan@cs.duke.edu>
// Created:      01/24/99
// Description:  
//
// $Id: sorting_adaptor.h,v 1.2 2004-08-12 12:39:09 jan Exp $
//
#ifndef SORTINGADAPTOR_H
#define SORTINGADAPTOR_H

#include <portability.h>

#include <assert.h>
#include <stdlib.h>
#include "app_config.h"
#include <ami_stream.h>
#include "rectangle.h"

//- SortingAdaptor
class SortingAdaptor {
//.  Adaptor to feed the sweeping-based spatial join algorithm with data
//.  rectangles obtained from a sorted sequence of data rectangles.
public:

    //-  SortingAdaptor
    SortingAdaptor(const char* inputStreamName);
    SortingAdaptor(const SortingAdaptor& other);
    SortingAdaptor& operator=(const SortingAdaptor& other);
    ~SortingAdaptor();
    //.  The R-tree adaptor accesses an R-tree given by a pointer.

    //- read_item
    AMI_err read_item(rectangle** item);
    //.  The method "read_item" returns the data rectangle with the 
    //.  smallest lower y-value.

    //- getMinY
    coord_t getMinY() const;
    //.  The method "getMinY" returns the smallest lower y-value that
    //.  is stored within the sorted sequence.

    //- empty
    bool empty() const;
    //.  Checks whether both priority queues are empty.

    //- size
    TPIE_OS_OFFSET size() const;
    //.  Returns the size of the input stream.

protected:

    AMI_STREAM<rectangle>* sortedStream_; //  Stream for sorted sequence.
    rectangle*             currentRect_;  //  Current rectangle.
    rectangle*             nextRect_;     //  Next rectangle.
    bool                   empty_;
    TPIE_OS_OFFSET         size_;

private:

    //  Forbid instantiation without parameters.
    SortingAdaptor();

};

inline bool SortingAdaptor::empty() const {
    return (nextRect_ == NULL);
}

inline TPIE_OS_OFFSET SortingAdaptor::size() const {
    return size_;
}

inline coord_t SortingAdaptor::getMinY() const {
    return nextRect_->ylo;
}

inline AMI_err SortingAdaptor::read_item(rectangle** item) {

    //  Check whether there are items to be processed.
    if (empty()) {

	memset((void*)currentRect_, 0, sizeof(rectangle));

	*item = currentRect_;

	return AMI_ERROR_END_OF_STREAM;
    } 
    else {

	//  Copy the next item.
	memcpy((void*)currentRect_, nextRect_, sizeof(rectangle));

	//  Read the next item from the stream.
	AMI_err result = sortedStream_->read_item(&nextRect_);

	//  Check, whether the stream is empty.
	if (result == AMI_ERROR_END_OF_STREAM) {
	    nextRect_ = NULL;
	}

	*item = currentRect_;
    }

    //  Return AMI_ERROR_NO_ERROR
    return AMI_ERROR_NO_ERROR;
}

#endif

//
//   End of File.
//

