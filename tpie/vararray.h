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

#include <cassert>
#include <cstdlib>
#include <string.h>

//----------------------------------------------------------------------

template <class T> class VarArray1D {

public:
    //  There is no default constructor.

    VarArray1D(memory_size_type dim0);
    VarArray1D(const VarArray1D& other);
    ~VarArray1D();

    VarArray1D<T>& operator=(const VarArray1D<T>& other);

    const T& operator()(memory_size_type index0) const;
    T& operator()(memory_size_type index0);

    memory_size_type size() const;
      
protected:
    T*             data;
    memory_size_type dim;
    
private:
    VarArray1D<T>() {}
    
};

//----------------------------------------------------------------------

template <class T> class VarArray2D {

public:
    //  There is no default constructor.
    VarArray2D(memory_size_type dim0, memory_size_type dim1);
    VarArray2D(const VarArray2D& other);
    ~VarArray2D();

    VarArray2D& operator=(const VarArray2D& other);

    const T& operator()(memory_size_type index0, memory_size_type index1) const;
    T& operator()(memory_size_type index0, memory_size_type index1);

    memory_size_type size() const;
    memory_size_type size(memory_size_type d) const;
      
protected:
    T*           data;
    memory_size_type dim[2];
    
private:
    VarArray2D() {}
    
};

//----------------------------------------------------------------------

template <class T> class VarArray3D {

public:
    //  There is no default constructor.
    VarArray3D(memory_size_type dim0, memory_size_type dim1, memory_size_type dim2);
    VarArray3D(const VarArray3D& other);
    ~VarArray3D();

    VarArray3D& operator=(const VarArray3D& other);

    const T& operator()(memory_size_type index0, memory_size_type index1, memory_size_type index2) const;
    T& operator()(memory_size_type index0, memory_size_type index1, memory_size_type index2);

    memory_size_type size() const;
    memory_size_type size(memory_size_type d) const;
      
protected:
    T*           data;
    memory_size_type dim[3];
    
private:
    VarArray3D() {}
    
};

//----------------------------------------------------------------------
//----------------------------------------------------------------------

template <class T>
VarArray1D<T>::VarArray1D(memory_size_type dim) {
    this->dim = dim;
    
    //  Allocate memory for dim0 elements of type/class T.
    data = new T[dim];
    
    //  Initialize memory.
    memset(static_cast<void*>(data), 
	   0, 
	   dim * sizeof(T));
}

template <class T>
VarArray1D<T>::VarArray1D(const VarArray1D& other) {
    *this = other;
}

template <class T>
VarArray1D<T>::~VarArray1D() {
    // Free allocated memory.
    delete[] data;
}

template <class T>
VarArray1D<T>& VarArray1D<T>::operator=(const VarArray1D<T>& other) {
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
const T& VarArray1D<T>::operator()(memory_size_type index0) const {
    assert(index0 < size());
    
    return data[index0];
}

template <class T>
T& VarArray1D<T>::operator()(memory_size_type index0) {
    assert(index0 < size());
    
    return data[index0];
}

template <class T>
memory_size_type VarArray1D<T>::size() const {
    return dim;
}

//----------------------------------------------------------------------

template <class T>  
VarArray2D<T>::VarArray2D(memory_size_type dim0, memory_size_type dim1) {
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
VarArray2D<T>::VarArray2D(const VarArray2D& other) {
    *this = other;
}

template <class T>  
VarArray2D<T>::~VarArray2D() {
    // Free allocated memory.
    delete[] data;
}

template <class T>  
VarArray2D<T>& VarArray2D<T>::operator=(const VarArray2D& other) {
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
T& VarArray2D<T>::operator()(memory_size_type index0, memory_size_type index1) {
    assert(index0 < size(0));
    assert(index1 < size(1));
    
    return data[index0 * size(1) + index1];
}

template <class T>  
const T& VarArray2D<T>::operator()(memory_size_type index0, memory_size_type index1) const {
    assert(index0 < size(0));
    assert(index1 < size(1));
    
    return data[index0 * size(1) + index1];
}

template <class T>  
memory_size_type VarArray2D<T>::size() const {

    return dim[0] * dim[1];
}

template <class T>  
memory_size_type VarArray2D<T>::size(memory_size_type d) const {
    assert(d<2);
    
    return dim[d];
}


//----------------------------------------------------------------------

template <class T>  
VarArray3D<T>::VarArray3D(memory_size_type dim0, memory_size_type dim1, memory_size_type dim2) {
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
VarArray3D<T>::VarArray3D(const VarArray3D& other) {
    *this = other;
}

template <class T>  
VarArray3D<T>::~VarArray3D() {
    // Free allocated memory.
    delete[] data;
}

template <class T>  
VarArray3D<T>& VarArray3D<T>::operator=(const VarArray3D& other) {
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
T& VarArray3D<T>::operator()(memory_size_type index0, memory_size_type index1, memory_size_type index2) {
    assert(index0 < size(0));
    assert(index1 < size(1));
    assert(index2 < size(2));
    
    return data[index0 * size(1) * size(2) + index1 * size(2) + index2];
}

template <class T>  
const T& VarArray3D<T>::operator()(memory_size_type index0, memory_size_type index1, memory_size_type index2) const {
    assert(index0 < size(0));
    assert(index1 < size(1));
    assert(index2 < size(2));
    
    return data[index0 * size(1) * size(2) + index1 * size(2) + index2];
}

template <class T>  
memory_size_type VarArray3D<T>::size() const {
    
    return dim[0] * dim[1] * dim[2];
}

template <class T>  
memory_size_type VarArray3D<T>::size(memory_size_type d) const {
    assert(d<3);
    
    return dim[d];
}

//----------------------------------------------------------------------

#endif
