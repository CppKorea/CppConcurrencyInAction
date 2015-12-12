#include <mutex>

//lock(), unlock(), try_lock() 함수를 구현해 놓아서 std::lock_guard<>와 함께 사용가능

class hierarchical_mutex
{
	std::mutex internal_mutex;
	unsigned long const hierarchy_value;
	unsigned long previous_hierarchy_value;
	static thread_local unsigned long this_thread_hierarchy_value;   // (1)

	void check_for_hierarchy_violation()
	{
		if (this_thread_hierarchy_value <= hierarchy_value)  // (2)
		{
			throw std::logic_error("mutex hierarchy violated");
		}
	}
	void update_hierarchy_value()
	{
		previous_hierarchy_value = this_thread_hierarchy_value;   // (3)
		this_thread_hierarchy_value = hierarchy_value;
	}
public:
	explicit hierarchical_mutex(unsigned long value):
	hierarchy_value(value),
		previous_hierarchy_value(0)
	{}

	void lock()
	{
		check_for_hierarchy_violation();
		internal_mutex.lock();  // (4)
		update_hierarchy_value();   // (5)
	}
	void unlock()
	{
		this_thread_hierarchy_value = previous_hierarchy_value;   // (6)
		internal_mutex.unlock();
	}
	//mutex의 lock을 다른 쓰레드가 잡고 있으면, 호출한 쓰레드에 false를 반환
	bool try_lock()
	{
		check_for_hierarchy_violation();
		if (!internal_mutex.try_lock())  // (7)
			return false;
		update_hierarchy_value();
		return true;
	}
};

thread_local unsigned long
hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX); // (8)