#include <list>
#include <mutex>
#include <algorithm>

std::list<int> some_list;	//전역변수
std::mutex some_mutex;		//some_list 보호하는 전역 인스턴스

void add_to_list(int new_value)
{
	std::lock_guard<std::mutex> guard(some_mutex);	//상호배제 되었음을 의미
	some_list.push_back(new_value);
}
bool list_contains(int value_to_find)
{
	std::lock_guard<std::mutex> guard(some_mutex);	//상호배제 되었음을 의미
	return std::find(some_list.begin(), some_list.end(), value_to_find)
		!= some_list.end();
}

//list_contains()는 절대로 add_to_list()가 수정하고 있는 리스트의 일부분을 볼 수 없음



