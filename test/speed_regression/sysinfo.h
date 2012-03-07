// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#ifndef __TPIE_SYSINFO__
#define __TPIE_SYSINFO__

#include <iostream>
#include <iomanip>
#include <boost/asio/ip/host_name.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace tpie {

extern const char * git_commit;
extern const char * git_refspec;

struct sysinfo {
	inline sysinfo()
		: m_platform(calc_platform())
		, m_hostname(boost::asio::ip::host_name())
		, m_blocksize(calc_blocksize())
	{

	}

	inline std::string commit()    const { return git_commit; }
	inline std::string refspec()   const { return git_refspec; }
	inline std::string platform()  const { return m_platform; }
	inline std::string hostname()  const { return m_hostname; }
	inline std::string blocksize() const { return m_blocksize; }
	inline std::string localtime() const {
		boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
		return to_simple_string(now);
	}

	template <typename V>
	inline void printinfo(std::string key, const V & value) {
		if (key != "") key += ':';
		std::cout.flags(std::ios::left);
		std::cout << std::setw(16) << key << value << std::endl;
	}

private:
	static const char * m_commit;
	static const char * m_refspec;
	const std::string m_platform;
	const std::string m_hostname;
	const std::string m_blocksize;

	static inline std::string calc_platform() {
		std::stringstream p;
#ifdef WIN32
		p << "Windows ";
#else
		p << "Linux ";
#endif
		p << (8*sizeof(size_t)) << "-bit";
		return p.str();
	}

	static inline std::string calc_blocksize() {
		std::stringstream ss;
#ifdef WIN32
		ss << STREAM_UFS_BLOCK_FACTOR*64;
#else
		ss << STREAM_UFS_BLOCK_FACTOR*4;
#endif
		ss << " KiB";
		return ss.str();
	}
};

inline std::ostream & operator<<(std::ostream & s, const sysinfo & info) {
	return s
		<< "Hostname:       " << info.hostname() << '\n'
		<< "Platform:       " << info.platform() << '\n'
		<< "Git branch:     " << info.refspec() << '\n'
		<< "Git commit:     " << info.commit() << '\n'
		<< "Local time:     " << info.localtime() << '\n'
		<< "Block size:     " << info.blocksize() << '\n';
}

}

#endif // __TPIE_SYSINFO__
