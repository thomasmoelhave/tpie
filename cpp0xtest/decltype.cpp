template <typename T>
class foo {};

class bar {
public:
  void baz() {}
};

int main() { 
  decltype(1+1) x = 1+1;
  foo<decltype(&bar::baz)> y;
  return 0; 
}
