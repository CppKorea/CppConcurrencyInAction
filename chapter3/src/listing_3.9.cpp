#include <mutex>
#include <iostream>
#include <thread>
#include <string>

class some_big_object
{
private:
	int a;
public:
	some_big_object( int _a ) :a(_a) {}
	~some_big_object() {};

	int getA() { return a; }
	void setA(int _a) { a = _a; }
};

void swap(some_big_object& lhs,some_big_object& rhs)
{
	int temp = lhs.getA();
	
	lhs.setA(rhs.getA());
	rhs.setA(temp);
}

class X
{
private:
    some_big_object some_detail;
    mutable std::mutex m;
public:
    X(some_big_object const& sd):some_detail(sd){}

	void printAll( std::string msg)
	{
		std::cout << msg << "some_detail : " << some_detail.getA() << std::endl;
	}

    friend void swap(X& lhs, X& rhs)
    {
        if(&lhs==&rhs)
            return;

		std::cout << "in funswap" << std::endl;
        
		std::unique_lock<std::mutex> lock_a(lhs.m,std::defer_lock);
        std::unique_lock<std::mutex> lock_b(rhs.m,std::defer_lock);
		/* defer_lock 인자를 이용하여 lock 생성시 unlock 상태로 생성한다 */

		std::cout << "owns_lock " << lock_a.owns_lock() << std::endl;
		/* 현재 mutex 소유권 확인 */

        std::lock(lock_a,lock_b);

		std::cout << "owns_lock " << lock_a.owns_lock() << std::endl;

        swap(lhs.some_detail,rhs.some_detail);
    } /* unique_lock 은 mutex 객체의 소유권 플래그를 확인하여 소유권을 가지고 있으면 소멸자에서 unlock 한다 */
};

int main() 
{
	int a = 0;
	some_big_object sboA(2);
	some_big_object sboB(3);

	X x(sboA);
	X x2(sboB);
	
	x.printAll( "x " );
	x2.printAll( "x2 " );

	swap(x, x2);

	x.printAll( "x " );
	x2.printAll( "x2 " );

	getchar();
	return 0;
}
