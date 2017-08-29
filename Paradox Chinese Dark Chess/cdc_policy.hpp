#include "cdc_define.h"

#pragma once

namespace chinese_dark_chess
{
	constexpr const bool g_CDC_POLICY_CHECK = true;

	enum ActionGeneratorPolicy: uint8_t
	{
		DEFAULT_POLICY = 0,
		GREEDY_POLICY = 1,
		NO_FLIPPING_POLICY =2
	};

	//action generator.
	class ActionGenerator
	{
	private:
		const State& _state;
		const bool _exist_undecided;
		const PieceType _self_pawn_id;
		const PieceType _enemy_pawn_id;
		ActionSet _action_set;
	
	private:
		//添加动作
		inline void AddAction(Action&& action)
		{
			_action_set.push(action);
		}

		void GenerateAllMoveAction()
		{
			BitBoard empty_board = _state.get_empty_piece_board();
			//get all allowed move
			for (uint8_t p = 0; p < 7; p++)
			{
				if (p != 1)//skip cannon.
				{
					uint8_t piece = (uint8_t)_self_pawn_id + p;
					if (_state.piece_board(piece).any())
					{
						for (size_t i = 0; i < g_CDC_MAX_LENGTH; i++)
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
		}

		void GenerateAllCannonFlyAction()
		{
			//cannon fly.
			const BitBoard& cannon_board = _state.piece_board(_self_pawn_id + 1);
			const BitBoard exist_piece_board = _state.get_exist_piece_board();
			BitBoard capturable_board = _state.piece_board(PIECE_UNKNOWN);
			for (size_t i = 0; i < 7; i++)
			{
				capturable_board |= _state.piece_board(_enemy_pawn_id + i);
			}

			if (cannon_board.any())
			{
				for (size_t i = 0; i < g_CDC_MAX_LENGTH; i++)
				{
					if (cannon_board[i] == true)
					{
						constexpr int dir[4] = { 1,-1,8,-8 };
						for (size_t n = 0; n < 4; n++)
						{
							int loc = (int)i;
							bool found_first = false;
							for (;;)
							{
								loc += dir[n];//get new location.
								if (loc < 0 || loc >= 32)
									break;
								if (exist_piece_board[loc] == true)
								{
									if (!found_first)
										found_first = true;
									else
									{
										if (capturable_board[loc] == true)
										{
											AddAction({ CAPTURE_ACTION,i,loc,PieceType(_self_pawn_id + 1) });
										}
										break;
									}
								}
							}
						}
					}
				}
			}
		}

		void GenerateAllCaptureAction()
		{
			//get all capture action.

			//PAWN ~ MINISTER
			BitBoard captureable_board = _state.piece_board(_enemy_pawn_id + 6);
			for (uint8_t i = 0; i < 7; i++)
			{
				uint8_t piece = _self_pawn_id + i;
				const BitBoard& equal_enemy_board = _state.piece_board(_enemy_pawn_id + i);

				//remove all PAWNS if it is king.
				if (i == 6)
				{
					captureable_board &= (~_state.piece_board(_enemy_pawn_id));//remove pawn
				}

				//add equal pieces.
				captureable_board |= equal_enemy_board; //add equal pieces.

				//skip CANNON.
				if (_state.piece_board(piece).any() && captureable_board.any() && i != 1)
				{
					for (size_t i = 0; i < g_CDC_MAX_LENGTH; i++)
					{
						if (_state.piece_board(piece)[i] == true)
						{
							//add capture
							if (captureable_board[g_MOVEABLE_INDEX[i][0]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][0],PieceType(piece) }); }
							if (captureable_board[g_MOVEABLE_INDEX[i][1]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][1],PieceType(piece) }); }
							if (captureable_board[g_MOVEABLE_INDEX[i][2]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][2],PieceType(piece) }); }
							if (captureable_board[g_MOVEABLE_INDEX[i][3]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][3],PieceType(piece) }); }

							//add both-capture
							if (equal_enemy_board[g_MOVEABLE_INDEX[i][0]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][0],PIECE_EMPTY }); }
							if (equal_enemy_board[g_MOVEABLE_INDEX[i][1]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][1],PIECE_EMPTY }); }
							if (equal_enemy_board[g_MOVEABLE_INDEX[i][2]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][2],PIECE_EMPTY }); }
							if (equal_enemy_board[g_MOVEABLE_INDEX[i][3]] == true) { AddAction({ CAPTURE_ACTION,i,g_MOVEABLE_INDEX[i][3],PIECE_EMPTY }); }
						}
					}
				}

				//clear all KING if it is PAWN.
				if (i == 0)
				{
					captureable_board = equal_enemy_board;//clear all kings.
				}
			}

			GenerateAllCannonFlyAction();
		}

		void GenerateAllFlippingAction()
		{
			if (_state.piece_board(0).any())
			{
				for (size_t i = 0; i < g_CDC_MAX_LENGTH; i++)
				{
					//i为未翻开的棋子
					if (_state.piece_board(0)[i] == true)
					{
						AddAction({ FLIPPING_ACTION, i, i, PIECE_UNDECIDED });
					}
				}
			}
		}

		void GenerateActionsByPolicy(ActionGeneratorPolicy policy)
		{
			switch (policy)
			{
			case chinese_dark_chess::DEFAULT_POLICY:
				GenerateAllCaptureAction();
				GenerateAllMoveAction();
				GenerateAllFlippingAction();
				break;
			case chinese_dark_chess::GREEDY_POLICY:
				GenerateAllCaptureAction();
				if (size() == 0)
				{
					GenerateAllMoveAction();
					GenerateAllFlippingAction();
				}
				break;
			case chinese_dark_chess::NO_FLIPPING_POLICY:
				GenerateAllCaptureAction();
				GenerateAllMoveAction();
				break;
			default:
				break;
			}
			
		}

		void AllFlipedResultAction()
		{
			//find piece location.
			size_t loc = 100;
			for (size_t i = 0; i < _state.piece_board(PIECE_UNDECIDED).upper_bound(); i++)
			{
				if (_state.piece_board(PIECE_UNDECIDED)[i] == true)
				{
					loc = i;
					break;
				}
			}
			GADT_CHECK_WARNING(g_CDC_POLICY_CHECK, loc == 100, "fail to find undecied piece");

			//generate actions.
			const HiddenPiece& hidden = _state.hidden_pieces();
			for (size_t i = 0; i < hidden.upper_bound(); i++)
			{
				if (hidden[i] > 0)
				{
					_action_set.push(Action(FLIPPED_RESULT_ACTION, loc, loc, PieceType(i)), hidden[i]);
				}
			}
		}

		void AllRemoveHiddenAction()
		{
			const HiddenPiece& hidden = _state.hidden_pieces();
			for (size_t i = 0; i < hidden.upper_bound(); i++)
			{
				if (hidden[i] > 0)
				{
					_action_set.push(Action(REMOVE_HIDDEN_ACTION, 63, 63, PieceType(i)), hidden[i]);
				}
			}
		}

	public:
		
		//default constructor
		ActionGenerator(const State & state, ActionGeneratorPolicy policy = DEFAULT_POLICY) :
			_state(state),
			_exist_undecided(state.exist_undecided_piece()),
			_self_pawn_id(state.next_player() == PLAYER_RED ? PIECE_RED_PAWN : PIECE_BLACK_PAWN),
			_enemy_pawn_id(state.next_player() == PLAYER_RED ? PIECE_BLACK_PAWN : PIECE_RED_PAWN),
			_action_set(!state.exist_undecided_piece())
		{
			if (_exist_undecided)
			{
				AllFlipedResultAction();
			}
			else if (state.remove_hidden_flag())
			{
				AllRemoveHiddenAction();
			}
			else
			{
				GenerateActionsByPolicy(policy);
			}
		}

		//get random action
		const Action& random_action() const
		{
			return _action_set.random_action();
		}
		
		//print all actions.
		void print() const
		{
			gadt::table::ConsoleTable table(5, _action_set.size() + 1);
			table.set_cell_in_row(0, { 
				{ "index", gadt::console::GRAY},
				{ "type", gadt::console::YELLOW },
				{ "source", gadt::console::BLUE },
				{ "dest", gadt::console::RED },
				{ "piece", gadt::console::GREEN }
			});
			table.set_width({ 3,5,3,3,3 });
			for (size_t i = 0; i < _action_set.size(); i++)
			{
				table.set_cell_in_row(i + 1, {
					{ gadt::console::IntergerToString(i) },
					{ print::ActionTypeToStr(_action_set.action(i).type)},
					{ Location(_action_set.action(i).source).str() },
					{ Location(_action_set.action(i).dest).str() },
					{ print::PieceToStr(_action_set.action(i).piece),print::PieceToColor(_action_set.action(i).piece) }
				});
			}
			table.print(true, false);
		}

		//get size.
		size_t size() const
		{
			return _action_set.size();
		}

		inline const Action& action(size_t index) const
		{
			return _action_set.action(index);
		}

		inline const Action& operator[](size_t index) const
		{
			return action(index);
		}
	};
}