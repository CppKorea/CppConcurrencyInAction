#include <atomic>
#include <cstddef>
#include <memory>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <time.h>

#define THREAD_COUNT    100
#define TEST_COUNT      100000

template<typename T>
class threadsafe_queue
{
private:
    struct node
    {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };
    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node* tail;
    std::condition_variable data_cond;
public:
    threadsafe_queue():
        head(new node),tail(head.get())
    {}
    threadsafe_queue(const threadsafe_queue& other)=delete;
    threadsafe_queue& operator=(const threadsafe_queue& other)=delete;

    void push(T new_value)
    {
        std::shared_ptr<T> new_data(
            std::make_shared<T>(std::move(new_value)));
        std::unique_ptr<node> p(new node);
        {
            std::lock_guard<std::mutex> tail_lock(tail_mutex);
            tail->data=new_data;
            node* const new_tail=p.get();
            tail->next=std::move(p);
            tail=new_tail;
        }
        data_cond.notify_one();
    }

private:
    node* get_tail()
    {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }
    std::unique_ptr<node> pop_head()
    {
        std::unique_ptr<node> old_head=std::move(head);
        head=std::move(old_head->next);
        return old_head;
    }
    std::unique_lock<std::mutex> wait_for_data()
    {
        std::unique_lock<std::mutex> head_lock(head_mutex);
        data_cond.wait(head_lock,[&]{return head.get()!=get_tail();});
        return std::move(head_lock);
    }
    std::unique_ptr<node> wait_pop_head()
    {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        return pop_head();
    }
    std::unique_ptr<node> wait_pop_head(T& value)
    {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        value=std::move(*head->data);
        return pop_head();
    }
public:
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_ptr<node> const old_head=wait_pop_head();
        return old_head->data;
    }
    void wait_and_pop(T& value)
    {
        std::unique_ptr<node> const old_head=wait_pop_head(value);
    }

private:
    std::unique_ptr<node> try_pop_head()
    {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if(head.get()==get_tail())
        {
            return std::unique_ptr<node>();
        }
        return pop_head();
    }
    std::unique_ptr<node> try_pop_head(T& value)
    {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if(head.get()==get_tail())
        {
            return std::unique_ptr<node>();
        }
        value=std::move(*head->data);
        return pop_head();
    }

public:
    std::shared_ptr<T> try_pop()
    {
        std::unique_ptr<node> old_head=try_pop_head();
        return old_head?old_head->data:std::shared_ptr<T>();
    }
    bool try_pop(T& value)
    {
        std::unique_ptr<node> const old_head=try_pop_head(value);
        return old_head;
    }
    void empty()
    {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        return (head.get()==get_tail());
    }
};

void test( int                             aThreadID,
           threadsafe_queue<std::string> * aThreadSafeQueue,
           int                             aCount )
{

    for ( int i = 0; i < aCount; i++ )
    {
        aThreadSafeQueue->push( "TEST1234567890" );

        //if ( ( i % 1000 ) == 0 )
        //{
        //    std::cout << aThreadID << " Thread : " << i << " are tested" << std::endl;
        //}
    }

    for ( int i = 0; i < aCount; i++ )
    {
        aThreadSafeQueue->try_pop();

        //if ( ( i % 1000 ) == 0 )
        //{
        //    std::cout << aThreadID << " Thread : " << i << " are tested" << std::endl;
        //}
    }
}

int main( int argc, char * argv[] )
{
    if ( argc == 3 )
    {
        int                           sThreadCount = atoi( argv[1] );
        int                           sTestCount = atoi( argv[2] );
        threadsafe_queue<std::string> sThreadSafeQueue;
        std::thread                 * sThread = new std::thread[sThreadCount];

        clock_t start, end;

        start = clock();
        for ( int i = 0; i < sThreadCount; i++ )
        {
            sThread[i] = std::thread( test,
                                      i,
                                      &sThreadSafeQueue,
                                      sTestCount );
        }

        for ( int i = 0; i < sThreadCount; i++ )
        {
            sThread[i].join();
        }
        end = clock();

        std::cout << ( end - start ) / CLOCKS_PER_SEC << std::endl;
    }

    return 0;
}
