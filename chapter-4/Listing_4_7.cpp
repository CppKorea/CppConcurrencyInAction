// [Listing 4.7]
// std::async로 호출하는 함수에 인자 전달하기

#include <string>
#include <future>

struct X
{
     void foo(int, std::string const&);
     std::string bar(std::string const&);
};

X x;

// p->foo(42,"Hello")를 호출한다. 이때 p는 변수 x의 주소이다.
auto f1 = std::async(&X::foo, &x, 42, "Hello");

// tmpx.bar("goodbye")를 호출한다. 이때 tmpx는 x의 복사본(copy)이다.
auto f2 = std::async(&X::bar, x,  "goodbye");

struct Y
{     
     double operator()(double);
};

Y y;


// Y클래스 생성자로부터 이동 생성된 tmpy를 사용해 tmpy(3.141)을 호출한다.
auto f3 = std::async(Y(), 3.141);

// y(2.718)을 호출한다.
auto f4 = std::async(std::ref(y), 2.718);

X baz(X&);

// baz(x)를 호출한다.
std::async(bax,std::ref(x));

class move_only
{
     move_only();
     move_only(move_only&&)
     move_only(move_only const&) = delete;
     move_only& operator=(move_only&&);
     move_only& operator=(move_only const&) = delete;

     void operator()();
};

// std::move(move_only())로부터 생성된 tmp를 사용해 tmp()를 호출한다.
auto f5 = std::async(move_only());
