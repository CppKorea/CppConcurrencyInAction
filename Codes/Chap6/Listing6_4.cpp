// Listing6_2, Listing6_3에서 보호받는 아이템(data_queue)과 뮤텍스를 사용했다.
// 세밀한 잠금을 사용하기 위해서 큐의 내부(구성부분)를 보고, 각 분리된 데이터 아이템 마다 뮤텍스를 할당해야한다.

// 가장 간단한 큐 자료구조는 링크드 리스트이다.
// 큐는 head 포인터를 포함하고 있다.
// head 포인터는 리스트의 첫번째 아이템을 가리키고, 각 아이템은 다음 다이템을 가리긴다.
// 데이터 아이템은 다음 아이템의 포인터로 헤드 포인터를 교체함으로써 큐에서 삭제되고, 이전 헤드가 반환된다. (데이터 꺼낼때 헤드 이동하고 꺼낸거임)

// 아이템들은 큐의 다른쪽 끝에 추가된다.(tail)
// 이렇게 하기 위해서는, 큐는 리스트의 마지막 아이템을 참조하는 tail 포인터도 가지고 있어야한다.
// 마지막 아이템의 next 포인터를 새 아이템으로 업데이트하고 tail 포인터가 새 아이템을 참조하게 하면서, 새 아이템이 추가된다.
// 리스트가 비어있을때는 head 와 tail 포인터 둘다 NULL이다.

// 아리에서 보게될 예제는 Listing6_2 큐 인터페이스의 cut-down 버전을 기반으로한 큐의 간단한 구현이다
// 이 큐는 오직 싱글 쓰레드만 지원하기 때문에, try_pop() 함수만 있고 wait_and_pop() 함수는 없다.


#include <memory>

template<typename T>
class queue {
private:
    struct node {
        T data;
        std::unique_ptr<node> next;

        node(T data_) : data(std::move(data_)) { }
    };

/* 1 */
    std::unique_ptr<node> head;
/* 2 */
    node* tail;

public:
    queue() { }

    queue(const queue& other) = delete;

    queue& operator=(const queue& other) = delete;

    std::shared_ptr<T> try_pop() {
        if (!head)
            return std::shared_ptr<T>();
        std::shared_ptr<T> const res(std::make_shared<T>(std::move(head->data)));
        std::unique_ptr<node> const old_head = std::move(head);
/* 3 */ head = std::move(old_head->next);
        return res;
    }

    void push(T new_value) {
        std::unique_ptr<node> p(new node(std::move(new_value)));
        node* const new_tail = p.get();
        if (tail) {
/* 4 */     tail->next = std::move(p);
        } else {
/* 5 */     head = std::move(p);
        }
/* 6 */ tail = new_tail;
    }
};

// 우선 Listing6_4에서는 노드들을 관리하기위해 std::unique_ptr<node>을 사용한다는 점을 유의하자.
//  L 이것들은 필요없게되면 delete를 작성하지 않아도 자동으로 삭제된다.
// 이 소유권 체인은 head 와 마지막 노드를 가리키는 tail로 관리된다.

// 비록 이 구현이 싱글 쓰레드에서 잘 될지라도, 멀티쓰레드에서 세밀한 잠금을 사용하려고 하면 몇몇 문제가 생길것이다.
// 2개의 데이터 아이템(head [1], tail [2])이 주어졌을때, 원칙적으로 2개의 뮤텍스를 사용해야한다.
// 하나는 head를 보호하고 다른 하나는 tail 을 보호해야한다.
// 하지만 여기서 몇몇 문제가 생긴다.

// 가장 명백한 문제는 push()가 head[5]와 tail[6]을 수정한다는 것이다.
// 그래서 잠그기위해 둘다 뮤텍스를 사용해야한다.
// 이건 큰 문제가 되지 않지만, 불행히도 두 뮤텍스를 잠그는 것이 가능하다. (두 뮤텍스를 잠궈야 한다.) ->나중에 확인해보고 수정
// 치명적인 문제는 push()와 pop()둘다 노드의 next 포인터에 접근한다는 것이다.
//  L push()는 tail->next [4]를 업데이트한다.
//  L try_pop()은 head->next [3]을 읽는다.
// 만약 큐에 한 아이템만 있었다면, head==tail이게되고, 결국 head->next와 tail->next가 같은 객체가된다.
// 그러므로 보호가 필요하다.
// head와 tail을 모두 읽지 않고 이 두 객체가 같다고 할 수 없기 때문에, push()와 try_pop() 모두 같은 뮤텍스로 잠궈야 한다.
// 그래서 전거보다 더 좋은 방법은 없다.
// 과연 이 딜레마에서 나올 방법은 없을까?
