// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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
#include <iostream>
#include <tpie/types.h>
#include <tpie/logstream.h>
#include <map>
#include <vector>
#include <boost/lexical_cast.hpp>

namespace tpie {

class teststream_buf: public std::basic_streambuf<char, std::char_traits<char> > {
private:
	const static size_t line_size = 2048;
	char m_line[line_size];
	bool m_new_line;
public:
	teststream_buf();
	virtual int overflow(int c = traits_type::eof());
	void stateAlign();
	virtual int sync();
};

class teststream: public std::ostream  {
private:
	teststream_buf m_buff;
	size_t failed;
	size_t total;
public:
	teststream();
	bool success();
	friend void result_manip(teststream & s, bool success);
};

template <class TP>
class testmanip {
	void (*_f)(teststream&, TP);
	TP _a;
public:
	inline testmanip(void (*f)(teststream&, TP), TP a) : _f(f), _a(a) {}
	inline friend std::ostream& operator<<(std::ostream& o, const testmanip<TP>& m) {
		(*m._f)(static_cast<teststream&>(o), m._a);
		return o;
	}
};


testmanip<bool> result(bool success);
testmanip<bool> success();
testmanip<bool> failure();

class tests {
public:
	tests(int argc, char ** argv, memory_size_type memory_limit=50);
	virtual ~tests();

	template <typename T>
	tests & setup(T t);

	template <typename T>
	tests & finish(T t);
	
	template <typename T>
	tests & test(T fct, const std::string & name);
	
	template <typename T, typename T1>
	tests & test(T fct, const std::string & name, const std::string & p1_name, T1 p1_default);

	template <typename T>
	tests & multi_test(T fct, const std::string & name);

	operator int();
protected:
	virtual void build_information(std::ostream & o);
private:
	void show_usage(std::ostream & o);

	struct func {
		virtual void operator()() = 0;
		virtual ~func() {}
	};

	template <typename T>
	struct func_impl: func {
		T t;
		func_impl(T t): t(t) {}
		virtual void operator()() {t();}
	};


	struct test_log_target: public log_target {
		std::stringstream output;
		std::stringstream error;
		
		void log(log_level level, const char * message, size_t) {
			output << message;
			if (level == LOG_FATAL || level == LOG_ERROR || level == LOG_WARNING) {
				error << message;
			}
		}
	};
	

	void start_test(const std::string & name);
	void end_test(bool result);

	template <typename T>
	T get_arg(const std::string & name, T def) const;

	bool bad, usage, version;
	std::string exe_name;
	std::string test_name;
	std::map<std::string, std::string> args;
	memory_size_type memory_limit;
	std::string current_name;
	std::vector<func *> setups;
	std::vector<func *> finishs;
	test_log_target log;
};

template <typename T>
struct get_arg_help {
	T operator()(const std::map<std::string, std::string> & args, const std::string & name, T def) {
		T arg=def;
		try {
			std::map<std::string, std::string>::const_iterator i=args.find(name);
			if (i != args.end()) arg=boost::lexical_cast<T>(i->second);
		} catch (std::bad_cast) {
			std::cerr << "The argument " << name << " has the wrong type" << std::endl;
		}
		return arg;
	}
};

template <>
struct get_arg_help<bool> {
	bool operator()(const std::map<std::string, std::string> & args, const std::string & name, bool) {
		return args.count(name) != 0;
	}
};	

template <typename T>
T tests::get_arg(const std::string & name, T def) const {
	return get_arg_help<T>()(args, name, def);
}

template <typename T>
tests & tests::test(T fct, const std::string & name) {
	if (name == test_name || test_name == "all") {
		start_test(name);
		end_test( fct());
	}
	return *this;
}

template <typename T, typename T1>
tests & tests::test(T fct, const std::string & name, const std::string & p1_name, T1 p1_default) {
	if (name == test_name || test_name == "all") {
		start_test(name);
		end_test(fct(get_arg(p1_name, p1_default)));
	}
	return *this;
}

template <typename T>
tests & tests::multi_test(T fct, const std::string & name) {
	if (name == test_name || test_name == "all") {
		start_test(name);
		teststream ts;
		fct(ts);
		end_test(ts.success()); 
	}
	return *this;
}


template <typename T>
tests & tests::setup(T t) {
	setups.push_back(new func_impl<T>(t));
	return *this;
}

template <typename T>
tests & tests::finish(T t) {
	finishs.push_back(new func_impl<T>(t));
	return *this;
}

} //namespace tpie
