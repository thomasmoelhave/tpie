// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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

#include <random>
#include <tpie/tpie.h>
#include <tpie/hash.h>

using namespace tpie::hash_bits;

namespace tpie {

namespace hash_bits {

size_t hash_codes[sizeof(size_t)][256];

} // namespace hash_bits

void init_hash() {
	std::mt19937 rng(9001);
	std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

	for(size_t i = 0; i < sizeof(size_t); ++i)
		for(size_t j = 0; j < 256; ++j)
			hash_codes[i][j] = dist(rng);
}


BufferedHash::BufferedHash(size_t seed) {
	value = seed;
	cur = buff;
	stop = buff + 1024;
}

void BufferedHash::copy(const BufferedHash & o) {
	value = o.value;
	memcpy(buff, o.buff, o.cur - o.buff);
	cur = buff + (o.cur - o.buff);
	stop = buff + 1024;
}

void BufferedHash::flush() {
	// murmur hash
	const size_t len = cur - buff;
	if (len < 4) return;

	const uint32_t* key_x4 = (const uint32_t*) buff;

	size_t i = len >> 2;
	auto h = value;
	do {
		uint32_t k = *key_x4++;
		k *= 0xcc9e2d51;
		k = (k << 15) | (k >> 17);
		k *= 0x1b873593;
		h ^= k;
		h = (h << 13) | (h >> 19);
		h += (h << 2) + 0xe6546b64;
	} while (--i);
	value = h;


	auto start = buff + ((len >> 2) << 2);
	memcpy(buff, start, cur - start);
	cur = buff + (cur - start);
}

uint32_t BufferedHash::finalize() {
	// murmur hash
	auto h = value;
	const size_t len = cur - buff;
	if (len & 3) {
		size_t i = len & 3;
		uint32_t k = 0;
		auto key = &buff[i - 1];
		do {
			k <<= 8;
			k |= *key--;
		} while (--i);
		k *= 0xcc9e2d51;
		k = (k << 15) | (k >> 17);
		k *= 0x1b873593;
		h ^= k;
	}
	h ^= len;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

} // namespace tpie
