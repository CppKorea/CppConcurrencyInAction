// Listing C.3 The receiver class

// Author	:
//		C++ Korea Concurrency Study Group
// Source Code : 
//		from	Appendix C	[C++ Concurrency in Action] 

namespace messaging
{
	class receiver
	{
		queue q;
	public:
		operator sender()
		{
			return sender(&q);
		}
		dispatcher wait()
		{
			return dispatcher(&q);
		}
	};
}
