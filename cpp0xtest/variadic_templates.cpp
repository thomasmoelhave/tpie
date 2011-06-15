template<typename... TS> 
class abe: public TS... {
 public:
};

class bar {};

int main() {
  abe<abe<abe<>>, bar>();
}
