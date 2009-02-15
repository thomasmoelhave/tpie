#ifndef _AMI_STREAM_COMPATIBILITY_H
#define _AMI_STREAM_COMPATIBILITY_H

#include <tpie/portability.h>

#if defined(AMI_IMP_SINGLE)
	TPIE_OS_UNIX_ONLY_WARNING_AMI_IMP_SINGLE
#else
#  define AMI_STREAM_IMP_SINGLE
#endif

#if defined(AMI_STREAM_IMP_USER_DEFINED)
#  warning The AMI_STREAM_IMP_USER_DEFINED flag is obsolete. 
#  warning User-defined AMIs are not supported anymore. 
#  warning Please contact the TPIE Project.
#endif // AMI_STREAM_IMP_USER_DEFINED

#if defined(AMI_STREAM_IMP_MULTI_IMP)
#  warning The AMI_STREAM_IMP_MULTI_IMP flag is obsolete. 
#  warning The usage of multiple AMIs is not supported anymore. 
#  warning Please contact the TPIE Project.
#endif // AMI_STREAM_IMP_MULTI_IMP

#define AMI_STREAM tpie::ami::stream

#define AMI_stream_base tpie::ami::stream
#define AMI_stream_single tpie::ami::stream

#endif // _AMI_STREAM_COMPATIBILITY_H
