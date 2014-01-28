#include <utility>

int && foobar();

int main() {
	int a = 9;
	int b = std::move(a);
	int c = std::forward<int>(a);
}
