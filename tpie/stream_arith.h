#ifndef _TPIE_AMI_STREAM_ARITH_H
#define _TPIE_AMI_STREAM_ARITH_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
// Get the definition of the scan_object class.
#include <tpie/scan.h>

namespace tpie {

    namespace ami {
	
#define SCAN_OPERATOR_DECLARATION(NAME,OP)				\
									\
	template<class T> class scan_ ## NAME : scan_object {		\
	public:								\
	    err initialize(void);					\
	    err operate(const T &op1, const T &op2, SCAN_FLAG *sfin,	\
			T *res, SCAN_FLAG *sfout);			\
	};								\
									\
		template<class T>					\
		err scan_ ## NAME<T>::initialize(void)			\
		{							\
		    return NO_ERROR;					\
		}							\
									\
									\
		template<class T>					\
		err scan_ ## NAME<T>::operate(const T &op1, const T &op2, \
					      SCAN_FLAG *sfin,		\
					      T *res, SCAN_FLAG *sfout)	\
		{							\
		    if ((*sfout = (sfin[0] && sfin[1]))) {		\
			*res = op1 OP op2;				\
			return SCAN_CONTINUE;				\
		    } else {						\
			return SCAN_DONE;				\
		    }							\
		}
	
	SCAN_OPERATOR_DECLARATION(add,+)
	    SCAN_OPERATOR_DECLARATION(sub,-)
	    SCAN_OPERATOR_DECLARATION(mult,*)
	    SCAN_OPERATOR_DECLARATION(div,/)
	    
    }  //  ami namespace

}  // tpie namespace


namespace tpie {

    namespace ami {

#define SCAN_SCALAR_OPERATOR_DECLARATION(NAME,OP)			\
									\
	template<class T> class scan_scalar_ ## NAME : scan_object {	\
	private:							\
	    T scalar;							\
	public:								\
	    scan_scalar_ ## NAME(const T &s);			  	\
	    virtual ~scan_scalar_ ## NAME(void);			\
	    err initialize(void);					\
	    err operate(const T &op, SCAN_FLAG *sfin,			\
			T *res, SCAN_FLAG *sfout);			\
	};								\
									\
									\
		template<class T>					\
		scan_scalar_ ## NAME<T>::				\
		scan_scalar_ ## NAME(const T &s) :			\
		    scalar(s)						\
		{							\
		}							\
									\
									\
		template<class T>					\
		scan_scalar_ ## NAME<T>::~scan_scalar_ ## NAME()	\
		{							\
		}							\
									\
									\
		template<class T>					\
		err scan_scalar_ ## NAME<T>::initialize(void)		\
		{							\
		    return NO_ERROR;					\
		}							\
									\
									\
		template<class T>					\
		err scan_scalar_ ## NAME<T>::operate(const T &op,	\
						     SCAN_FLAG *sfin,	\
						     T *res, SCAN_FLAG *sfout) \
		{							\
		    if ((*sfout = *sfin)) {				\
			*res = op OP scalar;				\
			return SCAN_CONTINUE;				\
		    } else {						\
			return SCAN_DONE;				\
		    }							\
		}
	
	
	SCAN_SCALAR_OPERATOR_DECLARATION(add,+)
	    SCAN_SCALAR_OPERATOR_DECLARATION(sub,-)
	    SCAN_SCALAR_OPERATOR_DECLARATION(mult,*)
	    SCAN_SCALAR_OPERATOR_DECLARATION(div,/)

	    }  //  ami namespace

}  //  tpie namespace

#endif // _AMI_STREAM_ARITH_H 
