#include "../include/gadtlib.h"
#include "../include/bitboard.hpp"
#include "../include/gameshell.h"
#include "../include/mcts_new.hpp"
#include "../include/minimax.hpp"
#include "../include/visual_tree.h"

#pragma once

#ifdef GADT_DEBUG_INFO
	#define CDC_DEBUG_INFO
#endif

namespace chinese_dark_chess
{
	constexpr const bool g_CDC_DEFINE_CHECK = true;
	constexpr const size_t g_CDC_MAX_ACTION_COUNT = 128;
	constexpr const size_t g_CDC_BITBOARD_SIZE = 17;
	constexpr const size_t g_CDC_BOARD_WIDTH = 8;
	constexpr const size_t g_CDC_BOARD_HEIGHT = 4;
	constexpr const size_t g_CDC_MAX_LENGTH = g_CDC_BOARD_WIDTH * g_CDC_BOARD_HEIGHT;

	//bitboard of the pieces.
	using BitBoard = gadt::bitboard::BitBoard64;
	using HiddenPiece = gadt::bitboard::BitPoker;

	extern size_t g_MOVEABLE_INDEX[g_CDC_MAX_LENGTH][4];
	extern BitBoard g_MOVEABLE_BITBOARD[g_CDC_MAX_LENGTH];

	//index of players.
	enum PlayerIndex : int8_t
	{
		PLAYER_RED = 1,
		PLAYER_BLACK = -1
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
	enum Result:int8_t
	{
		RESULT_RED_WIN = 1,
		RESULT_DRAW = 0,
		RESUKT_BLACK_WIN = -1,
		RESULT_UNFINISH = 2
	};

	
	
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
			GADT_CHECK_WARNING(g_CDC_DEFINE_CHECK, index >= g_CDC_MAX_LENGTH, "out of index");
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
		const PieceType		piece;

		Action(ActionType _type, size_t _source, size_t _dest, PieceType _piece):
			type(_type),
			source(_source),
			dest(_dest),
			piece(_piece)
		{
		}
	};

	//data of state.
	class StateData
	{
	private:
		PieceType _data[g_CDC_BOARD_WIDTH][g_CDC_BOARD_HEIGHT];
		std::vector<PieceType> _hidden_pieces;
		PlayerIndex _next_player;

	public:

		StateData();

		StateData(const State& state);

		StateData(std::vector<std::vector<PieceType>> data, std::vector<PieceType> hidden_pieces, PlayerIndex next_player);

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

		//updata data by State.
		void update(const State& state);

		//translate to state.
		State to_state() const;
	};

	//action set
	class ActionSet
	{
	private:
		using ActionPool = gadt::random::RandomPool<Action>;
		
		ActionPool _action_pool;
		bool _all_weight_equal;
		
	public:
		ActionSet(bool all_weight_equal = true):
			_action_pool(g_CDC_MAX_ACTION_COUNT)
		{
		}

		//get the size of the ActionSet
		size_t size() const
		{
			return _action_pool.size();
		}

		//push new action
		void push(Action action, size_t weight = 1)
		{
			_action_pool.add(weight, action);
		}

		//get random action, return Action() if not action exist.
		const Action& random_action() const
		{
			if (_all_weight_equal)
			{
				size_t rnd = rand() % size();
				return _action_pool.get_element(rnd);
			}
			
			return _action_pool.random();
		}

		//get ref of action by index.
		const Action& action(size_t index) const
		{
			return _action_pool.get_element(index);
		}
	};

	//basic data struct of game state.
	class State
	{
	private:
		BitBoard	_pieces[g_CDC_BITBOARD_SIZE];	//pieces.
		HiddenPiece	_hidden_pieces;					//pieces that still no be flipped.
		PlayerIndex _next_player;					//index of last moved player.
		size_t		_no_capture_count;				//the count of no capture, draw if the value more than 20.

#ifdef CDC_DEBUG_INFO
		StateData	_debug_data;					//debug data
#endif

	public:
		template <typename T>const BitBoard& piece_board(T id) const { return _pieces[id]; }
		const HiddenPiece& hidden_pieces() const { return _hidden_pieces; }
		PlayerIndex next_player() const { return _next_player; }
		size_t no_capture_count() const { return _no_capture_count; }

	public:

		//default constructor, generate a new state.
		State():
			_hidden_pieces(1306644573751223552),
			_next_player(PLAYER_RED),
			_no_capture_count(0)
		{
			_pieces[PIECE_UNKNOWN] = BitBoard(4294967295);
			_debug_data.update(*this);
		}

		State(const std::vector<std::vector<PieceType>>& data, HiddenPiece hidden, PlayerIndex next_player):
			_hidden_pieces(hidden),
			_next_player(next_player)
		{
			for (size_t y = 0; y < g_CDC_BOARD_HEIGHT; y++)
			{
				for (size_t x = 0; x < g_CDC_BOARD_WIDTH; x++)
				{
					_pieces[data[x][y]].set((y* g_CDC_BOARD_WIDTH) + x);
				}
			}
		}

		//return true if any undecided piece exist.
		inline bool exist_undecided_piece() const
		{
			return _pieces[PIECE_UNDECIDED].any();
		}

		//return true if the player have hidden pieces.
		inline bool exist_hidden_piece(PlayerIndex player) const
		{
			HiddenPiece reserve = (player == PLAYER_RED ? HiddenPiece(68719476480) : HiddenPiece(18446744004990074880));
			reserve &= _hidden_pieces;
			return reserve.any();
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

		//exchange player
		inline void exchange_player()
		{
			_next_player = PlayerIndex(-1 * _next_player);
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
	};

	//print something
	namespace print
	{
		std::string PieceToStr(PieceType piece);

		gadt::console::ConsoleColor PieceToColor(PieceType piece);
		
		std::string ActionTypeToStr(ActionType type);

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
