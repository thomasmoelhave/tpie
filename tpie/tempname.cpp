//
// File: tpie_tempnam.cpp
// Author: 
// Created: 02/02/02
//

#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <cstring>
#include "lib_config.h"
#include <tpie/tempname.h>
#include <string>

// Defined below.
std::string tpie_mktemp();

//tmp file globals
std::string default_path;
std::string default_base_name; 
std::string default_extension;

/* like tempnam, but consults environment in an order we like; note
 * that the returned pointer is to static storage, so this function is
 * not re-entrant. */
std::string tpie_tempnam(const std::string& post_base, const std::string& dir, const std::string& ext) 
{	std::string extension;
	std::string base_name;	
	std::string base_dir;
	
	extension = ext;
	if(extension.empty()) { 
		extension = default_extension;
		if(extension.empty())
			extension = "tpie";
	}
	
	base_name = default_base_name;
	if(base_name.empty())
		base_name = "TPIE";
	
	if(getenv(AMI_SINGLE_DEVICE_ENV) != NULL) 
		base_dir = getenv(AMI_SINGLE_DEVICE_ENV);
	else if(getenv(TMPDIR_ENV) != NULL) 
		base_dir = getenv(TMPDIR_ENV);
	else if(!dir.empty())
		base_dir = dir;
	else if(!default_path.empty())
		base_dir = default_path;
	else 
		base_dir = TMP_DIR;

	if(post_base.empty())
		return base_dir + TPIE_OS_DIR_DELIMITER + base_name + "_" + tpie_mktemp() + "." + extension;
	else 
		return base_dir + TPIE_OS_DIR_DELIMITER + base_name + "_" + post_base + "_" + tpie_mktemp() + "." + extension;
}

std::string tpie_mktemp()
{
	const std::string chars[] = 
	{ "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", 
	"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", 
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", 
	"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", 
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
	const int chars_count = 62;
	static TPIE_OS_TIME_T counter = time(NULL) % (chars_count * chars_count); 

	std::string result = "";
	result +=
		chars[counter/chars_count] +
		chars[counter%chars_count] +
		chars[TPIE_OS_RANDOM() % chars_count] +
		chars[TPIE_OS_RANDOM() % chars_count] +
		chars[TPIE_OS_RANDOM() % chars_count] +
		chars[TPIE_OS_RANDOM() % chars_count];

	counter = (counter + 1) % (chars_count * chars_count);

	return result;
}


void set_default_path(const std::string&  path) {
	default_path = path;
}

void set_default_base_name(const std::string& name) {
	default_base_name = name;
}

void set_default_extension(const std::string& ext) {
	default_extension = ext;
}


std::string& get_default_tmp_path() {
	return default_path;
}

std::string& get_default_base_name() {
	return default_base_name;
}

std::string& get_default_extension() {
	return default_extension;
}
