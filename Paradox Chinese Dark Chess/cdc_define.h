#include "include\gadtlib.h"
#include "include\bitboard.hpp"
#include "include\gameshell.h"
#include "include\mcts_new.hpp"
#include "include\minimax.hpp"
#include "include\visual_tree.h"

#pragma once

#ifdef GADT_DEBUG_INFO
	#define CDC_DEBUG_INFO
#endif

namespace chinese_dark_chess
{
	constexpr const bool g_CDC_DEFINE_CHECK = true;
	constexpr const size_t g_CDC_BOARD_WIDTH = 8;
	constexpr const size_t g_CDC_BOARD_HEIGHT = 4;
	constexpr const size_t g_CDC_MAX_LENGTH = g_CDC_BOARD_WIDTH * g_CDC_BOARD_HEIGHT;

	//index of players.
	enum PlayerIndex : uint8_t
	{
		PLAYER_RED = 0,
		PLAYER_BLACK = 1
	};
	
	//piece index, equal to PieceRank + PlayerIndex * 7
	enum PieceType : uint8_t
	{
		//0 unknown
		//1~7 red pieces
		//8~14 black pieces
		//15 empty

		PIECE_UNKNOWN = 0,
		PIECE_RED_PAWN = 1,
		PIECE_RED_CANNON = 2,
		PIECE_RED_KNIGHT = 3,
		PIECE_RED_ROOK = 4,
		PIECE_RED_MINISTER = 5,
		PIECE_RED_GUARD = 6,
		PIECE_RED_KING = 7,
		PIECE_BLACK_PAWN = 8,
		PIECE_BLACK_CANNON = 9,
		PIECE_BLACK_KNIGHT = 10,
		PIECE_BLACK_ROOK = 11,
		PIECE_BLACK_MINISTER = 12,
		PIECE_BLACK_GUARD = 13,
		PIECE_BLACK_KING = 14,
		PIECE_EMPTY = 15
	};

	//action types.
	enum ActionType: uint8_t
	{
		MOVE_ACTION = 0,			//move without caputure
		CAPTURE_ACTION = 1,			//capture a piece
		FLIPPING_ACTION = 2,		//flip a piece but not decide what piece it is
		FLIPPED_RESULT_ACTION = 3	//flip a piece and decide what it is.
	};

	//the result of a state.
	enum Result:uint8_t
	{
		RESULT_RED_WIN = 0,
		RESUKT_BLACK_WIN = 1,
		RESULT_DRAW = 2,
		RESULT_UNFINISH = 3
	};

	//bitboard of the pieces.
	using BitBoard = gadt::bitboard::BitBoard64;

	//array of pieces.
	using PieceArray = gadt::bitboard::ValueVector<g_CDC_MAX_LENGTH>;
	
	//basic data struct of game state.
	class State;

	//location.
	struct Location
	{
		size_t x;
		size_t y;

		template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
		Location(T _x, T _y): x(static_cast<size_t>(_x)), y(static_cast<size_t>(_y))
		{
			GADT_CHECK_WARNING(g_CDC_DEFINE_CHECK, _x < g_CDC_BOARD_WIDTH, "out of width");
			GADT_CHECK_WARNING(g_CDC_DEFINE_CHECK, _y < g_CDC_BOARD_HEIGHT, "out of height");
		}

		template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
		Location(T index):x(index % g_CDC_BOARD_WIDTH),y(index / g_CDC_BOARD_WIDTH)
		{
			GADT_CHECK_WARNING(g_CDC_DEFINE_CHECK, index < g_CDC_MAX_LENGTH, "out of index");
		}
	};

	/*
	* ActionData is the details about one action.
	* piece would be moved from source location to destination location.
	* if the action type is FLIPPING_ACTION, the new_piece is PIECE_UNKNOWN
	*/
	struct ActionData
	{
		const ActionType	type;
		const Location		source_loc;
		const Location		dest_loc;
		const PieceType		new_piece;

		ActionData(ActionType _type, Location _source_loc, Location _dest_loc, PieceType _new_piece):
			type(_type),
			source_loc(_source_loc),
			dest_loc(_dest_loc),
			new_piece(_new_piece)
		{
		}
	};

	//data of state.
	class StateData
	{
	private:
		PieceType _data[g_CDC_BOARD_WIDTH][g_CDC_BOARD_HEIGHT];

	public:
		//to next by action data.
		void to_next(const ActionData& action)
		{
			_data[action.source_loc.x][action.source_loc.y] = PIECE_EMPTY;
			_data[action.dest_loc.x][action.dest_loc.y] = action.new_piece;
		}

		//updata data by State.
		void update(const State& state);
	};

	/*
	* Action contains info about how to change the state:
	* 1. MOVE_ACTION 
	*	move_index: not 0 
	*	move_board: new board after move
	*	capture_index = move_index 
	*	capture_board = move_board
	*
	* 2. CAPTURE_ACTION
	*	move_index: not 0 
	*	move_board: new board after capture
	*	capture_index: not 0
	*	capture_board: new board after capture
	*
	* 3. FLIPPING_ACTION
	*	move index:0 
	*	move_board: new board without flipping piece.
	*	capture_index = move_index 
	*	capture_board = move_board
	*
	* 4. FLIPPED_RESULT_ACTION
	*	move index:0 
	*	move_board: new board without flipped piece.
	*	capture_index : not 0 
	*	capture_board = new board with flipped piece.
	*/
	struct Action
	{
		const PieceType	move_index;		//piece index, from 0 to 14
		const BitBoard	move_board;		//the new board of moved piece rank.
		const PieceType	capture_index;	//the index of captured index, no capture if the value is 0.
		const BitBoard	capture_board;	//the new board for captured piece rank.

#ifdef CDC_DEBUG_INFO
		ActionData debug_data;			//debug data.
#endif

		//get action type
		ActionType type() const
		{
#ifdef CDC_DEBUG_INFO
			return debug_data.type;
#else
			if (move_index != 0)
			{
				if (move_index == capture_index)
				{
					return MOVE_ACTION;
				}
				return CAPTURE_ACTION;
			}
			if (capture_index != 0)
			{
				return FLIPPED_RESULT_ACTION;
			}
			return FLIPPING_ACTION;
#endif
		}

		ActionData data(const State& s) const
		{
#ifdef CDC_DEBUG_INFO
			return debug_data;
#else
			auto GetIndex = [](const BitBoard& board)->size_t {
				for (size_t i = 0; i < g_CDC_MAX_LENGTH; i++)
				{
					if (board[i]) 
					{ 
						return i; 
					}
				}
				return g_CDC_MAX_LENGTH;
			};
			auto action_type = type();
			if (action_type == MOVE_ACTION)
			{
				//from A to B
				BitBoard a_and_b = move_board ^ s.piece_board(move_index);
				BitBoard a = a_and_b & s.piece_board(move_index);//source
				BitBoard b = a_and_b & move_board;//destination
				size_t source = GetIndex(a);
				size_t destination = GetIndex(b);
				return ActionData(MOVE_ACTION, Location(source), Location(destination), move_index);
			}
			if (action_type == CAPTURE_ACTION)
			{
				//from A to B
				BitBoard a_and_b = move_board ^ s.piece_board(move_index);
				BitBoard b = (~capture_board) & s.piece_board(capture_index);
				BitBoard a = a_and_b ^ b;
				size_t source = GetIndex(a);
				size_t destination = GetIndex(b);
				return ActionData(CAPTURE_ACTION, Location(source), Location(destination), move_index);
			}
			if (action_type == FLIPPING_ACTION)
			{
				//from A
				BitBoard a = move_board ^ s.piece_board(move_index);
				size_t source = GetIndex(a);
				return ActionData(FLIPPING_ACTION, Location(source), Location(source), move_index);
			}
			
			//flipped result action
			BitBoard a = move_board ^ s.piece_board(move_index);
			size_t source = GetIndex(a);
			return ActionData(FLIPPED_RESULT_ACTION, Location(source), Location(source), capture_index);
#endif
		}
	};

	//action set
	using ActionSet = std::vector<Action>;

	//basic data struct of game state.
	class State
	{
	private:
		BitBoard	_pieces[15];		//pieces.
		PieceArray	_hidden_pieces[2];  //pieces that still no be flipped.
		PlayerIndex _last_player;		//index of last moved player.
		size_t		_no_capture_count;	//the count of no capture, draw if the value more than 20.

#ifdef CDC_DEBUG_INFO
		StateData	_debug_data;		//debug data
#endif

	public:
		template <typename T>const BitBoard& piece_board(T id) const { return _pieces[id]; }
		const PieceArray& hidden_pieces(PlayerIndex index) const { return _hidden_pieces[index]; }
		PlayerIndex last_player() const { return _last_player; }
		size_t no_capture_count() const { return _no_capture_count; }

	public:
		
		//default constructor, generate a new state.
		State():
			_hidden_pieces
		{ 
				{1,1,1,1,1,2,2,3,3,4,4,5,5,6,6,7},
				{8,8,8,8,8,9,9,10,10,11,11,12,12,13,13,14} 
		},
			_last_player(PLAYER_RED),
			_no_capture_count(0)
		{
			_pieces[0] = BitBoard(4294967295);
			_debug_data.update(*this);
		}

		//to next state by an action.
		inline void to_next(const Action& action) 
		{
			GADT_CHECK_WARNING(g_CDC_DEFINE_CHECK, action.type() != FLIPPING_ACTION, "excute flipping act");

			_pieces[action.capture_index] = action.capture_board;
			_pieces[action.move_index] = action.capture_board;

#ifdef CDC_DEBUG_INFO
			_debug_data.to_next(action.debug_data);
#endif
		}

		//get result of the state.
		Result get_result() const
		{
			if (_no_capture_count > 20)
			{
				return RESULT_DRAW;
			}
			bool no_red =
				_hidden_pieces[PLAYER_RED].is_empty() &&
				_pieces[1].none() &&
				_pieces[2].none() &&
				_pieces[3].none() &&
				_pieces[4].none() &&
				_pieces[5].none() &&
				_pieces[6].none() &&
				_pieces[7].none();
			if (no_red) { return RESUKT_BLACK_WIN; }

			bool no_black =
				_hidden_pieces[PLAYER_BLACK].is_empty() &&
				_pieces[8].none() &&
				_pieces[9].none() &&
				_pieces[10].none() &&
				_pieces[11].none() &&
				_pieces[12].none() &&
				_pieces[13].none() &&
				_pieces[14].none();
			if (no_black) { return RESULT_RED_WIN; }
			return RESULT_DRAW;
		}

	};

	//print something
	namespace print
	{
		//print piece
		void piece(PieceType p);

		//print player.
		void player(PlayerIndex p);

		//print action
		void action(const Action& act, const State& s);

		//print state
		void state(const State& s);
	}
}
