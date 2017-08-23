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
		//��Ӷ���
		inline void AddAction(Action&& action)
		{
			_action_set.push(action);
		}

		void generate()
		{
			//����δȷ�������ӡ�
			if (_state.piece_board(0).any())
			{
				for (size_t i = 0; i < 32; i++)
				{
					//iΪδ����������
					if (_state.piece_board(0)[i] == true)
					{
						BitBoard temp = _state.piece_board(0);
						temp.reset(i);
						AddAction({ PIECE_UNKNOWN,temp,PIECE_UNKNOWN,temp });
					}
				}
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