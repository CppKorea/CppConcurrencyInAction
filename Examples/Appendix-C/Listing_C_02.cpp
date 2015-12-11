// Listing C.2 The sender class

// Author	:
//		C++ Korea Concurrency Study Group
// Source Code : 
//		from	Appendix C	[C++ Concurrency in Action] 

namespace messaging
{
	class sender
	{
		queue*q;
	public:
		sender() :
			q(nullptr)
		{}
		explicit sender(queue*q_) :
			q(q_)
		{}
		template<typename Message>
		void send(Message const& msg)
		{
			if (q)
			{
				q->push(msg);
			}
		}
	};
}
