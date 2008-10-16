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
#include <tempname.h>
#include <string>

// Defined below.
char* tpie_mktemp(char* str, const char* extension);

//tmp file globals
char* default_path = NULL; 
char* default_base_name = NULL; 
char* default_extension = NULL;

/* like tempnam, but consults environment in an order we like; note
 * that the returned pointer is to static storage, so this function is
 * not re-entrant. */
char *tpie_tempnam(const char *base, const char* dir, const char* ext) {
	const char* base_dir;
	const char* extension;
	const char* basename;
	static char tmp_path[BUFSIZ];

	extension = ext;
	if(extension == NULL) {
		extension = default_extension;
		if(extension == NULL)
			extension = "tpie";
	}

	basename = base;
	if(basename == NULL) {
		basename = default_base_name;
		if(basename == NULL)
			basename == "AMI";
	}

	base_dir = dir;
	if (base_dir == NULL) {
		base_dir = getenv(AMI_SINGLE_DEVICE_ENV);
		if (base_dir == NULL) {
			base_dir = getenv(TMPDIR_ENV);
			if (base_dir == NULL) {
				base_dir = default_path;
				if(base_dir == NULL) {
					base_dir = TMP_DIR;
				}
			}
		}
	}
	
	std::cout << base_dir << " " << basename << " " << extension << std::endl;
	sprintf(tmp_path, TPIE_OS_TEMPNAMESTR, base_dir, basename, extension);
	std::cout << tmp_path << std::endl;

	return tpie_mktemp(tmp_path, extension);    
}

char *tpie_mktemp(char* str, const char* extension) {
	const int random_string_length = 7;
	const char chars[] = 
	{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	const int chars_count = 62;
	static TPIE_OS_TIME_T counter = time(NULL) % (chars_count * chars_count); 
	TPIE_OS_SIZE_T pos = static_cast<TPIE_OS_SIZE_T>(strlen(str) - strlen(extension) - random_string_length);

	str[pos++] = chars[counter/chars_count];
	str[pos++] = chars[counter%chars_count];

	str[pos++] = chars[TPIE_OS_RANDOM() % chars_count];
	str[pos++] = chars[TPIE_OS_RANDOM() % chars_count];
	str[pos++] = chars[TPIE_OS_RANDOM() % chars_count];
	str[pos]   = chars[TPIE_OS_RANDOM() % chars_count];

	counter = (counter + 1) % (chars_count * chars_count);
	return str;
}

void set_default_tmp_names(char* path, char* base, char* extension) {
	default_path = path;
	default_base_name = base;
	default_extension = extension;
}

char *get_default_tmp_path() {
	return default_path;
}

char *get_default_base_name() {
	return default_base_name;
}

char *get_default_extension() {
	return default_base_name;
}
