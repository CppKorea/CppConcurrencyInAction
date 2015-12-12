#include <mutex>
#include <iostream>


class some_data
{
	int a;
	std::string b;
public:
	void do_something()	{}
};

class data_wrapper
{
private:
	some_data data;
	std::mutex m;
public:
	template<typename Function>
	void process_data(Function func)
	{
		std::lock_guard<std::mutex> l(m);	//process_data를 보호
		func(data); //foo가 보호 우회를 위해 malicious_function 통과 가능
	}
};

some_data* unprotected;

void malicious_function(some_data& protected_data)
{
	unprotected = &protected_data;
}

data_wrapper x;

void foo()
{
	x.process_data(malicious_function); // Pass in a malicious function
	//여기에 std::lock_guard를 하지 않음

	unprotected->do_something(); //mutex의 lock 없이 호출 가능
}