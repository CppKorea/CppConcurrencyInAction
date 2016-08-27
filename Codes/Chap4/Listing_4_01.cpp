// [Listing 4.1]
// Waiting for data to process with a std::condition_variable
// 조건변수를 사용한 대기.

#include <queue>
#include <mutex>
#include <condition_variable>

std::mutex mut;
std::queue<data_chunk> data_queue;
std::condition_variable data_cond;

// Producer
// 데이터를 queue에 삽입하는 루틴.
void data_preparation_thread()
{
	// 준비된 데이터가 있으면...
	while(more_data_to_prepare())
	{
		// 삽입할 데이터 생성
		data_chunk const data = prepare_data();
		
		// 잠금 처리
		// data_queue에 대한 안전한 접근 보장.
		std::lock_guard<std::mutex> lk(mut);
		
		// 생성된 데이터를 삽입
		data_queue.push(data);
		
		// Consumer 루틴에게 전파
		data_cond.notify_one();
	}
}

// Consumer
// 데이터를 queue에서 꺼내서 처리하는 루틴.
void data_processing_thread()
{
	// 무한루프
	while(true)
	{
		// 잠금 처리
		// data_queue에 대한 안전한 접근 보장.
		std::unique_lock<std::mutex> lk(mut);
		
		// 람다 함수 predicate를 사용한 조건 대기.
		// Wakeup이 될 때마다
		// !data_queue.empty() == ture 인지 검사.
		data_cond.wait(
			lk,[]{return !data_queue.empty();});
		
		// predicate가 참이면 대기를 멈추고 queue에서 데이터 인출
		data_chunk data=data_queue.front();
		data_queue.pop();
		
		// queue에 대한 접근 종료.
		// 잠금 해제
		lk.unlock();
		
		// 인출한 데이터를 처리.
		process(data);
		
		// 마지막 데이터일 경우 무한루프 종료.
		if(is_last_chunk(data))
			break;
	}
}
