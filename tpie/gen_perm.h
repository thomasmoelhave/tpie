// Copyright (c) 1994 Darren Vengroff
//
// File: ami_gen_perm.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/1/94
//
// $Id: ami_gen_perm.h,v 1.16 2005-11-17 17:11:24 jan Exp $
//
// General permutation.
//
#ifndef _TPIE_AMI_GEN_PERM_H
#define _TPIE_AMI_GEN_PERM_H

// Get definitions for working with Unix and Windows
#include <portability.h>
// Get AMI_scan_object.
#include <scan.h>
// Get AMI_sort
#include <sort.h>

#include <gen_perm_object.h>

namespace tpie {

    namespace ami {
	
// (tavi) moved dest_obj definition down due to error in gcc 2.8.1
	template<class T> class dest_obj;
	
// A comparison operator that simply compares destinations (for sorting).
	template<class T>
	int operator<(const dest_obj<T> &s, const dest_obj<T> &t) {
	    return s.dest < t.dest;
	}
	
	template<class T>
	int operator>(const dest_obj<T> &s, const dest_obj<T> &t) {
	    return s.dest > t.dest;
	}
	
    }  //  ami namespace

}  //  tpie namespace

namespace tpie {
    
    namespace ami {
	
	template<class T>
	class gen_perm_add_dest : scan_object {

	private:
	    // Prohibit these
	    gen_perm_add_dest(const gen_perm_add_dest<T>& other);
	    gen_perm_add_dest<T> operator=(const gen_perm_add_dest<T>& other);
	    
	    gen_perm_object *pgp;
	    TPIE_OS_OFFSET input_offset;
	    
	public:
	    gen_perm_add_dest(gen_perm_object *gpo) : pgp(gpo), input_offset(0) {
		//  No code in this constructor.
	    };

	    virtual ~gen_perm_add_dest(void) {
		//  No code in this destructor.
	    };
	    
	    err initialize(void) {
		input_offset = 0; 
		return NO_ERROR; 
	    };

	    err operate(const T &in, SCAN_FLAG *sfin, dest_obj<T> *out, SCAN_FLAG *sfout) {
		if (!(*sfout = *sfin)) {
		    return SCAN_DONE;
		}

		*out = dest_obj<T>(in, pgp->destination(input_offset++));

		return SCAN_CONTINUE;
	    }
	};

    }  //  ami namespace

}  // tpie namespace
    

namespace tpie {

    namespace ami {
	
	template<class T>
	class gen_perm_strip_dest : scan_object {
	    
	public:
	    err initialize(void) { 
		return NO_ERROR; 
	    };
	    
	    err operate(const dest_obj<T> &in, SCAN_FLAG *sfin, T *out, SCAN_FLAG *sfout) {

		if (!(*sfout = *sfin)) {
		    return SCAN_DONE;
		}

		*out = in.t;
		
		return SCAN_CONTINUE;
	    }
	};

    }  //  ami namespace

}  //  tpie namespace

namespace tpie {

    namespace ami {

	template<class T>
	class dest_obj {

	private:
	    T t;
	    TPIE_OS_OFFSET dest;

	public:
	    dest_obj() : t(), dest(0) {
		//  No code in this constructor.
	    };
	    
	    dest_obj(T t_in, TPIE_OS_OFFSET d) : t(t_in), dest(d) {
		//  No code in this constructor
	    };
	    
	    ~dest_obj() {
		//  No code in this destructor.
	    };

	    
	    // The second alternative caused problems on Win32 (jv)
//#if (__GNUC__ > 2) || (__GNUC__ == 2 &&  __GNUC_MINOR__ >= 8)
	    friend int operator< <> (const dest_obj<T> &s, const dest_obj<T> &t);
	    friend int operator> <> (const dest_obj<T> &s, const dest_obj<T> &t);
//#else
//    friend int operator< (const dest_obj<T> &s, const dest_obj<T> &t);
//    friend int operator> (const dest_obj<T> &s, const dest_obj<T> &t);
//#endif
	    friend err gen_perm_strip_dest<T>::operate(const dest_obj<T> &in,
						       SCAN_FLAG *sfin, T *out,
						       SCAN_FLAG *sfout);
	};

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {
	
	template<class T>
	err general_permute(stream<T> *instream, stream<T> *outstream,
			    gen_perm_object *gpo) {
	    
	    err ae;
	    gen_perm_add_dest<T> gpad(gpo);
	    gen_perm_strip_dest<T> gpsd;
	    stream< dest_obj<T> > sdo_in;
	    stream< dest_obj<T> > sdo_out;
	    
	    // Initialize
	    ae = gpo->initialize(instream->stream_len());
	    if (ae != NO_ERROR) {
		return ae;
	    }
    
	    // Scan the stream, producing an output stream that labels each
	    // item with its destination.
	    ae = scan(instream, &gpad,&sdo_in);	    
	    if (ae != NO_ERROR) {
		return ae;
	    }
	    
	    // Sort by destination.
	    ae = sort(&sdo_in, &sdo_out);
	    if (ae != NO_ERROR) {
		return ae;
	    }
	    
	    // Scan to strip off the destinations.
	    ae = scan(&sdo_out, &gpsd, outstream);	    
	    if (ae != NO_ERROR) {
		return ae;
	    }
	    
	    return NO_ERROR;        
	}
	
    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_GEN_PERM_H 
