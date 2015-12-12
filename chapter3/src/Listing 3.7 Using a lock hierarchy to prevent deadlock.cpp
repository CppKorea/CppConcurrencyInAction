#include "Listing 3.8 A simple hierarchical mutex.h"
#include <mutex>

hierarchical_mutex high_level_mutex(10000); 
hierarchical_mutex low_level_mutex(50000); 

int do_low_level_stuff();

int low_level_func()
{
	std::lock_guard<hierarchical_mutex> lk(low_level_mutex);
	return do_low_level_stuff();
}

void high_level_stuff(int some_param);

void high_level_func()
{
	std::lock_guard<hierarchical_mutex> lk(high_level_mutex);
	high_level_stuff(low_level_func()); 
}

void thread_a() // 룰을 준수하므로 제대로 돌아감
{
	high_level_func();
}

hierarchical_mutex other_mutex(100);
void do_other_stuff();

void other_stuff()
{
	high_level_func(); 
	do_other_stuff();
}

void thread_b() // 룰을 준수하지 않아 제대로 돌아가지 않음
{
	std::lock_guard<hierarchical_mutex> lk(other_mutex);  
	other_stuff();
}