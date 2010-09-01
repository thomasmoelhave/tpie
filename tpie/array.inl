// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team

#include "array.h"
#include <iostream>

bitarray::~bitarray(){
	resize(0);
}

inline size_t bitarray::words(size_t m){
	return (sizeof(storage_type)*8-1+m)/(sizeof(storage_type)*8);
}

bitarray::bitarray(size_t s=0): m_elements(0), m_size(0){
	resize(s);
}

bitarray::bitarray(size_t s, bool b): m_elements(0), m_size(0){	
	resize(s,b);
}

bitarray::bitarray(const bitarray & a):m_elements(0), m_size(0){
	resize(a.m_size);
	assert(m_size == a.m_size);
	for(size_t i=0;i<words(m_size);++i)
		m_elements[i] = a.m_elements[i];
}

void bitarray::operator=(const bitarray & a){
	resize(a.m_size);
	assert(m_size == a.m_size);
	for(size_t i=0;i<words(m_size);++i)
		m_elements[i] = a.m_elements[i];
}

void bitarray::resize(size_t s){	   	
	if(s == m_size) return;//0<=m_size - s <sizeof*8 is enough
	delete[] m_elements;	
	if(s==0) {m_elements=0;m_size=0;return;}
	m_size = s;//v*bits;
	m_elements = new storage_type[words(m_size)];
}	

void bitarray::resize(size_t s,bool b){
	resize(s);
	for(size_t i=0;i<words(m_size);++i)
		m_elements[i] = b?~0:0;	
}

inline bitarray::return_type bitarray::operator[](size_t t){
	assert(t < m_size); 
	bitarray::return_type res; 
	const size_t bits = 8*sizeof(storage_type);
	res.p = &m_elements[t/bits];
	res.index = t % bits;
	return res;
}

inline bool bitarray::operator[](size_t t)const{
	assert(t < m_size);
	const size_t bits = 8*sizeof(storage_type);
	return (m_elements[t/bits] >>(t%bits))&1;
}

inline size_t bitarray::size()const{
	return m_size;
}

inline bitarray::return_type::operator bool()const{
	return (*p >> index)&1;
}

inline bitarray::return_type & bitarray::return_type::operator=(const bool  b){
	*p = *p & (~(1<<index));
	*p = *p | (b<<index);
	return *this;
}

inline bool bitarray::empty()const{
	return m_size ==0;
}
 
inline bool bitarray::operator==(const bitarray   & other) const{
	if(this->size() != other.size()) return false;
	for(size_t i=0;i<words(m_size);++i) if(m_elements[i] != other.m_elements[i]) return false;
	return true;
}
