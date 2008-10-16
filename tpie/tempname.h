//
// File: tpie_tempnam.h
// Author: 
// Created: 02/02/02
//
// $Id: tpie_tempnam.h,v 1.4 2004-04-16 21:33:29 adanner Exp $
//
//
#ifndef _TPIE_TEMPNAM_H
#define _TPIE_TEMPNAM_H

// Get definitions for working with Unix and Windows
#include <portability.h>

// The name of the environment variable pointing to a tmp directory.
#define TMPDIR_ENV "TMPDIR"

// The name of the environment variable to consult for default device
// descriptions.
#define AMI_SINGLE_DEVICE_ENV "AMI_SINGLE_DEVICE"

char *tpie_tempnam(const char *post_base, const char *base = NULL, const char *dir = NULL, const char* ext = NULL);

void set_default_tmp_names(char* path, char* base, char* extension);

void set_default_tmp_path(char* path);
void set_default_base_name(char* name);
void set_default_extension(char* ext);

char *get_default_tmp_path();
char *get_default_base_name();
char *get_default_extension();

#endif // _TPIE_TEMPNAM_H 
