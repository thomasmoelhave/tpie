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

// Templates classes for one-, two-, and three-dimensional arrays. 
#ifndef _VARARRAY_H
#define _VARARRAY_H

#include <tpie/portability.h>
#include <tpie/types.h>
#include <cassert>
#include <cstdlib>
#include <string.h>

namespace tpie {
//----------------------------------------------------------------------

template <class T> class var_array_1d {
public:
    //  There is no default constructor.

    var_array_1d(memory_size_type dim0);
    var_array_1d(const var_array_1d& other);
    ~var_array_1d();

    var_array_1d<T>& operator=(const var_array_1d<T>& other);

    const T& operator()(memory_size_type index0) const;
    T& operator()(memory_size_type index0);

    memory_size_type size() const;
      
protected:
    T*             data;
    memory_size_type dim;
    
private:
    var_array_1d<T>() {}
    
};

//----------------------------------------------------------------------

template <class T> class var_array_2d {

public:
    //  There is no default constructor.
    var_array_2d(memory_size_type dim0, memory_size_type dim1);
    var_array_2d(const var_array_2d& other);
    ~var_array_2d();

    var_array_2d& operator=(const var_array_2d& other);

    const T& operator()(memory_size_type index0, memory_size_type index1) const;
    T& operator()(memory_size_type index0, memory_size_type index1);

    memory_size_type size() const;
    memory_size_type size(memory_size_type d) const;
      
protected:
    T*           data;
    memory_size_type dim[2];
    
private:
    var_array_2d() {}
    
};

//----------------------------------------------------------------------

template <class T> class var_array_3d {

public:
    //  There is no default constructor.
    var_array_3d(memory_size_type dim0, memory_size_type dim1, memory_size_type dim2);
    var_array_3d(const var_array_3d& other);
    ~var_array_3d();

    var_array_3d& operator=(const var_array_3d& other);

    const T& operator()(memory_size_type index0, memory_size_type index1, memory_size_type index2) const;
    T& operator()(memory_size_type index0, memory_size_type index1, memory_size_type index2);

    memory_size_type size() const;
    memory_size_type size(memory_size_type d) const;
      
protected:
    T*           data;
    memory_size_type dim[3];
    
private:
    var_array_3d() {}
    
};

//----------------------------------------------------------------------
//----------------------------------------------------------------------

template <class T>
var_array_1d<T>::var_array_1d(memory_size_type dim) {
    this->dim = dim;
    
    //  Allocate memory for dim0 elements of type/class T.
    data = new T[dim];
    
    //  Initialize memory.
    memset(static_cast<void*>(data), 
	   0, 
	   dim * sizeof(T));
}

template <class T>
var_array_1d<T>::var_array_1d(const var_array_1d& other) {
    *this = other;
}

template <class T>
var_array_1d<T>::~var_array_1d() {
    // Free allocated memory.
    delete[] data;
}

template <class T>
var_array_1d<T>& var_array_1d<T>::operator=(const var_array_1d<T>& other) {
    if (this != &other) {
	this->dim = other.dim;
	
	//  Allocate memory for dim elements of type/class T.
	data = new T[dim];
	
	//  Copy objects.
	for(int i = 0; i < dim; i++) {
	    data[i] = other.data[i];
	}

//	//  Initialize memory.
//	memcpy((void*)(this.data), 
//	       (void*)(other.data), 
//	       dim * sizeof(T));
	
    }
    return (*this);	
}

template <class T>
const T& var_array_1d<T>::operator()(memory_size_type index0) const {
    assert(index0 < size());
    
    return data[index0];
}

template <class T>
T& var_array_1d<T>::operator()(memory_size_type index0) {
    assert(index0 < size());
    
    return data[index0];
}

template <class T>
memory_size_type var_array_1d<T>::size() const {
    return dim;
}

//----------------------------------------------------------------------

template <class T>  
var_array_2d<T>::var_array_2d(memory_size_type dim0, memory_size_type dim1) {
    this->dim[0] = dim0;
    this->dim[1] = dim1;
    
    //  Allocate memory for dim0 * dim1 elements of type/class T.
    data = new T[dim0 * dim1];
    
    //  Initialize memory.
    memset(static_cast<void*>(data), 
	   0, 
	   dim0 * dim1 * sizeof(T));
}

template <class T>  
var_array_2d<T>::var_array_2d(const var_array_2d& other) {
    *this = other;
}

template <class T>  
var_array_2d<T>::~var_array_2d() {
    // Free allocated memory.
    delete[] data;
}

template <class T>  
var_array_2d<T>& var_array_2d<T>::operator=(const var_array_2d& other) {
    if (this != &other) {
	this->dim[0] = other.dim[0];
	this->dim[1] = other.dim[1];
	
	//  Allocate memory for dim0 * dim1 elements of type/class T.
	data = new T[dim[0] * dim[1]];
	
	//  Copy objects.
	int len = dim[0] * dim[1];
	for(int i = 0; i < len; i++) {
	    data[i] = other.data[i];
	}

//	//  Initialize memory.
//	memcpy((void*)(this.data), 
//	       (void*)(other.data), 
//	       dim[0] * dim[1] * sizeof(T)); 
	
    }
    return (*this);	
}

template <class T>  
T& var_array_2d<T>::operator()(memory_size_type index0, memory_size_type index1) {
    assert(index0 < size(0));
    assert(index1 < size(1));
    
    return data[index0 * size(1) + index1];
}

template <class T>  
const T& var_array_2d<T>::operator()(memory_size_type index0, memory_size_type index1) const {
    assert(index0 < size(0));
    assert(index1 < size(1));
    
    return data[index0 * size(1) + index1];
}

template <class T>  
memory_size_type var_array_2d<T>::size() const {

    return dim[0] * dim[1];
}

template <class T>  
memory_size_type var_array_2d<T>::size(memory_size_type d) const {
    assert(d<2);
    
    return dim[d];
}


//----------------------------------------------------------------------

template <class T>  
var_array_3d<T>::var_array_3d(memory_size_type dim0, memory_size_type dim1, memory_size_type dim2) {
    this->dim[0] = dim0;
    this->dim[1] = dim1;
    this->dim[2] = dim2;
    
    //  Allocate memory for dim0 * dim1 * dim2 elements of type/class T.
    data = new T[dim0 * dim1 * dim2];
    
    //  Initialize memory.
    memset(static_cast<void*>(data), 
	   0, 
	   dim0 * dim1 * dim2 * sizeof(T));
}

template <class T>  
var_array_3d<T>::var_array_3d(const var_array_3d& other) {
    *this = other;
}

template <class T>  
var_array_3d<T>::~var_array_3d() {
    // Free allocated memory.
    delete[] data;
}

template <class T>  
var_array_3d<T>& var_array_3d<T>::operator=(const var_array_3d& other) {
    if (this != &other) {
	this->dim[0] = other.dim[0];
	this->dim[1] = other.dim[1];
	this->dim[2] = other.dim[2];
	
	//  Allocate memory for dim0 * dim1 * dim2 elements of type/class T.
	data = new T[dim[0] * dim[1] * dim[2]];
	
	//  Copy objects.
	int len = dim[0] * dim[1] * dim[2];
	for(int i = 0; i < len; i++) {
	    data[i] = other.data[i];
	}

//	//  Initialize memory.
//	memcpy((void*)(this.data), 
//	       (void*)(other.data), 
//	       dim[0] * dim[1] * dim[2] * sizeof(T)); 
	
    }
    return (*this);	
}

template <class T>  
T& var_array_3d<T>::operator()(memory_size_type index0, memory_size_type index1, memory_size_type index2) {
    assert(index0 < size(0));
    assert(index1 < size(1));
    assert(index2 < size(2));
    
    return data[index0 * size(1) * size(2) + index1 * size(2) + index2];
}

template <class T>  
const T& var_array_3d<T>::operator()(memory_size_type index0, memory_size_type index1, memory_size_type index2) const {
    assert(index0 < size(0));
    assert(index1 < size(1));
    assert(index2 < size(2));
    
    return data[index0 * size(1) * size(2) + index1 * size(2) + index2];
}

template <class T>  
memory_size_type var_array_3d<T>::size() const {
    
    return dim[0] * dim[1] * dim[2];
}

template <class T>  
memory_size_type var_array_3d<T>::size(memory_size_type d) const {
    assert(d<3);
    
    return dim[d];
}


}
//----------------------------------------------------------------------

#endif
