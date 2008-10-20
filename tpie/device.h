// Copyright (c) 1993 Darren Erik Vengroff
//
// File: ami_device.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 8/22/93
//
// $Id: ami_device.h,v 1.6 2005-11-17 17:11:24 jan Exp $
//
#ifndef _TPIE_AMI_DEVICE_H
#define _TPIE_AMI_DEVICE_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <iostream>

#include <tpie/err.h>

namespace tpie {

    namespace ami {
	
	///*class device {

	//private:
	//    void dispose_contents();

	//protected:
	//    unsigned int argc;
	//    char         **argv;

	//public:
	//    device();
	//    device(unsigned int count, char **strings);
	//    device(const device& other);

	//    ~device();

	//    device& operator=(const device& other);
	//    
	//    err set_to_path(const char *path);
	//    err read_environment(const char *name);
	//    
	//    const char * operator[](unsigned int index);

	//    unsigned int arity(void);
	//};*/

    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_DEVICE_H 

