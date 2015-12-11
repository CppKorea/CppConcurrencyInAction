// [Listing 4.13] : Parallel Quicksort using futures
// future를 사용한 병렬 퀵소트

template <typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
	// 입력받은 리스트에 원소가 없으면 그대로 반환
	if (input.empty()) {
		return input;
	}

	// 정렬된 리스트를 저장할 변수
	std::list<T> result;

	// 입력받은 리스트의 첫 번째 원소를 result.begin()에 삽입.
	result.splice(result.begin(), input, input.begin());

	// result로 옮긴 원소를 pivot으로 설정.
	T const& pivot = *result.begin();

	// input을 2 그룹으로 나누고 2번째 그룹의 첫 원소 iterator를
	// divide_point에 저장.
	//		앞쪽(작은) 그룹은 pivot보다 작은 원소들.
	//		뒤쪽(큰)   그룹은 pivot보다 큰 원소들.
	auto divide_point = std::partition(input.begin(), input.end(),
		[&](T const& t) {return t < pivot; }
	);

	// 앞쪽 그룹을 lower_part의 뒤쪽부터 삽입.
	// 뒤쪽 그룹은 그대로 input에 남아있는 상태.
	std::list<T> lower_part;
	lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

	// 1. async()를 사용해서 비동기적으로 lower_part를 정렬시킴
	//		함수인자로는 병렬 퀵소트 주소를, 
	//		인자로 lower_part전달.
	std::future<std::list<T>> new_lower(
		std::async(&parallel_quick_sort<T>, std::move(lower_part)));

	// 2. 뒤쪽 그룹은 현재 스레드에서 다시 정렬
	auto new_higher(
		parallel_quick_sort(std::move(input)));

	// 3. 현재 스레드가 큰 그룹의 정렬을 마친 후,
	//    result 리스트의 뒤쪽 부분부터 큰 그룹을 삽입.
	result.splice(result.end(), new_higher); .
	
	// 4. 비동기적으로 실행시켰던 작은 그룹을 
	//    future를 사용해서 get().
	//    result리스트의 앞쪽 부터 삽입.
	result.splice(result.begin(), new_lower.get());

	// 정렬된 result 리스트를 반환.
	return result;
}
