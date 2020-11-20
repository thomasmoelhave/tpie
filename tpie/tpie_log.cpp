// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2011 The TPIE development team
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

// We are logging
#define TPL_LOGGING	1

#include <cstdlib>
#include <time.h>
#include <tpie/tempname.h>
#include <tpie/logstream.h>
#include <tpie/tpie_log.h>
#include <iostream>


namespace tpie {

namespace {
	std::vector<log_target *> log_targets;
	bool logging_enabled = true;
	file_log_target * file_target = 0;
	stderr_log_target * stderr_target = 0;
}

void add_log_target(log_target * t) {
	log_targets.push_back(t);
}

void remove_log_target(log_target * t) {
	auto i = std::find(log_targets.begin(), log_targets.end(), t);
	if (i != log_targets.end())
		log_targets.erase(i);
}

void begin_log_group(std::string_view name) {
	for (auto t: log_targets)
		t->begin_group(name);
}

void end_log_group() {
	for (auto t: log_targets)
		t->end_group();
}
 
void log_to_targets(log_level level, std::string_view message) {
	if (!logging_enabled) return;
	//TODO buffer message if there are no targets
	for (auto t: log_targets)
		t->log(level, message);
}

bool get_log_enabled() {
	return logging_enabled;
}

void set_log_enabled(bool enabled) {
	logging_enabled = enabled;
}


file_log_target::file_log_target(log_level threshold): m_threshold(threshold) {
	m_path = tempname::tpie_name("log", "" , "txt");
	m_out.open(m_path.c_str(), std::ios::trunc | std::ios::out);
}

void file_log_target::log(log_level level, std::string_view message) {
	if (level > m_threshold) return;

	if(LOG_DEBUG > level) { // print without indentation
		m_out << message;
		m_out.flush();
		return;
	}

	m_out << build_prefix(groups.size()) << " " << message;
	m_out.flush();
}

std::string file_log_target::build_prefix(size_t size) {
	return std::string(size, '|');
}

void file_log_target::begin_group(std::string_view name) {
	if(LOG_DEBUG > m_threshold) return;

	groups.push(std::string(name));

	m_out << build_prefix(groups.size()-1) << "> " << "Entering " << name << std::endl;
}

void file_log_target::end_group() {
	if(LOG_DEBUG > m_threshold) return;

	m_out << build_prefix(groups.size()-1) << "x " << "Leaving " << groups.top() << std::endl;
	groups.pop();
}

stderr_log_target::stderr_log_target(log_level threshold): m_threshold(threshold) {}

std::string stderr_log_target::build_prefix(size_t size) {
	std::string prefix;
	for(size_t i = 0; i < size; ++i) prefix += "|";
	return prefix;
}

void stderr_log_target::log(log_level level, std::string_view message) {
	if (level > m_threshold) return;

	if(LOG_DEBUG > level) { // print without indentation
		fwrite(message.data(), 1, message.size(), stderr);
		return;
	}

	std::string prefix = build_prefix(groups.size()) + " ";

	fwrite(prefix.c_str(), 1, prefix.size(), stderr);
	fwrite(message.data(), 1, message.size(), stderr);

}	

void stderr_log_target::begin_group(std::string_view name) {
	if(LOG_DEBUG > m_threshold) return;

	groups.push(std::string(name));
	
	std::string prefix = build_prefix(groups.size()-1) + "> ";
	std::string text = "Entering " + std::string(name) + "\n";

	fwrite(prefix.c_str(), sizeof(char), prefix.size(), stderr);
	fwrite(text.c_str(), sizeof(char), text.size(), stderr);
}

void stderr_log_target::end_group() {
	if(LOG_DEBUG > m_threshold) return;

	std::string text = "Leaving " + groups.top() + "\n";
	std::string prefix = build_prefix(groups.size()-1) + "x ";

	groups.pop();

	fwrite(prefix.c_str(), sizeof(char), prefix.size(), stderr);
	fwrite(text.c_str(), sizeof(char), text.size(), stderr);
}




const std::string& log_name() {
	return file_target->m_path;
}

void init_default_log() {
	if (file_target) return;
	file_target = new file_log_target(LOG_DEBUG);
	stderr_target = new stderr_log_target(LOG_INFORMATIONAL);
	add_log_target(file_target);
	add_log_target(stderr_target);
}

void finish_default_log() {
	if (!file_target) return;
	remove_log_target(file_target);
	remove_log_target(stderr_target);
	delete file_target;
	delete stderr_target;
	file_target = 0;
	stderr_target = 0;
}

} //namespace tpie
