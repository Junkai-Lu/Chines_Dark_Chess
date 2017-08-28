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

		Result Simulation(const State& start) const;

	public:
		MonteCarlo(const State& state) :
			_state(state),
			_actions(state)
		{
		}

		const Action& DoMonteCarlo(size_t simulation_times) const;

		/*const Action& DoFlatMonteCarlo(size_t simulation_times) const
		{

		}*/
	};
}
