#include <iostream>
#include <mutex>

class Y
{
private:
    int some_detail;
    mutable std::mutex m;

    int get_detail() const
    {
        std::lock_guard<std::mutex> lock_a(m);
        return some_detail;
    }
public:
    Y(int sd):some_detail(sd){}

    friend bool operator==(Y const& lhs, Y const& rhs)
    {
        if(&lhs==&rhs)
            return true;
        int const lhs_value=lhs.get_detail();
		int const rhs_value=rhs.get_detail();
        return lhs_value==rhs_value;
		
		/* 만약 lhs_value 값을 가져온 직 후  lhs 와 rhs 의  some_detail 값이 변할 경우
		   lhs 와 rhs 의 some_detail 값이 서로 같지 않음에도 true 가 리턴 될 수 있다. */
		/* ex ) 함수 호출 당시 lhs = 4, rhs = 5 값을 가지고 있을 때 lhs 값을 get_detail 한 직 후
		   lhs = 3 rhs = 4 값으로 변할경우 이 두값은 서로 같지 않음에도 true 가 리턴 될 것이다. */
    }

	void show_detail()
	{
		std::cout << get_detail() << std::endl;
	}
};

int main()
{
	bool equal = false;

	Y a(5);
	Y b(5);

	std::cout << "a : ";
	a.show_detail();
	std::cout << "b : ";
	b.show_detail();

	equal = (a == b);

	std::cout << "a == b : " << equal << std::endl;

	getchar();
}
