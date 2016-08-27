#include <exception>
#include <memory> // for std::shared_ptr<>

struct empty_stack: std::exception
{
    const char* what() const throw();
};

template<typename T>
class threadsafe_stack
{
public:
    threadsafe_stack();
    threadsafe_stack(const threadsafe_stack&);
    // Assignment operator is deleted
	// 그리고 swap 함수가 없어서, 스택이 스스로 할당될 순 없음
    threadsafe_stack& operator=(const threadsafe-stack&) = delete;

    void push(T new_value);
	// pop 오버로드 2개
    std::shared_ptr<T> pop();	//return std::shard_ptr<>
    void pop(T& value);			//값의 reference를 취함
    bool empty() const;
};