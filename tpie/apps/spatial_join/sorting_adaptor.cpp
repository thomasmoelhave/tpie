// Copyright (c) 1999 Jan Vahrenhold
//
// File:         sorting_adaptor.cpp
// Author:       Jan Vahrenhold <jan@cs.duke.edu>
// Created:      01/24/99
// Description:  
//
// $Id: sorting_adaptor.cpp,v 1.1 2003-11-21 17:01:09 tavi Exp $
//

#include <iostream>
using std::cerr;
using std::endl;
using std::istream;
using std::ostream;
#include "sorting_adaptor.h"
#include <ami_optimized_merge.h>

SortingAdaptor::SortingAdaptor(const char* inStreamName) {
    currentRect_ = new rectangle;
    nextRect_    = NULL;

    char sortedName[strlen(inStreamName)+strlen(".sorted")+1];
    strcpy(sortedName, inStreamName);
    strcat(sortedName, ".sorted");
      
    AMI_STREAM<rectangle> *unsortedStream = new AMI_STREAM<rectangle>(inStreamName);
    unsortedStream->persist(PERSIST_PERSISTENT);

    size_ = unsortedStream->stream_len();

    sortedStream_ = new AMI_STREAM<rectangle>(sortedName);
    sortedStream_->persist(PERSIST_PERSISTENT);

    coord_t dummyKey = 0;
    //rectangle r_dummyKey;
    //  Sort the rectangles in the input stream according to ylo.
    AMI_partition_and_merge(unsortedStream, sortedStream_,
     		sizeof(oid_t) + sizeof(coord_t), dummyKey);
    //AMI_partition_and_merge_stream(unsortedStream, sortedStream_);
    //AMI_sort(unsortedStream, sortedStream_);
    delete unsortedStream;
    sortedStream_->seek(0);

    //  Read the first item from the stream.
    AMI_err result = sortedStream_->read_item(&nextRect_);

    if (result != AMI_ERROR_NO_ERROR) {
	cerr << "Error while using " << sortedName << " (";
	cerr  << result << ")." << endl;
    }
}

SortingAdaptor::SortingAdaptor(const SortingAdaptor& other) {
    *this = other;
}

SortingAdaptor& SortingAdaptor::operator=(const SortingAdaptor& other) {
    if (this != &other) {
	cerr << "Please create a new sorting adaptor instead of ";
	cerr << "copying it." << endl;
    }
    return (*this);
}

SortingAdaptor::~SortingAdaptor() {

    sortedStream_->persist(PERSIST_DELETE);

    delete currentRect_;
    delete sortedStream_;
}

//
//   End of File.
//

