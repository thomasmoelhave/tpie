class Base {
public:
	virtual ~Base();
	virtual void foo();
};

class Derived : public Base {
public:
	void foo() override;
};

int main() {}
