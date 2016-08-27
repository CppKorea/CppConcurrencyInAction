// 분리된 데이터로 동시성을 가능하게 하기

// Listing6_4에서 보았던 문제점을 값이 없는 더미 노드를 미리 할당함으로써 해결할 수 있다.
// 더미 노드를 미리 할당하면 큐에는 항상 적어도 1개 이상의 노드(head 이면서 tail)가 있다.
// 이제 빈 큐의 head 와 tail을 NULL이 아니라 더미 노드를 가리킨다.
// 큐가 비어있을때 try_pop()이 head->next에 접근하지 않기때문에 안전하다.
// 큐에 노드를 추가한다면, head와 tail은 각각 다른 노드들을 가리키게된다.
// 그래서 head->next와 tail->next사이에 경쟁이 없게된다.
// 단점은 더미 노드들을 위해 포인터로 데이터를 저장하는 추가의 인다이렉션이 들어간다는 것이다.
// 아래에서 어떻게 구현하는제 보게될것이다.


#include <memory>

template<typename T>
class queue {
private:
    struct node {
/* 1 */ std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::unique_ptr<node> head;
    node* tail;

public:
    /* 2 */
    queue() : head(new node), tail(head.get()) { }

    queue(const queue& other) = delete;

    queue& operator=(const queue& other) = delete;

    std::shared_ptr<T> try_pop() {
/* 3 */ if (head.get() == tail) {
            return std::shared_ptr<T>();
        }
/* 4 */ std::shared_ptr<T> const res(head->data);
        std::unique_ptr<node> old_head = std::move(head);
/* 5 */ head = std::move(old_head->next);
/* 6 */ return res;
    }

    void push(T new_value) {
/* 7 */ std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
/* 8 */ std::unique_ptr<node> p(new node);
/* 9 */ tail->data = new_data;
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
};

// try_pop()이 약간 변하였다.
// 먼저, NULL을 체크하기보다 head와 tail[3]을 비교해야한다.
// 더미노드가 있으므로 head가 절대 NULL이 아니라서 NULL 체크할 필요가 없다.
// head가 std::unique_ptr<node>이기때문에, 비교를 하기위해 head.get()를 호출해야한다.
// 두번째로, node가 포인터로 데이터를 저장하기때문에[1], T의 새 인스턴스를 만드는것보다 포인터로 바로 접근하는게 더 낫다.
// push()에서 큰 기회가 있다.
//  L 힙에 T 인스턴스를 먼저 만들고 std::shared_ptr<>로 가져야한다.[7] (두번째 메모리 할당의 오버헤드를 피하기위해 std::make_shared를 사용한다.)
//  L 새로 만드는 노드는 더미노드라서 생성자에게 new_value를 넘겨줄 필요가 없다.[8]
//  L 대신에 오래된 노드에 new_value를 복사해서 넣어준다.[9]
// 마지막으로, 더미노드를 가지기 위해서, 생성자에서 만들어줘야한다.[2]

// push()는 head가 아니라 tail에 접근한다.
// try_pop()은 head와 tail에 모두 접근하지만, tail은 초기비교에만 필요하고, 잠금은 일시적이다.
// 더미노드때문에 try_pop()과 push()가 같은 노드에서 연산하지 않아서 더이상 뮤텍스가 필요하지 않다.(이득)
// 그래서 head용 뮤텍스 하나랑 tail용 하나만 있으면 된다.
// 그럼 어디서 잠금을 걸어야할까?

// 동시성의 기회를 최대화시키려면 가능한 짧은 시간동안 잠금을 유지해야한다.
// push()는 쉽다
//  L 뮤택스는 tail에 대한 모든 접근에서 잠겨야한다. (새 노드가 할당된후[8]와 현재 tail 노드에 데이터를 할당하기전[9]에 뮤텍스를 잠궈야한다.)
//  L 잠근 다음에 함수가 끝날때까지 유지시켜야한다.

// try_pop()은 쉽지 않다
// 먼저, head 뮤텍스를 잠구고 head가 끝나때까지 유지해야한다.
// 뮤텍스는 어떤 쓰레드가 pop할지 결정한다. 그래서 먼저 잠궈야한다.
// head가 변경되면[5], 뮤텍스의 잠금을 풀어야한다.
// 결과를 반환할때 잠글필요는 없다.[6]
// That leaves the access to tail needing a lock on the tail mutex.
// 오직 한번만 tail에 접근해야하기 때문에, 읽는 시간 동안 뮤텍스를 얻을 수 있다.
// 함수를 감싸는것이 가장 좋다.
// 사실 head 뮤텍스를 잠궈야 하는 코드가 멤버들의 집합이라서, 함수를 감싸는게 명확하다.
// 최종 코드는 Listing6_6에서 보자.