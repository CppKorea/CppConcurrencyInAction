#include <iostream>
#include <mutex>

class some_class
{
private:
	int data;
public:
	some_class() :data(2) {}
	some_class(const some_class& other)
	{
		data = other.data;
	}

	some_class& operator=(const some_class& other)
	{
		if (this != &other)
		{
			data = other.data;
		}

		return *this;
	}
	~some_class() {}

	int getA() { return data; }
	void setA(int _data) { data = _data; }
private:

};


void write_result( some_class &a, int result)
{
	a.setA(result);
}

int process( some_class &a)
{
	int b = a.getA();

	std::cout << "data : " << b << std::endl;
	return b;
}

/* unique_lock 을 이용하여 lock 의 granularity 를 세분화 하여 lock 경합을 줄였다. */
void get_and_process_data()
{
	std::mutex m;
	some_class get_next_data_chunk;

	std::unique_lock<std::mutex> my_lock(m);
	some_class data_to_process = get_next_data_chunk;
	my_lock.unlock(); /* (1) */

	/* 이 시점에 get_and_process_data 를 다른 스레드에서 호출할 경우 invariant 가 파괴될 가능성이 매우 크다. */ 
	int result = process(data_to_process);

	my_lock.lock(); /* (2) */
	write_result(data_to_process, result);
}

int main()
{
	get_and_process_data();

	getchar();

	return 0;
}
