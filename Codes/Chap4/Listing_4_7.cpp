// [Listing 4.7]
// Passing arguments to a function with std::async
// std::async로 호출하는 함수에 인자 전달하기

#include <string>
#include <future>

struct X
{
	void foo(int, std::string const&);
	std::string bar(std::string const&);
};

X baz(X&);

struct Y
{
	double operator()(double);
};

class move_only
{
public:
	move_only();
	move_only(move_only&&);
	move_only(move_only const&) = delete;
	move_only& operator=(move_only&&);
	move_only& operator=(move_only const&) = delete;

	void operator()();
};

// 실제 사용 예시
// ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
void inFunction(void) 
{
	X x;
	Y y;
	// p->foo(42,"Hello")를 호출한다. 이때 p는 변수 x의 주소이다.
	auto f1 = std::async(&X::foo, &x, 42, "Hello");

	// tmpx.bar("goodbye")를 호출한다. 이때 tmpx는 x의 복사본(copy)이다.
	auto f2 = std::async(&X::bar, x, "goodbye");

	// Y클래스 생성자로부터 이동 생성된 tmpy를 사용해 tmpy(3.141)을 호출한다.
	auto f3 = std::async(Y(), 3.141);

	// y(2.718)을 호출한다.
	auto f4 = std::async(std::ref(y), 2.718);

	// baz(x)를 호출한다.
	std::async(baz, std::ref(x));

	// std::move(move_only())로부터 생성된 tmp를 사용해 tmp()를 호출한다.
	auto f5 = std::async(move_only());

	// 새로운 thread를 생성하여 실행
	auto f6 = std::async(std::launch::async, Y(), 1.2);

	// wait()이나 get()이 호출되면 실행
	auto f7 = std::async(std::launch::deferred, baz, std::ref(x));

	// 구현에 따라서 선택되도록 지정
	auto f8 = std::async(
		std::launch::deferred | std::launch::async,
		baz, std::ref(x));

	// 구현에 따라서 선택되도록 지정
	auto f9 = std::async(baz, std::ref(x));

	// deferred된 함수를 호출
	f7.wait();

}
