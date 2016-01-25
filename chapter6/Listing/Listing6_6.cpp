// 세밀한 잠금이 적용된 쓰레드세이프 큐

#include <memory>
#include <__mutex_base>

template<typename T>
class threadsafe_queue {
private:
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node* tail;

    node* get_tail() {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }

    std::unique_ptr<node> pop_head() {
        std::lock_guard<std::mutex> head_lock(head_mutex);

        if (head.get() == get_tail()) {
            return nullptr;
        }
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }

public:
    threadsafe_queue() : head(new node), tail(head.get()) { }

    threadsafe_queue(const threadsafe_queue& other) = delete;

    threadsafe_queue& operator=(const threadsafe_queue& other) = delete;

    std::shared_ptr<T> try_pop() {
        std::unique_ptr<node> old_head = pop_head();
        return old_head ? old_head : std::shared_ptr<T>();
    }

    void push(T new_value) {
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
        std::unique_ptr<node> p(new node);
        node* const new_tail = p.get();
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data = new_data;
        tail->next = std::move(p);
        tail = new_value;
    }
};

// 6.1.1에서 보았던 가이드라인을 생각하며 이 코드를 공격적으로 봐보자.
// 파괴된 invariant를 보기전에, 이것들을 확실하게 알아야한다.
//  L tail->next == nullptr
//  L tail->data == nullptr
//  L head == tail 은 리스드가 비었음을 의미한다.
//  L 1개의 원소가 있을때는 head->next == tail 이다.
//  L 리스트의 각 노드 x에 대하여, 'x != tail', 'x->data'는 T 인스턴스를 가리키고, x->next는 리스트의 다음 노드를 가리킨다.
//  L head의 next 노드를 따가가다보면 결국 tail이 나올것이다.

// push() 자체는 똑바르다.
// 자료구조의 수정은 tail_mutex에의해 보호받고, 불변을 유지한다.
// (새 tail 노드는 비어있고, data와 next는 현재 리스트에서 가장 마지막에 있는 tail노드가 적용된다.)

// 흥미로운 부분은 try_pop()이다.
// tail 읽기를 보호하기위해, tail_mutex의 잠금만이 필요한것이 아니다.
// head의 데이터를 읽어서 경쟁상태를 만들면 안된다.
// 만약 그 mutex를 가지고 있지 않다면, try_pop()을 호출하는 쓰레드와 push()를 동시에 호출하는 쓰레드는 서로 정의된 작업순서가 없다.
// 게다가 각 멤버 함수가 mutex로 잠그고있으면, 다른 뮤텍스들로 잠거야한다.
// 같은 데이터에 접근할 가능성이 있다.
// 큐의 모든 데이터들은 push()로 들어갔다.
// 모든 쓰레드가 작업순서 정의 없이 같은 데이터에 접근하기 때문에, 5장에서 봤듯이 경쟁상태와 undefined behavior가 발생한다.
// 감사하게도 get_tail()의 tail_mutex 잠금은 모든것을 해결해준다.
// get_tail()의 호출은 push()의 호출과 같이 같은 뮤텍스를 잠그기때문에, 두 호출사이에 순서가 정의되어있다.
// tail의 전 값을 볼경우에는 get_tail()의 호출이 push()전에 일어나고,
// tail의 새 값을 보고 전 tail의 값을 새 데이터에 넣을때는, push()이후에 일어난다.

// get_tail()의 호출이 head_mutex 잠금안에서 일어나는것도 중요하다
// 만약 그렇지 않으면, pop_head()호출은 get_tail()과 head_mutex의 자금 사이에서 stuck 될수 있다.
// 다른 쓰레드들이 try_pop()을 호출하고 잠금을 먼저 필요로 하기 때문에, 초기 쓰레드는 다음 과정으로 만들어야한다.

// std::unique_ptr<node> pop_head() {                       // 이건 잘못된 구현이다.
//     node* const old_tail = get_tail();                   // [1] head_mutex잠금 밖에서 old tail의 값을 가져온다.
//     std::lock_guard<std::mutex> head_lock(head_mutex);
//
//     if (head.get() == old_tail) {                      // [2]
//         return nullptr;
//     }
//     std::unique_ptr<node> old_head = std::move(head);
//     head = std::move(old_head->next);                    // [3]
//     return old_head;
// }

// 이 broken 시나리오에서 get_tail()을 잠금 밖에서 호출하고 있다
// initial 쓰레드가 head_mutex에 잠금을 요청할때 head와 tail을 수정해버려서,
// get_tail()에서 반환받은 tail 노드가 더이상 tail 노드가 아닐 뿐만 아니라, 리스트의 노드도 아니게된다.
// 그러면 head와 old_tail의 비교는 실패하게된다.
// 결과적으로 head를 업데이트할때 head를 tail 너머의 리스트 밖으로 보낼 수 있기 때문에 자료구조가 파괴된다
// get_tail() 호출을 잠금 안으로 옮겨야한다
// => 다른 쓰레드들이 head와 tail을 수정하지 못하고 불변을 유지함.

// pop_head()가 head를 업데이트하면서 노드를 삭제하고나면, mutex의 잠금이 풀리게된다.
// try_pop()은 데이터를 거내고 삭제해 버린다

// 예외들은 흥미롭다
// 데이터 할당 패턴을 변경해서, 예외들은 다른 곳에서 발생한다.
// 예외를 던지는 try_pop()의 연산은 뮤텍스를 잠금이다.
// 데이터는 잠금을 요청할때까지 수정되지 않는다.
// 그러므로 try_pop()은 exception-safe하다
// 반면에 push()는 힙에 새 T 인스턴스를 할당하고, 새 노드 인스턴스 할당하는 과정에서 예외를 던질 수 있다
// 하지만 둘다 스마트포인터를 사용하기 때문에, 예외가 던져지면 해제된다.
// 그렇게 되면 잠금을 걸고나서 할일이 없다.
// 고로 push()도 exception-safe하다