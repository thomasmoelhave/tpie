#include <string>
int main() {
  union {
	std::string a;
	int b;
  };
}
