// Copyright (c) 2002 Jan Vahrenhold
//
// File: vararray.h
// Author: Jan Vahrenhold <jan@math.uni-muenster.de>
// Created: 2002/12/02
//
// Description: Templates classes for one-, two-, and 
//              three-dimensional arrays. 
//
// $Id: vararray.h,v 1.1 2003-04-17 20:20:03 jan Exp $
//
#ifndef _VARARRAY_H
#define _VARARRAY_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

//----------------------------------------------------------------------

template <class T> class VarArray1D {

public:
    //  There is no default constructor.

    VarArray1D(unsigned int dim0);
    VarArray1D(const VarArray1D& other);
    ~VarArray1D();

    VarArray1D<T>& operator=(const VarArray1D<T>& other);

    const T& operator()(size_t index0) const;
    T& operator()(size_t index0);

    size_t size() const;
      
protected:
    T*           data;
    unsigned int dim;
    
private:
    VarArray1D<T>() {}
    
};

//----------------------------------------------------------------------

template <class T> class VarArray2D {

public:
    //  There is no default constructor.
    VarArray2D(unsigned int dim0, unsigned int dim1);
    VarArray2D(const VarArray2D& other);
    ~VarArray2D();

    VarArray2D& operator=(const VarArray2D& other);

    const T& operator()(size_t index0, size_t index1) const;
    T& operator()(size_t index0, size_t index1);

    size_t size() const;
    size_t size(size_t d) const;
      
protected:
    T*           data;
    unsigned int dim[2];
    
private:
    VarArray2D() {}
    
};

//----------------------------------------------------------------------

template <class T> class VarArray3D {

public:
    //  There is no default constructor.
    VarArray3D(unsigned int dim0, unsigned int dim1, unsigned int dim2);
    VarArray3D(const VarArray3D& other);
    ~VarArray3D();

    VarArray3D& operator=(const VarArray3D& other);

    const T& operator()(size_t index0, size_t index1, size_t index2) const;
    T& operator()(size_t index0, size_t index1, size_t index2);

    size_t size() const;
    size_t size(size_t d) const;
      
protected:
    T*           data;
    unsigned int dim[3];
    
private:
    VarArray3D() {}
    
};

//----------------------------------------------------------------------
//----------------------------------------------------------------------

template <class T>
VarArray1D<T>::VarArray1D(unsigned int dim) {
    this->dim = dim;
    
    //  Allocate memory for dim0 elements of type/class T.
    data = new T[dim];
    
    //  Initialize memory.
    memset((void*)data, 
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
const T& VarArray1D<T>::operator()(size_t index0) const {
    assert(index0 < size());
    
    return data[index0];
}

template <class T>
T& VarArray1D<T>::operator()(size_t index0) {
    assert(index0 < size());
    
    return data[index0];
}

template <class T>
size_t VarArray1D<T>::size() const {
    return dim;
}

//----------------------------------------------------------------------

template <class T>  
VarArray2D<T>::VarArray2D(unsigned int dim0, unsigned int dim1) {
    this->dim[0] = dim0;
    this->dim[1] = dim1;
    
    //  Allocate memory for dim0 * dim1 elements of type/class T.
    data = new T[dim0 * dim1];
    
    //  Initialize memory.
    memset((void*)data, 
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
	int dim = dim[0] * dim[1];
	for(int i = 0; i < dim; i++) {
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
T& VarArray2D<T>::operator()(size_t index0, size_t index1) {
    assert(index0 < size(0));
    assert(index1 < size(1));
    
    return data[index0 * size(1) + index1];
}

template <class T>  
const T& VarArray2D<T>::operator()(size_t index0, size_t index1) const {
    assert(index0 < size(0));
    assert(index1 < size(1));
    
    return data[index0 * size(1) + index1];
}

template <class T>  
size_t VarArray2D<T>::size() const {

    return dim[0] * dim[1];
}

template <class T>  
size_t VarArray2D<T>::size(size_t d) const {
    assert(d<2);
    
    return dim[d];
}


//----------------------------------------------------------------------

template <class T>  
VarArray3D<T>::VarArray3D(unsigned int dim0, unsigned int dim1, unsigned int dim2) {
    this->dim[0] = dim0;
    this->dim[1] = dim1;
    this->dim[2] = dim2;
    
    //  Allocate memory for dim0 * dim1 * dim2 elements of type/class T.
    data = new T[dim0 * dim1 * dim2];
    
    //  Initialize memory.
    memset((void*)data, 
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
	int dim = dim[0] * dim[1] * dim[2];
	for(int i = 0; i < dim; i++) {
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
T& VarArray3D<T>::operator()(size_t index0, size_t index1, size_t index2) {
    assert(index0 < size(0));
    assert(index1 < size(1));
    assert(index2 < size(2));
    
    return data[index0 * size(1) * size(2) + index1 * size(2) + index2];
}

template <class T>  
const T& VarArray3D<T>::operator()(size_t index0, size_t index1, size_t index2) const {
    assert(index0 < size(0));
    assert(index1 < size(1));
    assert(index2 < size(2));
    
    return data[index0 * size(1) * size(2) + index1 * size(2) + index2];
}

template <class T>  
size_t VarArray3D<T>::size() const {
    
    return dim[0] * dim[1] * dim[2];
}

template <class T>  
size_t VarArray3D<T>::size(size_t d) const {
    assert(d<3);
    
    return dim[d];
}

//----------------------------------------------------------------------

#endif
