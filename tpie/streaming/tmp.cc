#include "virtual.h"

using namespace tpie::streaming;
int main() {
  virtual_sink_impl<int> s(NULL);
  return 0;
}
