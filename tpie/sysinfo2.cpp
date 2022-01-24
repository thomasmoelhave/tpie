#include <tpie/sysinfo.h>
#include <tpie/file.h> // for block size
#include <tpie/tpie_log.h>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <ctime>
#ifdef WIN32
#include <winsock.h>
#else
#include <unistd.h>
#endif
namespace tpie {

namespace {

template <typename V>
std::string custominfo_(std::string key, const V & value) {
    std::stringstream builder;
    if (key != "") key += ':';
    builder.flags(std::ios::left);
    builder << std::setw(16) << key << value;
    return builder.str();
}


template <typename V>
void printinfo_(std::string key, const V & value) {
    std::cout << custominfo_(key, value) << std::endl;
}

}

std::string sysinfo::calc_platform() {
    std::stringstream p;
#ifdef WIN32
    p << "Windows ";
#else
    p << "Linux ";
#endif
    p << (8*sizeof(size_t)) << "-bit";
    return p.str();
}

std::string sysinfo::calc_hostname() {
    char name[512];
    if (gethostname(name, 511) != 0) {
        log_debug() << "Failed to get hostname" << std::endl;
        return "Exception";
    }
    return name;
}

std::string sysinfo::calc_blocksize() {
    std::stringstream ss;
    ss << blocksize_bytes() / 1024
        << " KiB";
    return ss.str();
}

std::string sysinfo::localtime() const {
    const auto now = std::chrono::system_clock::now();
    const auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream output_stream;
    auto time_info = std::localtime(&in_time_t);
    if(time_info != nullptr) return "Error";
    output_stream << std::put_time(time_info, "%Y-%m-%d.%H_%M_%S");
    return output_stream.str();
}

memory_size_type sysinfo::blocksize_bytes() {
    return get_block_size();
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Report default system info to the specified \c ostream.
/// \sa sysinfo::printinfo
///////////////////////////////////////////////////////////////////////////////
std::ostream & operator<<(std::ostream & s, const sysinfo & info) {
	return s
		<< "Hostname:       " << info.hostname() << '\n'
		<< "Platform:       " << info.platform() << '\n'
		<< "Git branch:     " << info.refspec() << '\n'
		<< "Git commit:     " << info.commit() << '\n'
		<< "Local time:     " << info.localtime() << '\n'
		<< "Block size:     " << info.blocksize() << '\n'
		<< "Parallel sort:  "
#ifdef TPIE_PARALLEL_SORT
		<< "Enabled"
#else
		<< "Disabled"
#endif
		<< '\n'
		<< "Snappy:         "
#ifdef TPIE_HAS_SNAPPY
		<< "Enabled"
#else
		<< "Disabled"
#endif
		<< '\n'
		;
}

std::string sysinfo::custominfo(std::string key, long value) {
    return custominfo_(key, value);
}
std::string sysinfo::custominfo(std::string key, const std::string & value) {
    return custominfo_(key, value);
}
std::string sysinfo::custominfo(std::string key, const char * value) {
    return custominfo_(key, value);
}

///////////////////////////////////////////////////////////////////////////
/// \brief Print custom info to std::cout.
///////////////////////////////////////////////////////////////////////////
void sysinfo::printinfo(std::string key, long value) {
    printinfo_(key, value);
}
void sysinfo::printinfo(std::string key, const std::string & value)  {
    printinfo_(key, value);
}

void sysinfo::printinfo(std::string key, const char * value) {
    printinfo_(key, value);
}


} // namespace tpie
