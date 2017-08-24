#include "cdc_define.h"

#pragma once

namespace chinese_dark_chess
{
	//action generator.
	class ActionGenerator
	{
	private:
		const State& _state;
		const PieceType _self_pawn_id;
		const PieceType _enemy_pawn_id;
		ActionSet _action_set;

	private:
		//添加动作
		inline void AddAction(Action&& action)
		{
			_action_set.push(action);
		}

		void GennerateAllMoveAction()
		{
			BitBoard empty_board = _state.get_empty_piece_board();
			//get all allowed move
			for (uint8_t p = 0; p < 7; p++)
			{
				uint8_t piece = (uint8_t)_self_pawn_id + p;
				if (_state.piece_board(piece).any())
				{
					for (size_t i = 0; i < 32; i++)
					{
						if (_state.piece_board(piece)[i] == true)
						{
							if (empty_board[g_MOVEABLE_INDEX[i][0]] == true) { AddAction({ MOVE_ACTION,i,g_MOVEABLE_INDEX[i][0],PieceType(piece) }); }
							if (empty_board[g_MOVEABLE_INDEX[i][1]] == true) { AddAction({ MOVE_ACTION,i,g_MOVEABLE_INDEX[i][1],PieceType(piece) }); }
							if (empty_board[g_MOVEABLE_INDEX[i][2]] == true) { AddAction({ MOVE_ACTION,i,g_MOVEABLE_INDEX[i][2],PieceType(piece) }); }
							if (empty_board[g_MOVEABLE_INDEX[i][3]] == true) { AddAction({ MOVE_ACTION,i,g_MOVEABLE_INDEX[i][3],PieceType(piece) }); }
						}
					}
				}
			}
		}

		void GennerateAllCaptureAction()
		{
			//get all capture action.
			
			//PAWN
			BitBoard captureable_board = _state.piece_board(_enemy_pawn_id + 6);
			if (_state.piece_board(_self_pawn_id).any())
			{
				for (size_t i = 0; i < 32; i++)
				{
					if (_state.piece_board(_self_pawn_id)[i] == true)
					{
						if (captureable_board[g_MOVEABLE_INDEX[i][0]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][0],_self_pawn_id }); }
						if (captureable_board[g_MOVEABLE_INDEX[i][1]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][1],_self_pawn_id }); }
						if (captureable_board[g_MOVEABLE_INDEX[i][2]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][2],_self_pawn_id }); }
						if (captureable_board[g_MOVEABLE_INDEX[i][3]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][3],_self_pawn_id }); }
					}
				}
			}

			//CANNON ~ MINISTER
			captureable_board = _state.piece_board(_enemy_pawn_id);
			for (uint8_t i = 1; i < 6; i++)
			{
				uint8_t piece = _self_pawn_id + i;
				if (_state.piece_board(piece).any())
				{
					for (size_t i = 0; i < 32; i++)
					{
						if (_state.piece_board(piece)[i] == true)
						{
							if (captureable_board[g_MOVEABLE_INDEX[i][0]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][0],PieceType(piece) }); }
							if (captureable_board[g_MOVEABLE_INDEX[i][1]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][1],PieceType(piece) }); }
							if (captureable_board[g_MOVEABLE_INDEX[i][2]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][2],PieceType(piece) }); }
							if (captureable_board[g_MOVEABLE_INDEX[i][3]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][3],PieceType(piece) }); }
						}
					}
				}
				captureable_board |= _state.piece_board(piece);
			}

			//KING
			captureable_board &= (~_state.piece_board(_self_pawn_id));//remove pawn
			PieceType king = PieceType(_self_pawn_id + 6);
			if (_state.piece_board(king).any())
			{
				for (size_t i = 0; i < 32; i++)
				{
					if (_state.piece_board(_self_pawn_id + 6)[i] == true)
					{
						if (captureable_board[g_MOVEABLE_INDEX[i][0]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][0],king }); }
						if (captureable_board[g_MOVEABLE_INDEX[i][1]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][1],king }); }
						if (captureable_board[g_MOVEABLE_INDEX[i][2]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][2],king }); }
						if (captureable_board[g_MOVEABLE_INDEX[i][3]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][3],king }); }
					}
				}
			}
		}

		void GennerateAllFlippingAction()
		{
			if (_state.piece_board(0).any())
			{
				for (size_t i = 0; i < 32; i++)
				{
					//i为未翻开的棋子
					if (_state.piece_board(0)[i] == true)
					{
						AddAction({ FLIPPING_ACTION, i, i, PIECE_UNKNOWN });
					}
				}
			}
		}

		void GennerateAllAction()
		{
			//存在未确定的棋子。
			GennerateAllMoveAction();
			GennerateAllCaptureAction();
			GennerateAllFlippingAction();
		}

	public:
		
		//default constructor
		ActionGenerator(const State & state) :
			_state(state),
			_self_pawn_id(state.last_player() == PLAYER_RED ? PIECE_RED_PAWN : PIECE_BLACK_PAWN),
			_enemy_pawn_id(state.last_player() == PLAYER_RED ? PIECE_BLACK_PAWN : PIECE_RED_PAWN),
			_action_set()
		{
			GennerateAllAction();
		}

		//get random action
		const Action& random_action() const
		{
			return _action_set.random_action();
		}
		
		//print all actions.
		void print() const
		{
			gadt::table::ConsoleTable table(4, _action_set.size() + 1);
			table.set_cell_in_row(0, { 
				{ "type", gadt::console::YELLOW },
				{ "source", gadt::console::BLUE },
				{ "dest", gadt::console::BLUE },
				{ "piece", gadt::console::GREEN }
			});
			table.set_width({ 3,3,3,3 });
			for (size_t i = 0; i < _action_set.size(); i++)
			{
				table.set_cell_in_row(i + 1, {
					{ gadt::console::IntergerToString(_action_set.action(i).type)},
					{ gadt::console::IntergerToString(_action_set.action(i).source) },
					{ gadt::console::IntergerToString(_action_set.action(i).dest) },
					{ gadt::console::IntergerToString(_action_set.action(i).new_piece) }
				});
			}
			table.print();
		}
	};
}