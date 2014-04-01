// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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

/* This is an example TPIE program.
 * It illustrates the use of tpie::file and tpie::file::stream to implement an
 * external memory queue.
 */

#include <tpie/file.h>
#include <tpie/tempname.h>
#include <tpie/tpie.h>
#include <iostream>

struct em_queue {
	typedef int T;
	tpie::file<T> queue;
	tpie::file<T>::stream front;
	tpie::file<T>::stream back;
	tpie::stream_size_type enqueued;

	void enqueue(const T & item) {
		back.write(item);
		++enqueued;
	}

	T dequeue() {
		assert(enqueued > 0);
		--enqueued;
		return front.read();
	}

	void open_queue(tpie::temp_file & fileName) {
		queue.open(fileName, tpie::access_read_write, sizeof(tpie::stream_size_type));
		front.attach(queue);
		back.attach(queue);
		back.seek(0, tpie::file_base::stream::end);
		enqueued = queue.size();
		if (queue.user_data_size() > 0) {
			tpie::stream_size_type dequeued;

			queue.read_user_data(dequeued);
			// The above line is roughly equivalent to:
			queue.read_user_data(&dequeued, sizeof(tpie::stream_size_type));

			std::cout << "Seek to " << dequeued << std::endl;
			front.seek(dequeued);
			enqueued -= dequeued;
		}
	}

	void close_queue() {
		std::cout << "Write offset " << front.offset() << std::endl;
		queue.write_user_data(front.offset());
		front.detach();
		back.detach();
		queue.close();
	}
};

int main() {
	tpie::tpie_init();
	{
		tpie::temp_file fl;
		int wr = 42;
		int rd = 42;
		{
			em_queue q;
			q.open_queue(fl);
			for (size_t i = 0; i < 100000; ++i) {
				if (1 == i % 3) {
					int item = q.dequeue();
					if (item != rd) {std::cout << "Wrong item" << std::endl; return 1;}
					++rd;
				} else {
					q.enqueue(wr);
					++wr;
				}
			}
			q.close_queue();
		}
		{
			em_queue q;
			q.open_queue(fl);
			while (q.enqueued > 0) {
				int item = q.dequeue();
				if (item != rd) {std::cout << "Wrong item" << std::endl; return 1;}
				++rd;
			}
			q.close_queue();
		}
	}
	std::cout << std::flush;
	tpie::tpie_finish();
	return 0;
}
