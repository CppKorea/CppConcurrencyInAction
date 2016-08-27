// [Listing 4.16] 
// The "getting_pin" state function for the simple ATM implementation
// 단순한 ATM 구현을 위한 [핀 번호 입력] 상태 함수

// ATM 클래스 멤버함수 :: [핀 번호 입력]
void atm::getting_pin()
{
	// 입력을 기다리면서..
	incoming.wait()
		// 1. 하드웨어에서 번호가 눌리면 동작하는 핸들러(handle)
		.handle<digit_pressed>(
			// 람다함수로 처리
			[&](digit_pressed const& msg)
	{
		// 핀 번호 길이는 4글자.
		unsigned const pin_length = 4;
		// 하드웨어에서 전달한 번호를 추가.
		pin += msg.digit;

		// 4글자 모두 채워졌을 경우
		if (pin.length() == pin_length){	
			// 은행에 account, pin, incoming을 인자로하는
			// verify_pin() '작업'을 전달.
			bank.send(verify_pin(account, pin, incoming));

			// [핀 인증] 상태로 전이
			state = &atm::verifying_pin;
		}
	}
			)
		// 2. 마지막 글자를 지우는 동작 핸들러
		.handle<clear_last_pressed>(
			[&](clear_last_pressed const& msg)
	{
		// 현재까지 입력받은 핀 번호 길이를 확인
		// 0글자가 아니면
		if (!pin.empty()){
			// 1글자 감소
			pin.resize(pin.length() - 1);
		}
		// 상태 전이 없음
	}
			)
		// 3. 취소 버튼이 눌리면 동작하는 핸들러
		.handle<cancel_pressed>(
			[&](cancel_pressed const& msg)
	{
		// [진행 완료] 상태로 전이
		state = &atm::done_processing;
	}
	);
}
