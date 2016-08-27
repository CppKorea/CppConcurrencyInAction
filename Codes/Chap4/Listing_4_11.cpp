// [Listing 4.11]
// 이벤트가 발생하기가지 시간제한을 두고 대기하기.

#include <condition_variable>
#include <mutex>
#include <chrono>
std::condition_variable cv;
bool done;
std::mutex m;

bool wait_loop()
{

   auto const timeout = std::chrono::steady_clock::now()
                        + std::chrono::milliseconds(500);
   std::unique_lock<std::mutex> lk(m);
   while(!done)
   {
      // 지금으로부터 500초간 대기.
      // 500초 안에 notify()가 호출되지 않으면, while문 탈출
      if( cv.wait_until( lk, timeout ) == std::cv_status::timeout )
         break;
   }
   return done;
}
