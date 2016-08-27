// [Listing 4.9]
// Running code on a GUI thread using std::packaged_task
// std::packaged_task를 사용한 GUI 스레드에서의 실행 코드

#include <deque>
#include <mutex>
#include <future>
#include <thread>
#include <utility>
std::mutex m;
std::deque<std::packaged_task<void()> > tasks;

// GUI종료 메세지 수신
bool gui_shutdown_message_received();
// GUI 메세지 처리
void get_and_process_gui_message();

// 1. GUI 스레드가 실행하는 루틴
void gui_thread()
{
	// 2. GUI 종료 메세지가 수신될 때까지...
	while (!gui_shutdown_message_received())
	{
		// 3. GUI 메세지를 수신하여 처리.
		get_and_process_gui_message();
		
		// void() 형 함수를 수행하는 task 변수 생성
		std::packaged_task<void()> task;

		// 잠금 처리
		// 작업 큐에 대한 안전한 접근 보장
		{
			std::lock_guard<std::mutex> lk(m);
			// 4. 작업 큐에 내용을 계속 확인.
			if (tasks.empty()) {
				continue;
			}

			// 5. 루프 이탈 : 작업 큐에 작업이 존재.
			// 첫번째 작업을 task 변수로 이동시킴.
			task = std::move(tasks.front());
			tasks.pop_front();
		}
		// 6. task변수에 저장된 void()형 작업(함수)을 실행.
		task();
	}
}

// GUI 스레드.
// 백그라운드에서 gui_thread()루틴 실행.
std::thread gui_bg_thread(gui_thread);


// GUI 스레드로 작업을 전달하는 템플릿 함수.
template<typename Func>
std::future<void> post_task_for_gui_thread(Func f)
{
	// 7. 함수(Callable Object)를 전달받아
	// packaged_task 생성
	std::packaged_task<void()> task(f);

	// 8. 앞서 생성한 task로부터 future변수를 생성.
	// packaged_task 내부의 promise 변수와 연결
	std::future<void> res = task.get_future();

	// 잠금
	// 작업 큐에 대한 안전한 접근 보장.
	std::lock_guard<std::mutex> lk(m);

	// 9. 작업을 작업 큐에 삽입.
	tasks.push_back(std::move(task));

	// 10. future변수를 반환.
	return res;
}
