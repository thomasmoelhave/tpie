//
// File: tpie_tempnam.cpp
// Author: 
// Created: 02/02/02
//
#include <versions.h>
VERSION(tpie_tempnam_cpp,"$Id: tpie_tempnam.cpp,v 1.2 2002-07-20 21:31:10 tavi Exp $");

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib_config.h"
#include <tpie_tempnam.h>

// Defined below.
char *tpie_mktemp(char *str);

/* like tempnam, but consults environment in an order we like; note
 * that the returned pointer is to static storage, so this function is
 * not re-entrant. */
char *tpie_tempnam(char *base, const char* dir) {
  char *base_dir;
  static char tmp_path[BUFSIZ];
  char *path;

  if (dir == NULL) {
    // get the dir
    base_dir = getenv(AMI_SINGLE_DEVICE_ENV);
    if (base_dir == NULL) {
      base_dir = getenv(TMP_DIR_ENV);
      if (base_dir == NULL) {
	base_dir = TMP_DIR;
      }
    }
    sprintf(tmp_path, "%s/%s_XXXXXX", base_dir, base);
  } else {
    sprintf(tmp_path, "%s/%s_XXXXXX", dir, base);
  }

  path = tpie_mktemp(tmp_path);    
  return path;
}

char *tpie_mktemp(char *str) {
  const char chars[] = 
  { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
  const int chars_count = 62;
  static int counter = time(NULL) % (chars_count * chars_count); 
  int pos = strlen(str) - 6;

  str[pos++] = chars[counter/chars_count];
  str[pos++] = chars[counter%chars_count];
  str[pos++] = chars[random()%chars_count];
  str[pos++] = chars[random()%chars_count];
  str[pos++] = chars[random()%chars_count];
  str[pos] = chars[random()%chars_count];
  counter = (counter + 1) % (chars_count * chars_count);
  return str;
}
