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
char *tpie_mktemp(char *str);

/* like tempnam, but consults environment in an order we like; note
 * that the returned pointer is to static storage, so this function is
 * not re-entrant. */
char *tpie_tempnam(const char *base, const char* dir) {
  const char* base_dir;
  static char tmp_path[BUFSIZ];

  if (dir == NULL) {
		// get the dir
		base_dir = getenv(AMI_SINGLE_DEVICE_ENV);
		if (base_dir == NULL) {
			base_dir = getenv(TMPDIR_ENV);
			if (base_dir == NULL) {
				base_dir = TMP_DIR;
			}
		}
    sprintf(tmp_path, TPIE_OS_TEMPNAMESTR, base_dir, base);
  } else {
    sprintf(tmp_path, TPIE_OS_TEMPNAMESTR, dir, base);
  }

	return tpie_mktemp(tmp_path);    
}

char *tpie_mktemp(char *str) {
  const char chars[] = 
  { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
  const int chars_count = 62;
  static TPIE_OS_TIME_T counter = time(NULL) % (chars_count * chars_count); 
  TPIE_OS_SIZE_T pos = static_cast<TPIE_OS_SIZE_T>(strlen(str) - 6);

  str[pos++] = chars[counter/chars_count];
  str[pos++] = chars[counter%chars_count];

  str[pos++] = chars[TPIE_OS_RANDOM() % chars_count];
  str[pos++] = chars[TPIE_OS_RANDOM() % chars_count];
  str[pos++] = chars[TPIE_OS_RANDOM() % chars_count];
  str[pos]   = chars[TPIE_OS_RANDOM() % chars_count];

  counter = (counter + 1) % (chars_count * chars_count);
  return str;
}
