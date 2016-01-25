#include <__mutex_base>
#include <queue>

template<typename T>
class threadsafe_queue {
private:
    mutable std::mutex mut;
    std::queue<std::shared_ptr<T>> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue() { }

    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
/* 1 */ value = std::move(*data_queue.front());
        data_queue.pop();
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;
/* 2 */ value = std::move(*data_queue.front());
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
/* 3 */ std::shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>();
/* 4 */ std::shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }

    void push(T new_value) {
/* 5 */ std::shared_ptr<T> data(std::make_shared<T>(std::move(new_value)));
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};

// std::shared_ptr<>로 데이터를 홀딩한 결과는 간단하다.
//  L 새 데이터를 받기 위해 래펀런스 변수를 받아오는 pop() 함수들은 저장된 포인터 [1, 2번]을 역참조하고,
// std::shared_ptr<> 인스턴스를 반환하는 pop() 함수들은 호출자에게 반환하기전에 큐에서 찾는다.[3, 4번]

// 만약 std::shared_ptr<>이 데이터를 잡고있다면 장점이 있다.
//  L 새 인스턴스 할당이 push() [5번]의 잠금 밖에서 가능하다. (Listing6_2에서는 pop()이 잠겨있는 동안 했었다.)
// 메모리 할당이 꽤 비싼 연산이기때문에, 큐의 성능면에서 이득이다.
// 이건 뮤텍스가 잡고있는 시간을 줄여주고, 다른 쓰레드가 그동안 다른 작업들을 수행할 수 있도록 해준다.

// 스택 예제처럼 한 뮤텍스를 전체 자료구조를 보호하기 위해 사용하는것은 이 큐가 지원하는 동시성을 제한한다.
//  L 비록 여러 쓰레드들이 큐의 다양한 함수들에서 막혔을지라도, 한번에 오직 한 쓰레드는 일할 수 있다.
//  L 하지만 이 제한은 std::queue<>의 사용때문이다.
//    L 표준 컨테이너를 사용함으로써 보호받든 안받든 한 아이템(큐)을 가지고 있다.
// 자료 구조의 세부 구현을 컨트롤함으로써, 보다 세밀한 잠금을 제공하고 더 높은 수준의 동시성이 가능하게 한다.