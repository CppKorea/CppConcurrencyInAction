#include <queue>
#include <__mutex_base>

template<typename T>
class threadsafe_queue {
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue() { }

    void push(T new_value) {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(std::move(new_value));
/* 1 */ data_cond.notify_one();
    }

/* 2 */
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
    }

/* 3 */
    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(mut);
/* 4 */ data_cond.wait(lk, [this] { return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty()) return false;
        value = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
/* 5 */     return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};

// 4장의 쓰레드 세이프 큐를 다시 만든 것이다.
// 앞에서 스택을 std::stack<> 을 모델로 구현한 것처럼 이 큐도 std::queue 를 모델로 구현했다
// 다중쓰레드의 동시 접근으로부터의 안전해야하기때문에 표준 컨테이너 어댑터와 인터페이스가 다르다.

// 이 큐 자료구조 구현은 push()의 data_cond.notify_one() [1번]과 wait_and_pop()[2,3번]을 제외하면 Listing6_1의 스택과 비슷하다.
// 이 두 오버로딩한 try_pop() 메소드들은 큐가 비어있어도 예외를 던지지 않는것을 제외하면 Listing6_1의 pop()함수와 거의 똑같다.
// 예외를 던지지 않는 대신에 데이터가 있는지 알려주는 bool 값을 반환하거나, [5번]처럼 데이터가 없는 경우에는 NULL 포인터를 반환한다.
// 스택도 이런 방법으로 구현할 수 있을 것이다.

// 새로운 wait_and_pop() 함수들은 스택에서 보았던 큐 항목에 대한 대기문제의 해결책이다.
// empty() 를 계속 호출하는것보다 대기하는 쓰레드가 wait_and_pop()을 한번 호출하고 자료구조가 상태변수로 기다려주는 것이 낫다.
// data_cond.wait()는 큐가 적어도 하나의 원소를 가질때까지 반환하지 않는다.
// 그러므로 이 코드가 비어있는 큐에서 가능한지, 그리고 데이터가 뮤텍스의 잠금으로 계속 보호 받는지 걱정할 필요 없다.

// exception-safety 에 관해 약간의 차이(오해?)가 있다
//  L 여러 쓰레드가 아이템이 푸시되고 있는 큐를 기다릴때, 오직 한 쓰레드가 data_cond.notify_one()에 의해 깨어난다.
// 하지만 쓰레드가 wait_and_pop()에서 새 std::shared_ptr<> 생성[4번] 같은걸로 예외를 던지면 다른 쓰레드들은 깨어나지 않는다.
// 이를 수용할 수 없으면, data_cond.notify_all()로 변경해야한다.
// data.notify_all()은 모든 쓰레드들을 깨우지만, 대부분은 큐가 비어있는것을 확인하고 다시 자러간다.
// 두번째 대안은 wait_and_pop()이 예외를 던질때 notify_one()을 호출하도록 하는것이다.
// notify_one을 호출함으로써 다른 쓰레드는 저장된 값을 retrieve할 수 있게 된다.
// 세번재 대안은 std::shared_ptr<> 생성자를 push() 호출할때로 옮기고 std::shared_ptr<> 인스턴스 대신 데이터 값들을 직접 저장하는것이다.
// std::shared_ptr<>을 내부 std::queue<> 밖에서 복사를 하게 되면 예외를 던지지 않아서, wait_and_pop()은 안전하다.
// 다음 볼 큐는 이를 염두해 두고 구현한 것이다.
// Listing6_3으로 ㄱ

//  [MSDN 참고]
//   L condition_variable::notify_all : condition_variable 개체를 기다리는 모든 스레드를 차단해제합니다.
//   L condition_variable::notify_one : condition_varibale 개체를 기다리는 스레드 중 하나를 차단해제 합니다.