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
#include <iomanip>
#include <vector>
#include <cmath>

namespace tpie {
namespace test {

class stat {
	static const int width = 16;

public:
	inline stat(const std::vector<const char *> & name) {
		mean.resize(name.size(), 0);
		m2.resize(name.size(), 0);
		n = 0;
		next_col = 0;
		std::cout << std::setw(width) << "Test";
		for(size_t i=0; i < name.size(); ++i) 
			std::cout << std::setw(width) << name[i];
		std::cout << std::endl;
	}

	template <typename T>
	inline void operator()(const std::vector<T> & times) {
		for (size_t i = 0; i < times.size(); ++i) {
			(*this)(times[i]);
		}
	}

	template <typename T>
	inline void operator()(const T & time) {
		if (next_col == 0) {
			++n;
			std::cout << std::setw(width) << n;
		}
		std::cout << std::setw(width) << time << std::flush;

		double delta = time - mean[next_col];
		mean[next_col] += static_cast<double>(delta) / n;
		m2[next_col] = m2[next_col] + delta*(time - mean[next_col]);

		++next_col;
		if (next_col == mean.size()) {
			next_col = 0;
			std::cout << '\n';
			print_mean_line();
			std::cout << '\r' << std::flush;
		}
	}

	inline ~stat() {
		print_mean_line();
		std::cout << std::endl << std::setw(width) << "stddev";
		for(size_t i=0; i < mean.size(); ++i) 
			std::cout << std::setw(width) << sqrt(m2[i]/(n-1));
		std::cout << std::endl;
	}

private:
	void print_mean_line() {
		std::cout << std::setw(width) << "mean";
		for(size_t i=0; i < mean.size(); ++i) 
			std::cout << std::setw(width) << mean[i];
	}

	std::vector<double> mean;
	std::vector<double> m2;
	size_t n;
	size_t next_col;
};

} // namespace test
} // namespace tpie
