#include "monte_carlo.h"

namespace chinese_dark_chess
{
	Result MonteCarlo::Simulation(const State & start) const
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

	const Action & MonteCarlo::DoMonteCarlo(size_t simulation_times) const
	{
		//constexpr bool enable_log = false;

		//return the first one if there is only one available action.
		if (_actions.size() == 1)
			return _actions[0];

		//excute simulations for each action.
		const size_t single_action_simu_time = 1 + (simulation_times / _actions.size());
		size_t best_count = 0;
		size_t best_index = 0;
		for (size_t i = 0; i < _actions.size(); i++)
		{
			size_t count = 1;
			State new_state = _state;
			new_state.to_next(_actions.action(i));
			for (size_t n = 0; n < single_action_simu_time; n++)
			{
				Result result = Simulation(new_state);
				if ((int8_t)result == (int8_t)_state.next_player())
					count++;
			}
			if (count > best_count)
			{
				best_index = 1;
				best_count = count;
			}
		}

		//select best win count action.
		return _actions[best_index];
	}
}
