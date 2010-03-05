#include <stdio.h>
#include <iostream>
#include "virtual_real.h"
#include <boost/timer.hpp>
#include <tpie/util.h>
#include <boost/filesystem.hpp>

using namespace tpie::streaming;
using namespace tpie;
using namespace boost::filesystem;

class test_sink: public memory_single {
public:
  typedef empty_type begin_data_type;
  typedef empty_type end_data_type;

  typedef int item_type;
  int val;
  void begin(stream_size_type items=max_items, empty_type *data=0) {
    unused(items);
    unused(data);
    val=0;
  }
  void push(int x) {
    val += x;
  }
  void end(empty_type * data=0) {
    unused(data);
    if (val == 42) printf("Im only here to cheat the optimizer\n");
  }
};

template <typename dest_t>
class test_source: public common_single<test_source<dest_t>, dest_t> {
public:
  typedef common_single<test_source<dest_t>, dest_t> parent_t;
  using parent_t::dest;

  test_source(dest_t & dest): parent_t(dest, 0.0) {}
  void run() {
    dest().begin();
    for(int j=0; j < 150; ++j)
      for(int i=0; i < 1000000; ++i)
	dest().push(i);
    dest().end();
  }
};

int bestValue;
double bestTime;

template <int buff_size> 
void vbufftest() {
  double time;
  if (buff_size == 1) {
    typedef test_sink dst_t;
    typedef virtual_source_impl_real_single<dst_t> vsrc_t;
    typedef virtual_sink_real_impl_single<int> vsink_t;
    typedef test_source<vsink_t> src_t;
    dst_t dest;
    vsrc_t vsrc(dest);
    vsink_t vsink(&vsrc);
    src_t src(vsink);
    boost::timer t;
    src.run();
    time = t.elapsed();
  } else {
    typedef test_sink dst_t;
    typedef virtual_source_impl_real<dst_t, buff_size> vsrc_t;
    typedef virtual_sink_real_impl<int, buff_size> vsink_t;
    typedef test_source<vsink_t> src_t;
    dst_t dest;
    vsrc_t vsrc(dest);
    vsink_t vsink(&vsrc);
    src_t src(vsink);
    boost::timer t;
    src.run();
    time = t.elapsed();
  }
  printf("%2d: %2.3lf\n", buff_size, time);
  if (time < bestTime*0.90) {
    bestTime = time;
    bestValue = buff_size;
  }
}

int main(int argc, char ** argv) {
  bestTime = 10000000;
  vbufftest<1>();
  vbufftest<2>();
  vbufftest<4>();
  vbufftest<6>();
  vbufftest<10>();
  vbufftest<16>();
  vbufftest<24>();
  vbufftest<32>();
  vbufftest<40>();
  vbufftest<64>();
  printf("Virtual buffer size choosen to %d\n", bestValue);
  
  create_directories( path(argv[1]).parent_path() );
  FILE * f = fopen(argv[1], "wb");
  if (f == NULL) {
    std::cerr << "Unable to write to file \"" << argv[1] << "\", build system is broken" << std::endl;
    return 1;
  }
  fprintf(f, "#define TPIE_STREAMING_VBUFF_SIZE %d\n", bestValue);
  fclose(f);
  return 0;
}
