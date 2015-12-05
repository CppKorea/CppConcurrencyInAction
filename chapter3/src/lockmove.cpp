#include <iostream>
#include <mutex>
#include <string>

void prepare_data()
{

}

void do_something()
{
}

class Y
{
public:
	std::mutex some_mutex;

	void process_data()
	{
		std::unique_lock<std::mutex> lk( get_lock() ); /* (1) */
		do_something();

		std::cout << "owns_lock : " << lk.owns_lock() << std::endl;
	} 

private:
	std::unique_lock<std::mutex> get_lock()
	{
		std::unique_lock<std::mutex> lk(some_mutex);
		prepare_data();

		return lk; /* (1) rvalue 이므로 소유권이 자동적으로 move 한다 */
	} /* gateway_class */


};
int main()
{
	Y a;
	a.process_data();

	getchar();

	return 0;
}
