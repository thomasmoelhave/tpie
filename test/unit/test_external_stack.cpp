// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
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
#include "../app_config.h"
#include <tpie/portability.h>
#include <tpie/tpie.h>
#include <tpie/util.h>
#include <tpie/stack.h>
#include <tpie/stream.h>
#include <tpie/prime.h>
#include "common.h"

using namespace tpie;

bool stack_test(size_t size) {
  ami::stack<size_t> s;
  size_t i=1234;
  for(size_t _=0; _ < size; ++_) {
    s.push(i) ;
    ++i;
    if (s.size() != _ +1) {
      std::cerr << "size failed" << std::endl;
      return false;
    }
  }
  size_t o=i-1;
  for(size_t _=0; _ < size; ++_) {
    s.push(i) ;
    const size_t * x;
    s.pop(&x);
    if (*x != i) {
      std::cerr << "Wrong element" << std::endl;
      return false;
    }
    ++i;
    
    if (s.size() != size) {
      std::cerr << "size failed 2" << std::endl;
      return false;
    }
  }

  for(size_t _=0; _ < size; ++_) {
    const size_t * x;
    s.pop(&x);
    if (*x != o) {
      std::cerr << "Wrong element 2" << std::endl;
      return false;
    }
    --o;
  }
  
  if (s.size() != 0) return false;
  return true;
}


bool perform_test(const std::string & test) {
  if (test == "small")
    return stack_test(1024 * 1024 * 3);
  else if (test == "large")
    return stack_test(1024*1024*1024);
  return false;
}

int main(int argc, char **argv) {
  tpie_initer _;
  if (argc != 2) return 1;
  bool ok=perform_test(std::string(argv[1]));
  return ok?EXIT_SUCCESS:EXIT_FAILURE;
}
