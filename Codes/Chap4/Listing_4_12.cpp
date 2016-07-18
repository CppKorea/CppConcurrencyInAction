// [Listing 4.12 : A sequental implementation of Quicksort]
// 퀵소트 : 순차적인 구현

template <typename T>
std::list<T> sequential_quick_sort(std::list<T> input)
{
	// 입력받은 리스트가 공백이면 정렬 없이 반환.
	if (input.empty()) {
		return input;
	}

	// 결과 반환을 위한 리스트
	std::list<T> result;

	// 1. input.begin()에 있는 요소를 result.begin()으로 이동.
	result.splice(result.begin(), input, input.begin());

	// 2. 방금 전달받은 요소를 pivot으로 설정
	T const& pivot = *result.begin();

	// 3. input에서 pivot보다 작은 그룹과
	//	  큰 그룹으로 나누고, 큰 그룹의 첫번째 iterator 반환
	auto divide_point = std::partition(input.begin(), input.end(),
		[&](T const& t) {return t < pivot; }
	);

	// 4. lower_part 리스트에 splice함수를 사용해서,
	//    작은 그룹의 첫 번째 요소부터 마지막 요소까지 이동.
	std::list<T> lower_part;
	lower_part.splice(
		lower_part.end(), input, input.begin(), divide_point);

	// 5. pivot보다 작은 그룹을 퀵소트 정렬
	auto new_lower(
		sequential_quick_sort(std::move(lower_part)));
	// 6. pivot보다 큰 그룹을 퀵소트 정렬
	auto new_higher(
		sequential_quick_sort(std::move(input)));

	// 7. result에는 현재 pivot만 삽입된 상태.
	//    pivot의 뒤쪽부터 pivot보다 큰 요소들의 그룹을 삽입.
	//    이미 정렬된 상태.
	result.splice(result.end(), new_higher);
	// 8. pivot의 앞쪽부터 pivot보다 작 요소들의 그룹을 삽입.
	//    이미 정렬된 상태.
	result.splice(result.begin(), new_lower);

	// 정렬된 list를 반환.
	return result;
}
