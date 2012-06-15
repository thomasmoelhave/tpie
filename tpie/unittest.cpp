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

#include <tpie/unittest.h>
#include <tpie/tpie.h>
#include <tpie/tpie_log.h>
#include <tpie/memory.h>
#include <tpie/tpie_log.h>
#include <sstream>

namespace tpie {

tests::tests(int argc, char ** argv, memory_size_type memory_limit): memory_limit(memory_limit) {
	exe_name = argv[0];
	bool has_seen_test=false;
	bad=false;
	usage=false;
	version=false;
	for (int i=1; i < argc; ++i) {
		if (argv[i][0] != '-') {
			if (has_seen_test) {
				std::cerr << "More then one test was supplied" << std::endl;
				usage=true;
				bad=true;
				break;
			}
			test_name = argv[i];
			has_seen_test=true;
			continue;
		}

		if (argv[i][1] == 'v') {
			version=true;
			break;
		}
		
		if (argv[i][1] == 'h') {
			usage=true;
			break;
		}
		
		if (argv[i][1] != '-') {
			usage=true;
			bad=true;
			std::cerr << "Unknow switch " << argv[i] << std::endl;
			break;
		}

		if (std::string("--help") == argv[i]) {
			usage=true;
			break;
		}

		if (std::string("--version") == argv[i]) {
			version=true;
			break;
		}

		if (i + 1 < argc && argv[i+1][0] != '-') {
			args[argv[i]+2] = argv[i+1];
			++i;
		} else
			args[argv[i]+2] = "";
	}

	if (!usage && !version && !has_seen_test) {
		std::cerr << "No test supplied" << std::endl;
		usage=true;
		bad=true;
	}
	tpie::tpie_init(tpie::ALL -tpie::DEFAULT_LOGGING);
	get_log().add_target(&log);
	tpie::get_memory_manager().set_limit(get_arg("memory", memory_limit)*1024*1024);
}

tests::~tests() {
	get_log().remove_target(&log);
	tpie::tpie_finish();
}

void tests::start_test(const std::string & name) {
	log.output.str("");
	log.error.str("");
	for (size_t i=0; i < setups.size(); ++i)
		(*setups[i])();

	current_name=name;
	std::cout << name << ' ';
	for (size_t i=name.size()+1; i < 79-6; ++i)
		std::cout << '.';
	std::cout << " [    ]\r";
	std::cout << std::flush;
}

void tests::end_test(bool result) {
	//Erace last line
	std::cout << '\r';
	for (size_t i=0; i < 79; ++i)
		std::cout << ' ';
	std::cout << '\r';
	
	std::cout << log.output.str();	
	
	//Print status line
	std::cout << current_name << ' ';
	for (size_t i= current_name.size()+1; i < 79-6; ++i)
		std::cout << '.';
	if (result)
		std::cout << " [ ok ]" << std::endl;
	else
		std::cout << " [fail]" << std::endl;

	for (size_t i=0; i < finishs.size(); ++i)
		(*finishs[i])();

	if (!result) bad=true;
}

void tests::build_information(std::ostream & o) {
	o << "TODO sysinfo" << std::endl;
}	

void tests::show_usage(std::ostream & o) {
	o << "Usage: " << exe_name << " [TEST_NAME] [OPTION]..." << std::endl;
	o << "Run unit test" << std::endl;
	o << "  -h, --help              Show this help message" << std::endl;
	o << "  -v, --version           Show version information" << std::endl;
	o << "      --memory SIZE       Change the maximal amout of memory allowed to be used (default: " 
	  << memory_limit << ")" << std::endl;
}

tests::operator int() {
	if (usage) {
		if (bad) {
			std::cerr << std::endl;
			show_usage(std::cerr);
		} else
			show_usage(std::cout);
	}

	if (version) {
		build_information(std::cout);
	}

	if (bad) return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

} //namespace tpie
