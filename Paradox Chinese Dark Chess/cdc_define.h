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
	constexpr const size_t g_CDC_BITBOARD_SIZE = 17;
	constexpr const size_t g_CDC_BOARD_WIDTH = 8;
	constexpr const size_t g_CDC_BOARD_HEIGHT = 4;
	constexpr const size_t g_CDC_MAX_LENGTH = g_CDC_BOARD_WIDTH * g_CDC_BOARD_HEIGHT;

	extern size_t g_MOVEABLE_INDEX[32][4];

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
		//1 unknown
		//2~8 red pieces
		//9~15 black pieces
		//16 empty
		PIECE_UNKNOWN = 0,
		PIECE_UNDECIDED = 1,
		PIECE_RED_PAWN = 2,
		PIECE_RED_CANNON = 3,
		PIECE_RED_KNIGHT = 4,
		PIECE_RED_ROOK = 5,
		PIECE_RED_MINISTER = 6,
		PIECE_RED_GUARD = 7,
		PIECE_RED_KING = 8,
		PIECE_BLACK_PAWN = 9,
		PIECE_BLACK_CANNON = 10,
		PIECE_BLACK_KNIGHT = 11,
		PIECE_BLACK_ROOK = 12,
		PIECE_BLACK_MINISTER = 13,
		PIECE_BLACK_GUARD = 14,
		PIECE_BLACK_KING = 15,
		PIECE_EMPTY = 16,
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

		std::string str() const
		{
			std::stringstream ss;
			ss << "(" << x << "," << y << ")";
			return ss.str();
		}
	};

	/*
	* Action is the details about one action.
	* piece would be moved from source location to destination location.
	* Action contains info about how to change the state:

	* 1. MOVE_ACTION 
	*	source != dest
	*   piece != UNKNOWN
	*
	* 3. FLIPPING_ACTION
	*	source == dest
	*   piece == UNKONW
	*
	* 4. FLIPPED_RESULT_ACTION
	*	source == dest
	*   piece != UNKNOWN
	*/
	struct Action
	{
		const ActionType	type;
		const size_t        source;
		const size_t        dest;
		const PieceType		new_piece;

		Action(ActionType _type, size_t _source, size_t _dest, PieceType _new_piece):
			type(_type),
			source(_source),
			dest(_dest),
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
		//get piece by location
		PieceType piece(Location loc)
		{
			return _data[loc.x][loc.y];
		}

		//get pice by x and y
		PieceType piece(size_t x, size_t y)
		{
			return _data[x][y];
		}

		//to next by action data.
		void to_next(const Action& action)
		{
			_data[action.source % 8][action.source /8] = PIECE_EMPTY;
			_data[action.dest % 8][action.dest / 8] = action.new_piece;
		}

		//updata data by State.
		void update(const State& state);
	};

	//action set
	class ActionSet
	{
	private:
		std::vector<Action> _actions;
		
	public:
		ActionSet():
			_actions()
		{
		}

		//get the size of the ActionSet
		size_t size() const
		{
			return _actions.size();
		}

		//push new action
		void push(Action action)
		{
			_actions.push_back(action);
		}

		//get random action, return Action() if not action exist.
		const Action& random_action() const
		{
			if (size() > 0)
			{
				size_t rnd = rand() % size();
				return _actions[rnd];
			}
			GADT_CHECK_WARNING(g_CDC_DEFINE_CHECK, true, "empty action set for random action.");
		}

		//get ref of action by index.
		const Action& action(size_t index) const
		{
			return _actions[index];
		}
	};

	//basic data struct of game state.
	class State
	{
	private:
		BitBoard	_pieces[g_CDC_BITBOARD_SIZE];	//pieces.
		PieceArray	_hidden_pieces;					//pieces that still no be flipped.
		PlayerIndex _last_player;					//index of last moved player.
		size_t		_no_capture_count;				//the count of no capture, draw if the value more than 20.

#ifdef CDC_DEBUG_INFO
		StateData	_debug_data;					//debug data
#endif

	public:
		template <typename T>const BitBoard& piece_board(T id) const { return _pieces[id]; }
		const PieceArray& hidden_pieces() const { return _hidden_pieces; }
		PlayerIndex last_player() const { return _last_player; }
		size_t no_capture_count() const { return _no_capture_count; }

	public:

		//default constructor, generate a new state.
		State():
			_hidden_pieces
		{
			(uint8_t)(uint8_t)PIECE_RED_PAWN,(uint8_t)PIECE_RED_PAWN,(uint8_t)PIECE_RED_PAWN,(uint8_t)PIECE_RED_PAWN,(uint8_t)PIECE_RED_PAWN,
			(uint8_t)PIECE_RED_CANNON,(uint8_t)PIECE_RED_CANNON,
			(uint8_t)PIECE_RED_KNIGHT,(uint8_t)PIECE_RED_KNIGHT,
			(uint8_t)PIECE_RED_ROOK,(uint8_t)PIECE_RED_ROOK,
			(uint8_t)PIECE_RED_MINISTER,(uint8_t)PIECE_RED_MINISTER,
			(uint8_t)PIECE_RED_GUARD,(uint8_t)PIECE_RED_GUARD,
			(uint8_t)PIECE_RED_KING,
			(uint8_t)PIECE_BLACK_PAWN,(uint8_t)PIECE_BLACK_PAWN,(uint8_t)PIECE_BLACK_PAWN,(uint8_t)PIECE_BLACK_PAWN,(uint8_t)PIECE_BLACK_PAWN,
			(uint8_t)PIECE_BLACK_CANNON,(uint8_t)PIECE_BLACK_CANNON,
			(uint8_t)PIECE_BLACK_KNIGHT,(uint8_t)PIECE_BLACK_KNIGHT,
			(uint8_t)PIECE_BLACK_ROOK,(uint8_t)PIECE_BLACK_ROOK,
			(uint8_t)PIECE_BLACK_MINISTER,(uint8_t)PIECE_BLACK_MINISTER,
			(uint8_t)PIECE_BLACK_GUARD,(uint8_t)PIECE_BLACK_GUARD,
			(uint8_t)PIECE_BLACK_KING
		},
			_last_player(PLAYER_RED),
			_no_capture_count(0)
		{
			_pieces[PIECE_UNKNOWN] = BitBoard(4294967295);
			_debug_data.update(*this);
		}

		//return true if any undecided piece exist.
		inline bool exist_undecided_piece() const
		{
			return _pieces[PIECE_UNDECIDED].any();
		}

		//return true if the player have hidden pieces.
		inline bool exist_hidden_piece(PlayerIndex player) const
		{
			PieceType min = player == PLAYER_RED ? PIECE_RED_PAWN : PIECE_BLACK_PAWN;
			PieceType max = player == PLAYER_RED ? PIECE_RED_KING : PIECE_BLACK_KING;
			for (size_t i = 0; i < _hidden_pieces.length(); i++)
			{
				if (_hidden_pieces[i] >= min && _hidden_pieces[i] <= max)
				{
					return true;
				}
			}
			return false;
		}

		//to next state by an action.
		void to_next(const Action& action);

		//get result of the state.
		Result get_result() const;

		//get all exist piece board
		inline BitBoard get_exist_piece_board() const
		{
			return ~(_pieces[PIECE_EMPTY]);
		}

		//get empty piece board.
		inline BitBoard get_empty_piece_board() const
		{
			return _pieces[PIECE_EMPTY];
		}

		//get state data.
		StateData data() const
		{
#ifdef CDC_DEBUG_INFO
			return _debug_data;
#else
			StateData data;
			data.update(*this);
			return data;
#endif
		}

		//return true if any undecided piece been changed.
		inline bool CheckUndecidedPiece()
		{
			if (_pieces[PIECE_UNDECIDED].any())
			{
				GADT_CHECK_WARNING(g_CDC_DEFINE_CHECK, _hidden_pieces.length() == 0, "do not have any hidden piece.");
				for (size_t i = 0; i < 32; i++)
				{
					if (_pieces[PIECE_UNDECIDED][i] == true)
					{
						_pieces[PIECE_UNDECIDED].reset(i);
						PieceType piece = (PieceType)_hidden_pieces.draw_and_remove_value();
						_pieces[piece].set(i);
					}
				}
				return true;
			}
			return false;
		}
	};

	//print something
	namespace print
	{
		//print piece
		void PrintPiece(PieceType p);

		//print player.
		void PrintPlayer(PlayerIndex p);

		//print action
		void PrintAction(const Action& act);

		//print state
		void PrintState(const State& s);
	}
}
