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

std::string tpie_tempnam(const std::string& post_base, 
						 const std::string& base = std::string(), 
						 const std::string& dir = std::string(), 
						 const std::string& ext = std::string());

void set_default_tmp_names(const std::string& path, const std::string& base, const std::string& extension);

void set_default_tmp_path(const std::string& path);
void set_default_base_name(const std::string& name);
void set_default_extension(const std::string& ext);

std::string& get_default_tmp_path();
std::string& get_default_base_name();
std::string& get_default_extension();

#endif // _TPIE_TEMPNAM_H 
