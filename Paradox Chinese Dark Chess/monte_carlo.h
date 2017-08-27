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
					return result;
				
				ActionGenerator actions(state);
				state.to_next(actions.random_action());
			}
			return Result();
		}

	public:
		MonteCarlo(const State& state) :
			_state(state),
			_actions(state)
		{
		}

		const Action& DoMonteCarlo(size_t simulation_times) const
		{
			const size_t single_action_simu_time = 1 + (simulation_times / _actions.size());
			std::vector<double> values(_actions.size(), 0.0);
			for (size_t i = 0; i < _actions.size(); i++)
			{
				for (size_t n = 0; n < single_action_simu_time; n++)
				{

				}
			}
		}

		const Action& DoFlatMonteCarlo(size_t simulation_times) const
		{

		}
	};
}
