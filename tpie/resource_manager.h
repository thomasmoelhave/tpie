// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
//
// Copyright 2011, The TPIE development team
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

///////////////////////////////////////////////////////////////////////////
/// \file tpie/memory.h Memory management subsystem.
///////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_RESOURCE_MANAGER_H__
#define __TPIE_RESOURCE_MANAGER_H__

#include <tpie/config.h>
#include <tpie/util.h>
#include <tpie/resources.h>
#include <mutex>
#include <unordered_map>
#include <type_traits>
#include <utility>
#include <memory>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace tpie {

inline void segfault() {
	std::abort();
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Thrown when trying to allocate too much of a resource.
///
/// When the resource limit is exceeded and the resource limit enforcement policy
/// is set to THROW, this error is thrown by the resource subsystem.
///////////////////////////////////////////////////////////////////////////////
struct out_of_resource_error : public std::exception {
	std::string msg;
	out_of_resource_error(std::string s) : msg(std::move(s)) { }
	virtual const char* what() const throw() {return msg.c_str();}
};


///////////////////////////////////////////////////////////////////////////////
/// \brief Resource management object used to track resource usage.
///////////////////////////////////////////////////////////////////////////////
class resource_manager {
public:
	///////////////////////////////////////////////////////////////////////////
	/// Memory limit enforcement policies.
	///////////////////////////////////////////////////////////////////////////
	enum enforce_t {
		/** Ignore when running out of the resource. */
		ENFORCE_IGNORE,
		/** \brief Log to debug log when the resource limit is exceeded.
		 * Note that not all violations will be logged. */
		ENFORCE_DEBUG,
		/** \brief Log a warning when the resource limit is exceeded. Note that
		 * not all violations will be logged. */
		ENFORCE_WARN,
		/** Throw an out_of_resource_error when the resource limit is exceeded. */
		ENFORCE_THROW
	};

	///////////////////////////////////////////////////////////////////////////
	/// Return the current amount of the resource used.
	///////////////////////////////////////////////////////////////////////////
	size_t used() const throw();

	///////////////////////////////////////////////////////////////////////////
	/// Return the amount of the resource still available to be assigned.
	///////////////////////////////////////////////////////////////////////////
	size_t available() const throw();

	///////////////////////////////////////////////////////////////////////////
	/// Return the resource limit.
	///////////////////////////////////////////////////////////////////////////
	size_t limit() const throw() {return m_limit;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Update the resource limit.
	/// If the resource limit is exceeded by decreasing the limit,
	/// no exception will be thrown.
	/// \param new_limit The new resource limit.
	///////////////////////////////////////////////////////////////////////////
	void set_limit(size_t new_limit);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Set the resource limit enforcement policy.
	/// \param e The new enforcement policy.
	///////////////////////////////////////////////////////////////////////////
	void set_enforcement(enforce_t e);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the current resource limit enforcement policy.
	///////////////////////////////////////////////////////////////////////////
	enforce_t enforcement() {return m_enforce;}

	void register_increased_usage(size_t amount);

	void register_decreased_usage(size_t amount);

	virtual std::string amount_with_unit(size_t amount) const {
		std::ostringstream os;
		os << amount << " amount of " << resource_managed;
		return os.str();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \internal
	/// Construct the resource manager object.
	///////////////////////////////////////////////////////////////////////////
	resource_manager(resource_type type);

	virtual ~resource_manager() = default;

private:
	void print_resource_complaint(std::ostream & os, size_t amount, size_t usage);

protected:
	std::atomic<size_t> m_used;
	size_t m_limit;
	size_t m_maxExceeded;
	size_t m_nextWarning;
	enforce_t m_enforce;

	resource_type resource_managed;
};

} //namespace tpie

#endif //__TPIE_RESOURCE_MANAGER_H__
