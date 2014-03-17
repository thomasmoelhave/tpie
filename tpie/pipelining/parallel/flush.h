// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014, The TPIE development team
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

#ifndef TPIE_PIPELINING_PARALLEL_FLUSH_H
#define TPIE_PIPELINING_PARALLEL_FLUSH_H

namespace tpie {

namespace pipelining {

namespace parallel_bits {

// The following is derived from the Stack Overflow answer by joshperry:
// http://stackoverflow.com/a/1967183
template <typename Type>
class has_flush {
private:
	class yes { char m; };
	class no { yes m[2]; };
	struct BaseMixin {
		void flush() {}
	};
	struct Base : public Type, public BaseMixin {};
	template <typename T, T t>  class Helper {};
	template <typename U>
	static no deduce(U*, Helper<void (BaseMixin::*)(), &U::flush>* = 0);
	static yes deduce(...);
public:
	static const bool result = sizeof(yes) == sizeof(deduce((Base*)(0)));
};

///////////////////////////////////////////////////////////////////////////////
/// Helper class used to call flush() on pipelines specifying maintain_order.
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t, bool hasFlush=has_flush<dest_t>::result>
struct flush_help;

///////////////////////////////////////////////////////////////////////////////
/// Specialization of flush_help for classes NOT defining flush().
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
struct flush_help<dest_t, false> {
	static void maybe_flush(dest_t &, bool) {}

	static void check_flush(bool maintainOrder) {
		if (maintainOrder) {
			// User wants to maintain order, but has NOT defined flush().
			std::stringstream err;
			err << "maintain_order was specified, but node "
				<< pipelining::bits::extract_pipe_class_name(typeid(dest_t).name())
				<< " does not implement flush()";
			throw invalid_argument_exception(err.str());
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
/// Specialization of flush_help for classes defining flush().
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
struct flush_help<dest_t, true> {
	static void maybe_flush(dest_t & dest, bool maintainOrder) {
		if (maintainOrder)
			dest.flush();
	}

	static void check_flush(bool maintainOrder) {
		if (maintainOrder) {
			// User wants to maintain order, and has defined flush().
		}
	}
};

} // namespace parallel_bits

} // namespace pipelining

} // namespace tpie

#endif // TPIE_PIPELINING_PARALLEL_FLUSH_H
