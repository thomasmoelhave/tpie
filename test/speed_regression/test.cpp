#include "../app_config.h"

#include <tpie/stream.h>
#include <tpie/sort.h>
#include "testtime.h"
#include <tpie/streaming.h>
#include <tpie/mm_manager.h>
#include <stdint.h>

struct node {
  uint64_t x;
  uint64_t y;
  uint64_t xo;
  uint64_t yo;
  uint64_t oo;
};

struct XCMP {
  inline int compare(const node & a, const node & b) const {
    return b.x-a.x;
  }
  inline bool operator() (const node & a, const node & b) const {
    return a.x < b.x;
  }
};

struct YCMP {
  inline int compare(const node & a, const node & b) const {
    return b.y-a.y;
  }
  inline bool operator() (const node & a, const node & b) const {
    return a.y < b.y;
  }
};
  
using namespace tpie;
using namespace tpie::ami;
using namespace tpie::test;
using namespace tpie::streaming;

struct OOF {
  inline uint64_t & operator()(node & n) const {return n.oo;}
};

struct XOF {
  inline uint64_t & operator()(node & n) const {return n.xo;}
};

struct YOF {
  inline uint64_t & operator()(node & n) const {return n.yo;}
};

template <typename dest_t, typename fetch_t> 
struct OA: public common_single<OA<dest_t, fetch_t>, dest_t> {
public:
  typedef common_single<OA<dest_t, fetch_t>, dest_t> parent_t;
  typedef node item_type;
  uint64_t count;
  
  OA(dest_t & dest): parent_t(dest, 0) {};
  
  void begin(TPIE_OS_OFFSET size=0) {
    count=0;
    parent_t::begin(size);
  }

  void end() {
    parent_t::end();
  }

  inline void push(const node & n) {
    node nn = n;
    fetch_t()(nn) = count;
    ++count;
    parent_t::dest().push(nn);
  }
};

typedef stream_sink<stream<node> > sink_t;
typedef OA<sink_t, YOF> yoa_t;
typedef tpie::streaming::sort<yoa_t, YCMP> ysort_t;
typedef OA<ysort_t, XOF> xoa_t;
typedef tpie::streaming::sort<xoa_t, XCMP> xsort_t;
typedef OA<xsort_t, OOF> ooa_t;
typedef stream_source<stream<node>, ooa_t> source_t;

int main(int argc, char ** argv) {
  MM_manager.set_memory_limit(1024*1024*256);
  test_realtime_t start;
  test_realtime_t end;
  uint64_t size = atoi(argv[2]) * 1024ll;
  
  remove("tmp");
  remove("tmp2");

  {
    stream<node> s("tmp", WRITE_STREAM);
    for(uint64_t i=0; i < size; ++i) {
      node n;
      n.x = random();
      n.y = random();
      s.write_item(n);
    };
  } 

  getTestRealtime(start);  
  if (!strcmp("old",argv[1]) ) {
    stream<node> s("tmp", READ_WRITE_STREAM);
    for(uint64_t i=0; i < size; ++i) {
      node * n;
      s.read_item(&n);
      node n2 = *n;
      n2.oo = i;
      s.seek(i);
      s.write_item(n2);
    }
    
    XCMP x;
    tpie::ami::sort(&s, &x);
    s.seek(0);

    for(uint64_t i=0; i < size; ++i) {
      node * n;
      s.read_item(&n);
      node n2 = *n;
      n2.xo = i;
      s.seek(i);
      s.write_item(n2);
    }

    YCMP y;
    tpie::ami::sort(&s, &y);
    s.seek(0);

    for(uint64_t i=0; i < size; ++i) {
      node * n;
      s.read_item(&n);
      node n2 = *n;
      n2.yo = i;
      s.seek(i);
      s.write_item(n2);
    }
  } else {
    stream<node> s1("tmp", READ_STREAM);
    stream<node> s2("tmp2", WRITE_STREAM);
    sink_t sink(&s2);
    sink.setMemoryName("sink");
    yoa_t yoa(sink);
    yoa.setMemoryName("yoa");
    ysort_t ysort(yoa);
    ysort.setMemoryName("ysort");
    xoa_t xoa(ysort);
    xoa.setMemoryName("xoa");
    xsort_t xsort(xoa);
    xsort.setMemoryName("xsort");
    ooa_t ooa(xsort);
    ooa.setMemoryName("ooa");
    source_t source(&s1, ooa); 
    source.setMemoryName("source");

    priority_memory_manager m;
    m.add(&ooa);
    m.add(&source);
    m.allocate(TPIE_OS_SIZE_T(1024*1024*254));
    source.run();
  }
  getTestRealtime(end);


  std::cout << " " << testRealtimeDiff(start,end) << std::endl;
  std::cout.flush();
  remove("tmp");
  remove("tmp");
}
