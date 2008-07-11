// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami_scan.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Header Created: 5/25/94
//
// This file was created mechanically by make, as specified in Makefile.
// There is no reason it should ever be edited by hand.
//
// The following Id string applies to the header used in the processs of
// generating this file.  The header, stored in ami_scan.h.head, may be
// edited if necessary.
//
// $Id: ami_scan.h.head,v 1.5 2003-04-20 23:12:42 tavi Exp $
//
//
#ifndef _AMI_SCAN_H
#define _AMI_SCAN_H

#include <ami_err.h>
#include <ami_stream.h>

typedef int AMI_SCAN_FLAG;

// The base class for scan objects.
class AMI_scan_object {
public:
    virtual AMI_err initialize(void) = 0;
};

// BEGIN MECHANICALLY GENERATED CODE.



// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami_scan.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Header Created: 5/25/94
//
// This file was created mechanically by make, as specified in Makefile.
// There is no reason it should ever be edited by hand.
//
// The following Id string applies to the header used in the processs of
// generating this file.  The header, stored in ami_scan.h.head, may be
// edited if necessary.
//
// $Id: ami_scan.h.head,v 1.5 2003-04-20 23:12:42 tavi Exp $
//
//
#ifndef _AMI_SCAN_H
#define _AMI_SCAN_H

#include <ami_err.h>
#include <ami_stream.h>

typedef int AMI_SCAN_FLAG;

// The base class for scan objects.
class AMI_scan_object {
public:
    virtual AMI_err initialize(void) = 0;
};

// BEGIN MECHANICALLY GENERATED CODE.




// END MECHANICALLY GENERATED CODE.

// The following Id string applies to the tail used in the processs of
// generating this file.  The tail, stored in ami_scan.h.tail, may be
// edited if necessary.
//
// $Id: ami_scan.h.tail,v 1.3 2002-01-14 16:02:43 tavi Exp $

// A class template for copying streams by scanning.
template<class T>
class AMI_identity_scan : public AMI_scan_object {
public:
    AMI_err initialize(void) { return AMI_ERROR_NO_ERROR; }
    AMI_err operate(const T &in, AMI_SCAN_FLAG *sfin,
                    T *out, AMI_SCAN_FLAG *sfout);
};

template<class T>
AMI_err AMI_identity_scan<T>::operate(const T &in, AMI_SCAN_FLAG *sfin,
                                      T *out, AMI_SCAN_FLAG *sfout)
{
    if (*sfout = *sfin) {
        *out = in;
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};


// A copy function for streams
template<class T>
AMI_err AMI_copy_stream(AMI_stream_base<T> *t, AMI_stream_base<T> *s)
{
    AMI_identity_scan<T> id;

    return AMI_scan(t, &id, s);
}

#endif // _AMI_SCAN_H 


// END MECHANICALLY GENERATED CODE.

// The following Id string applies to the tail used in the processs of
// generating this file.  The tail, stored in ami_scan.h.tail, may be
// edited if necessary.
//
// $Id: ami_scan.h.tail,v 1.3 2002-01-14 16:02:43 tavi Exp $

// A class template for copying streams by scanning.
template<class T>
class AMI_identity_scan : public AMI_scan_object {
public:
    AMI_err initialize(void) { return AMI_ERROR_NO_ERROR; }
    AMI_err operate(const T &in, AMI_SCAN_FLAG *sfin,
                    T *out, AMI_SCAN_FLAG *sfout);
};

template<class T>
AMI_err AMI_identity_scan<T>::operate(const T &in, AMI_SCAN_FLAG *sfin,
                                      T *out, AMI_SCAN_FLAG *sfout)
{
    if (*sfout = *sfin) {
        *out = in;
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};


// A copy function for streams
template<class T>
AMI_err AMI_copy_stream(AMI_stream_base<T> *t, AMI_stream_base<T> *s)
{
    AMI_identity_scan<T> id;

    return AMI_scan(t, &id, s);
}

#endif // _AMI_SCAN_H 

