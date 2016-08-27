// [Listing 4.15] A simple implementation of an ATM logic class
// ATM Logic 클래스의 간단한 구현

struct card_inserted
{
	std::string account;
};

// ATM 클래스
class atm
{
	// 메시징 라이브러리
	messaging::receiver incoming;
	messaging::sender bank;
	messaging::sender interface_hardware;

	// ATM의 멤버 함수로 이어지는 함수 포인터.
	// ATM의 상태 == 현재 실행중인 함수
	void (atm::*state)();

	std::string account;
	std::string pin;

	// 1. 함수 == 상태.
	//    [카드 삽입 대기] 루틴
	void waiting_for_card()
	{
		// 2. 하드웨어에서는 카드 삽입 요청을 보여주면서...
		interface_hardware.send(display_enter_card());

		// 3. 입력을 기다린다.
		incoming.wait()
			// 카드 삽입 이벤트 처리.
			.handle<card_inserted>(
				// 4. 람다 함수를 사용.
				//    참조로 메세지를 받아서 처리.
				[&](card_inserted const& msg)
		{
			// 계좌번호 확인
			account = msg.account;
			pin = "";
			// 핀 번호를 입력하도록 하드웨어 화면을 변경
			interface_hardware.send(display_enter_pin());
			// [핀 번호 입력] 상태로 전이
			state = &atm::getting_pin;
		}
		);
	}
	// [핀 번호 입력] 함수(상태)
	void getting_pin();

public:
	// 5. 실행. main()에서 호출
	void run()
	{
		// 6. ATM의 최초 상태는 [카드 삽입 대기]
		state = &atm::waiting_for_card;
		try{
			// 무한루프를 돌면서...
			for (;;){
				// 7. 함수(상태)를 실행
				(this->*state)();
			}
		}
		// 별도의 예외처리는 없음.
		catch (messaging::close_queue const&)
		{}
	}
};
