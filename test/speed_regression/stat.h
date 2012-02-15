// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2005-2009, The TPIE development team
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


namespace tpie {
  namespace test {
  
	  class stat {
		  static const int width = 16;
	  public:
		  inline stat(const std::vector<const char *> & name) {
			  mean.resize(name.size(), 0);
			  m2.resize(name.size(), 0);
			  n = 0;
			  std::cout << std::setw(width) << "Test";
			  for(size_t i=0; i < name.size(); ++i) 
				  std::cout << std::setw(width) << name[i];
			  std::cout << std::endl;
		  }
		  
		  template <typename T>
		  inline void operator()(const std::vector<T> & times) {
			  ++n;
			  std::cout << "\r" << std::setw(width) << n;
			  for(size_t i=0; i < times.size(); ++i) {
				  double delta = times[i] - mean[i];
				  mean[i] += static_cast<double>(delta) / n;
				  m2[i] = m2[i] + delta*(times[i] - mean[i]);
				  std::cout << std::setw(width) <<  times[i];
			  }
			  std::cout << std::endl << std::setw(width) << "mean";
			  for(size_t i=0; i < times.size(); ++i) 
				  std::cout << std::setw(width) << mean[i];
			  std::cout << std::flush;
		  }

		  inline ~stat() {
			  std::cout << std::endl << std::setw(width) << "stddev";
			  for(size_t i=0; i < mean.size(); ++i) 
					  std::cout << std::setw(width) << sqrt(m2[i]/(n-1));
			  std::cout << std::endl;
		  }
	  private:
		  std::vector<double> mean;
		  std::vector<double> m2;
		  size_t n;
	  };
	  
  }
}
