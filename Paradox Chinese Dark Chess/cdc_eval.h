#include "cdc_define.h"

#pragma once

namespace chinese_dark_chess
{
	enum StateProcessType : uint8_t
	{
		STATE_BEGINING = 0,
		STATE_BATTLE = 1,
		STATE_DEAD = 2
	};

	class StateEval
	{
	private:
		const State& _state;

	public:
		StateEval(const State& state):
			_state(state)
		{
		}
	};
}
