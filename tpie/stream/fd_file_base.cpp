#include <tpie/stream/fd_file_base.inl>
#include <tpie/stream/posix_bte.h>
using namespace tpie::stream;

#ifdef WIN32

#else
template
class fd_file_base<posix_block_transfer_engine>;
#endif
