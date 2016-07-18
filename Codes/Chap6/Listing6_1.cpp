#include <exception>
#include <stack>
#include <__mutex_base>

struct empty_stack : std::exception {
    const char* what() const throw;
};


template<typename T>
class threadsafe_stack {
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack() { }

    threadsafe_stack(const threadsafe_stack& other) {
        std::lock_guard<std::mutex> lock(other.m);
/* 1 */ data = other.data;
        // 데이터를 복사/이동할때 예외를 던지거나
        // 아래 자료구조를 확장하기에 충분하지 않은 메모리가 할당될 수 있음
        // may throw an exception if either copying/moving the data value throws an exception
        // or not enough memory can be allocated to extend the underlying data structure.
    }

    threadsafe_stack& operator=(const threadsafe_stack) = delete;

    void push(T new_value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }

    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(m);
/* 2 */ if (data.empty()) throw empty_stack();
        // 첫번째로 오버로딩한 pop()
        // 스스로 empty_stack 예외로 던진다.
        // 비어있는 상태일때 안전하게 해준다.
/* 3 */ std::shared_ptr<T> const res(std::make_shared(std::move(data.top())));
        // 몇가지 이유로 인해 예외가 발생할 수 있음
        // std::make_shared 호출은 새 객체를 메모리에 할당하지 못하면 예외를 던지고
        // 내부 데이터는 레퍼런스 카운팅을 필요로 하거나 반환될 아이템의 복사 생성자 혹은 이동 생성자는 새로 할당된 메모리로 복사/이동할때 예외를 던진다.
        // 두 경우 모두 C++ 런타임과 표준 라이브러리에서 메모리 누수를 막고, 제대로 새 객체가 소멸되도록 보장한다.
/* 4 */ data.pop();
        // 결과 반환으로 예외를 던지지 못하게 보장한다
        // exception-safe 함.
        return res;
    }

    void pop(T& value) {
        // 두번째로 오버로딩한 pop() 이것도 비슷하다.
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
/* 5 */ value = std::move(data.top());
        // 새 객체와 std::shared_ptr 인스턴스를 생성하는것보다
        // 복사 할당 또는 이동 할당 연산자는 예외를 던질 수 있다.
/* 6 */ data.pop();
        // data.pop()이 호출되기 전까지 자료 구조를 수정하지 않는것은 예외 발생을 막는것이다.
        // 또한 이것도 exception-safe하다.
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
    // 마지막으로 empty()는 어떠한 데이터도 수정하지 않는다.
    // 그러므로 이것도 exception-safe 하다.
};

// 기본적인 쓰레드 세이프를 제공하는 방법은 각각의 멤버 함수를 뮤텍스(m)로 잠그는 것이다.
// 이는 한번에 오직 한 쓰레드가 데이터에 접근할 수 있도록 한다.
// 그러므로써 모든 멤버 함수들은 불변을 유지하고 어떤한 쓰레드도 불변이 깨지는것을 볼 수 없다.

// empty()와 pop() 함수들은 경쟁 상태가 발생한 가능성이있다.
// 이 코드들은 명시적으로 pop()에 잠금이 걸려있을때 스택이 비어있는지 명시적으로 확인하기 때문에, 문제가 되지 않는다.
// pop()을 호출하는 부분에서 꺼낸 데이터를 바로 반환해 줌으로써 std::stack의 top(), pop() 멤버 함수에서 발생할 수 있는 잠재적인 경쟁상태를 피한다.

// 여기에는 예외가 발생할 수 있는 몇몇 소스가 있다
// 뮤텍스를 잠그는 것은 예외를 던질 수 있지만, 극히 드물다. (std::mutex 소스 참고)
// 가각의 멤버 함수에서 첫번째로 하는것이 뮤텍스를 잠그는 것이기도 하다.
// 어느 데이터도 수정되지 않기때문에 안전하다
// 뮤텍스를 잠금 해제하는것은 실패하지 않고 항상 안전하다.
// 그리고 std::lock_guard<>는 뮤텍스가 잠긴채로 남아있지 않게 방지한다.

// 잠금이 걸려있는 동안 유저코드를 호출하면 교착상태가 발생할 가능성이 있다.
//  L 복사 생성자 혹은 이동 생성자 1, 3
//  L 들어있는 데이터 항목의 복사 할당 연산자 혹은 이동 할당 연산자 5
//  L 사용자 정의 연산자
// 만약 이 함수들이 스택에 삽입, 삭제같이 잠금을 필요로하는 멤버 함수가 겹쳐서 호출되면 교착상태가 발생할 가능성이 있다.
// 하지만 스택의 사용자가 스택에 아이템 삽입, 삭제를 복사나 메모리 할당 없이 할 수 없도록 보장한다.

// 모든 멤버 함수가 데이터를 보호하기위해 std::lock_guard<>를 사용하기 때문에 수많은 쓰레드가 스택의 멤버함수를 호출해도 안전하다
// 오직 생성자와 소멸자만 안전하지 않지만, 큰 문제는 되지 않는다.
// 객체는 오직 한번 생성되고 소멸될 수 있기 때문이다.
// 완전히 생성되지 않거나 파괴된 객체의 멤버함수를 호출하는것은 concurrency든 아니든 좋은 생각이 아니다.
// 결과적으로, 사용자는 반드시 객체가 완벽하게 생성되기 전까지는 다른 쓰레드들이 스택에 접근하지 못하도록 보장하고, 스택이 소멸되기전에 스택의 접근을 중단시켜야한다.

// 잠금을 사용해서 다중 쓰레드가 멤버함수를 동시에 호출해도 안전할지라도, 한번에 오직 한 쓰레드만 스택 자료구조에서 일을 할 수 있다. (직렬화)
// 이 쓰레드의 직렬화는 어플리케이션의 성능을 제한한다.
// 한 쓰레드가 잠금을 기다리는 동안에는 아무것도 할 수 없다.
// 또한 이 스택은 추가될 아이템을 기다리는 어떠한 수단도 제공해 주지 않는다. (추가가 언제 완료되었는지 알 수 가 없음)
// 그래서 쓰레드가 기다려야 한다면, 주기적으로 empty()를 호출하거나, 그냥 pop()을 호출하고 empty_stack 예외를 캐치해야한다.
// 만약 이런 시나리오가 필요하다면, 이 스택구현은 멍청한 선택이다.
// 대기하는 쓰레드는 데이터를 확인하면서 자원을 소비하거나 사용자가 외부 대기 및 알림코드(예:상태변수)를 작성해야하기 때문에 쓰래기다.
// 4장의 큐는 자료구조 안에서 상태변수를 사용해서 스스로 대기하는 모습을 보여준다. ([4장] 조건변수를 사용해 특정 조건 기다리기)
// 그래서 다음으로 이것을 살펴보도록 하자 (Listing6_2.cpp)