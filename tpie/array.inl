// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team

#include "array.h"
#include <iostream>


inline size_t array<bool>::words(size_t m){
	return (sizeof(storage_type)*8-1+m)/(sizeof(storage_type)*8);
}

array<bool>::array(size_t s=0): m_elements(0), m_size(0){
	resize(s);
}

array<bool>::array(size_t s, bool b): m_elements(0), m_size(0){	
	resize(s,b);
}

array<bool>::array(const array<bool> & a):m_elements(0), m_size(0){
	resize(a.m_size);
	for(size_t i=0;i<words(m_size);++i)
		m_elements[i] = a.m_elements[i];
}

array<bool>::~array(){
	resize(0);
}

void array<bool>::operator=(const array<bool> & a){
	resize(a.m_size);
	for(size_t i=0;i<words(m_size);++i)
		m_elements[i] = a.m_elements[i];
}

void array<bool>::resize(size_t s){	   	
	if(s == m_size) return;//0<=m_size - s <sizeof*8 is enough
	delete[] m_elements;	
	if(s==0) {m_elements=0;m_size=0;return;}
	m_size = s;//v*bits;
	m_elements = new storage_type[words(m_size)];
}	

void array<bool>::resize(size_t s,bool b){
	resize(s);
	for(size_t i=0;i<words(m_size);++i)
		m_elements[i] = b?~0:0;	
}

inline array<bool>::return_type array<bool>::operator[](size_t t){
	assert(t < m_size); 
	array<bool>::return_type res; 
	const size_t bits = 8*sizeof(storage_type);
	res.p = &m_elements[t/bits];
	res.index = t % bits;
	return res;
}

inline bool array<bool>::operator[](size_t t)const{
	assert(t < m_size);
	const size_t bits = 8*sizeof(storage_type);
	return (m_elements[t/bits] >>(t%bits))&1;
}

inline size_t array<bool>::size()const{
	return m_size;
}

inline array<bool>::return_type::operator bool()const{
	return (*p >> index)&1;
}

inline array<bool>::return_type & array<bool>::return_type::operator=(const bool  b){
	*p = *p & (~(1<<index));
	*p = *p | (b<<index);
	return *this;
}

inline bool array<bool>::empty()const{
	return m_size ==0;
}
