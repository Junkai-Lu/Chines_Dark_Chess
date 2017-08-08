#include "cdc_define.h"

#pragma once

namespace chinese_dark_chess
{
	//action generator.
	class ActionGenerator
	{
	private:
		const State& _state;
		ActionSet _action_set;

	private:
		void generate()
		{
			//flipping pieces.
			for (size_t i = 0; i < 32; i++)
			{

			}
		}

	public:
		
		//default constructor
		ActionGenerator(const State & state) :
			_state(state),
			_action_set()
		{
			generate();
		}


	};
}