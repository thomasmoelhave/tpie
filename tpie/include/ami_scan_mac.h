// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami_scan_mac.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/24/94
//
// $Id: ami_scan_mac.h,v 1.1 1994-05-25 19:33:40 dev Exp $
//
#ifndef _AMI_SCAN_MAC_H
#define _AMI_SCAN_MAC_H

// Just in case, make sure we know about the AMI_STREAM class.
// Note that AMI_STREAM is a macro that will have been defined
// to match a particular implementation before this header file
// is loaded.
template<class T> class AMI_STREAM;

// Macros for defining parameters to AMI_scan()
#define __SPARM_BASE(T,io,n) AMI_STREAM< T ## n > *io ## n
#define __SPARM_1(T,io) __SPARM_BASE(T,io,1)  
#define __SPARM_2(T,io) __SPARM_BASE(T,io,2), __SPARM_1(T,io)
#define __SPARM_3(T,io) __SPARM_BASE(T,io,3), __SPARM_2(T,io)
#define __SPARM_4(T,io) __SPARM_BASE(T,io,4), __SPARM_3(T,io)

// Macros for defining types in a template for AMI_scan()
#define __STEMP_BASE(T,n) class T ## n
#define __STEMP_1(T) __STEMP_BASE(T,1)  
#define __STEMP_2(T) __STEMP_BASE(T,2), __STEMP_1(T)
#define __STEMP_3(T) __STEMP_BASE(T,3), __STEMP_2(T)
#define __STEMP_4(T) __STEMP_BASE(T,4), __STEMP_3(T)

// Temporary space used within AMI_scan
#define __STS_BASE(T,t,n) T ## n t ## n 
#define __STSPACE_1(T,t) __STS_BASE(T,t,1)
#define __STSPACE_2(T,t) __STS_BASE(T,t,2) ; __STSPACE_1(T,t)
#define __STSPACE_3(T,t) __STS_BASE(T,t,3) ; __STSPACE_2(T,t)
#define __STSPACE_4(T,t) __STS_BASE(T,t,4) ; __STSPACE_3(T,t)

// An array of flags.
#define __FSPACE(f,n) AMI_SCAN_FLAG f[n]

// Read inputs into temporary space.  Set the flag based on whether the
// read was succesful or not.  If it was unsuccessful for any reason other
// than EOS, then break out of the scan loop.
#define __STSR_BASE(t,ts,f,e,n)						    \
if (!(f[n-1] = ((e = ts ## n->get_item(&t ## n)) == AMI_ERROR_NO_ERROR))) {   \
    if (e != AMI_ERROR_EOS) {						    \
        break;								    \
    }									    \
}

#define __STS_READ_1(t,ts,f,e) __STSR_BASE(t,ts,f,e,1) 
#define __STS_READ_2(t,ts,f,e) __STSR_BASE(t,ts,f,e,2) __STS_READ_1(t,ts,f,e)
#define __STS_READ_3(t,ts,f,e) __STSR_BASE(t,ts,f,e,3) __STS_READ_2(t,ts,f,e)
#define __STS_READ_4(t,ts,f,e) __STSR_BASE(t,ts,f,e,4) __STS_READ_3(t,ts,f,e)

// Write outputs.  Only write if the flag is set.  If there is an
// error during the write, then break out of the scan loop.
#define __STSW_BASE(u,us,f,e,n)						    \
if (f[n-1] && (e = us ## n -> put_item(u ## n)) != AMI_ERROR_NO_ERROR) {    \
    break;								    \
}

#define __STS_WRITE_1(u,us,f,e) __STSW_BASE(u,us,f,e,1)
#define __STS_WRITE_2(u,us,f,e) __STSW_BASE(u,us,f,e,2) __STS_WRITE_1(u,us,f,e)
#define __STS_WRITE_3(u,us,f,e) __STSW_BASE(u,us,f,e,3) __STS_WRITE_2(u,us,f,e)
#define __STS_WRITE_4(u,us,f,e) __STSW_BASE(u,us,f,e,4) __STS_WRITE_3(u,us,f,e)


// Arguments to the operate() call
#define __SCA_BASE(t,n) t ## n
#define __SCALL_ARGS_1(t) __SCA_BASE(t,1)
#define __SCALL_ARGS_2(t) __SCA_BASE(t,2), __SCALL_ARGS_1(t)
#define __SCALL_ARGS_3(t) __SCA_BASE(t,3), __SCALL_ARGS_2(t)
#define __SCALL_ARGS_4(t) __SCA_BASE(t,4), __SCALL_ARGS_3(t)

// Operate on the inputs to produce the outputs.
#define __SCALL_BASE(t,nt,if,sop,u,nu,of) \
    sop->operate(__SCALL_ARGS_ ## nt (*t), if, __SCALL_ARGS_ ## nu (&u), of)

#define __SCALL_OP_1_1(t,if,sop,u,of) __SCALL_BASE(t,1,if,sop,u,1,of)
#define __SCALL_OP_1_2(t,if,sop,u,of) __SCALL_BASE(t,1,if,sop,u,2,of)
#define __SCALL_OP_1_3(t,if,sop,u,of) __SCALL_BASE(t,1,if,sop,u,3,of)
#define __SCALL_OP_1_4(t,if,sop,u,of) __SCALL_BASE(t,1,if,sop,u,4,of)

#define __SCALL_OP_2_1(t,if,sop,u,of) __SCALL_BASE(t,2,if,sop,u,1,of)
#define __SCALL_OP_2_2(t,if,sop,u,of) __SCALL_BASE(t,2,if,sop,u,2,of)
#define __SCALL_OP_2_3(t,if,sop,u,of) __SCALL_BASE(t,2,if,sop,u,3,of)
#define __SCALL_OP_2_4(t,if,sop,u,of) __SCALL_BASE(t,2,if,sop,u,4,of)

#define __SCALL_OP_3_1(t,if,sop,u,of) __SCALL_BASE(t,3,if,sop,u,1,of)
#define __SCALL_OP_3_2(t,if,sop,u,of) __SCALL_BASE(t,3,if,sop,u,2,of)
#define __SCALL_OP_3_3(t,if,sop,u,of) __SCALL_BASE(t,3,if,sop,u,3,of)
#define __SCALL_OP_3_4(t,if,sop,u,of) __SCALL_BASE(t,3,if,sop,u,4,of)

#define __SCALL_OP_4_1(t,if,sop,u,of) __SCALL_BASE(t,4,if,sop,u,1,of)
#define __SCALL_OP_4_2(t,if,sop,u,of) __SCALL_BASE(t,4,if,sop,u,2,of)
#define __SCALL_OP_4_3(t,if,sop,u,of) __SCALL_BASE(t,4,if,sop,u,3,of)
#define __SCALL_OP_4_4(t,if,sop,u,of) __SCALL_BASE(t,4,if,sop,u,4,of)

// The template for the whole AMI_scan().
#define __STEMPLATE(in_arity, out_arity)				    \
template< __STEMP_ ## in_arity (T), class SC, __STEMP_ ## out_arity (U) >   \
AMI_err AMI_scan( __SPARM_ ## in_arity (T,_ts_),			    \
                  SC *soper, __SPARM_ ## out_arity (U,_us_))		    \
{	    								    \
    __STSPACE_ ## in_arity (T,*_t_);					    \
    __STSPACE_ ## out_arity (U,_u_);					    \
	    								    \
    __FSPACE(_if_,in_arity);						    \
    __FSPACE(_of_,out_arity);						    \
	    								    \
    AMI_err _op_err_, _ami_err_;					    \
	    								    \
    do {	    							    \
	    								    \
        __STS_READ_ ## in_arity (_t_,_ts_,_if_,_ami_err_)		    \
            								    \
        _op_err_ = __SCALL_OP_ ## in_arity ## _ ##			    \
            out_arity(_t_,_if_,soper,_u_,_of_);				    \
	    								    \
        __STS_WRITE_ ## out_arity(_u_,_us_,_of_,_ami_err_)		    \
            								    \
    } while (_op_err_ != AMI_SCAN_CONTINUE);				    \
	    								    \
    if ((_ami_err_ != AMI_ERROR_NO_ERROR) &&				    \
        (_ami_err_ != AMI_ERROR_EOS)) {					    \
        return _ami_err_;						    \
    }	    								    \
    	    								    \
    return _op_err_;							    \
}

// Finally, the templates themsleves.
__STEMPLATE(1,1); __STEMPLATE(1,2); __STEMPLATE(1,3); __STEMPLATE(1,4);
__STEMPLATE(2,1); __STEMPLATE(2,2); __STEMPLATE(2,3); __STEMPLATE(2,4);
__STEMPLATE(3,1); __STEMPLATE(3,2); __STEMPLATE(3,3); __STEMPLATE(3,4);
__STEMPLATE(4,1); __STEMPLATE(4,2); __STEMPLATE(4,3); __STEMPLATE(4,4);

#endif // _AMI_SCAN_MAC_H 
