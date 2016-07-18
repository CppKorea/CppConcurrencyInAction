// [ Listing C.1 ]
// A simple message queue

// Source : 
//    p.342  [C++ Concurrency in Action] 

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

namespace messaging
{

    // Base class for our queue entries
    struct message_base
    {
        virtual ~message_base()
        {}
    };

    // Each message type has a specialization
    template<typename Msg>
    struct wrapped_message:
        message_base
    {
        Msg contents;
        explicit wrapped_message(Msg const& contents_):
        contents(contents_)
        {}
    };

    // Our message queue
    class queue
    {
        std::mutex m;
        std::condition_variable c;
        // Actual queue stores pointer to message_base
        std::queue<std::shared_ptr<message_base> > q;
    public:
        template<typename T>
        void push(T const& msg)
        {
            std::lock_guard<std::mutex> lk(m);
            // Wrap posted message and store pointer
            q.push(std::make_shared<wrapped_message<T> >(msg));
            c.notify_all();
        }
    
        std::shared_ptr<message_base> wait_and_pop()
        {
            std::unique_lock<std::mutex> lk(m);
            // Block until queue isn't empty
            c.wait(lk,[&]{return !q.empty();});
            auto res=q.front();
            q.pop();
            return res;
        }
    };  // class queue
  
}   // namespace messaging
