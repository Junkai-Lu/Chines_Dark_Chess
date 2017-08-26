#pragma once

#include "cdc_policy.hpp"

namespace chinese_dark_chess
{
	class MonteCarlo
	{
	private:

		const State&			_state;
		const ActionGenerator	_actions;

	private:

		Result Simulation(const State& start)
		{
			State state = start;
			for (;;)
			{
				Result result = state.get_result();
				if (result != RESULT_UNFINISH)
				{
					return result;
				}

				//to next.
				
			}
		}

	public:
		MonteCarlo(const State& state) :
			_state(state),
			_actions(state)
		{
		}

		const Action& result() const
		{
			
		}
	};
}
