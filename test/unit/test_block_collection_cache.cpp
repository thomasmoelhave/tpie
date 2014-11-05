// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino=(0 :
// Copyright 2014, The TPIE development team
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

// block_collection_cache usage test

#include <tpie/tpie.h>
#include "block_collection.h"

bool collection_basic() {
	temp_file file;
	block_collection_cache collection(file.path(), true, max_block_size * 5);
	return basic(collection);
}

bool collection_erase() {
	temp_file file;
	block_collection_cache collection(file.path(), true, max_block_size * 5);
	return erase(collection);
}

bool collection_overwrite() {
	temp_file file;
	block_collection_cache collection(file.path(), true, max_block_size * 5);
	return overwrite(collection);
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(collection_basic, "basic")
		.test(collection_erase, "erase")
		.test(collection_overwrite, "overwrite");
}
