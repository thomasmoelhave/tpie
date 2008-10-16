// Copyright (c) 1993 Darren Erik Vengroff
//
// File: ami_device.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 8/22/93
//
// $Id: ami_device.h,v 1.6 2005-11-17 17:11:24 jan Exp $
//
#ifndef _AMI_DEVICE_H
#define _AMI_DEVICE_H

// Get definitions for working with Unix and Windows
#include <portability.h>

#include <iostream>

#include <err.h>

class AMI_device {
    friend std::ostream &operator<<(std::ostream &os, const AMI_device &dev);
private:
    void dispose_contents();
protected:
    unsigned int argc;
    char **argv;
public:
    AMI_device();
    AMI_device(unsigned int count, char **strings);
    AMI_device(const AMI_device& other);
    ~AMI_device();
    AMI_device& operator=(const AMI_device& other);
    AMI_err set_to_path(const char *path);
    AMI_err read_environment(const char *name);
    const char * operator[](unsigned int index);
    unsigned int arity(void);
};

// Output operator
std::ostream &operator<<(std::ostream &os, const AMI_device &dev);

#endif // _AMI_DEVICE_H 

