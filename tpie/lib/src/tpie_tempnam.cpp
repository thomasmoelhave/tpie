//
// File: tpie_tempnam.cpp
// Author: 
// Created: 02/02/02
//
#include <versions.h>
VERSION(tpie_tempnam_cpp,"$Id: tpie_tempnam.cpp,v 1.1 2002-02-02 18:08:38 tavi Exp $");

#include <stdio.h>
#include <stdlib.h>
#include "lib_config.h"
#include <tpie_tempnam.h>

/* like tempnam, but consults environment in an order we like note
 * that the returned pointer is to static storage, so this function is
 * not re-entrant. */
char *tpie_tempnam(char *base) {
  char *base_dir;
  static char tmp_path[BUFSIZ];
  char *path;

  // get the dir
  base_dir = getenv(AMI_SINGLE_DEVICE_ENV);
  if (base_dir == NULL) {
	base_dir = getenv(TMP_DIR_ENV);
	if (base_dir == NULL) {
	  base_dir = TMP_DIR;
	}
  }

  sprintf(tmp_path, "%s/%s_XXXXXX", base_dir, base);
  path = mktemp(tmp_path);
  if(!path) {
	LOG_FATAL_ID("could not mktemp");
	LOG_FATAL_ID(tmp_path);
	LOG_FATAL_ID(path);
	tp_assert(path, "No temporary path name returned.");
  }
  return path;
}
