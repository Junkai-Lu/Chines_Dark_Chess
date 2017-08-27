#include "cdc_define.h"

using namespace gadt;
using std::cout;
using std::endl;

namespace chinese_dark_chess
{
	//global variable.
	size_t g_MOVEABLE_INDEX[g_CDC_MAX_LENGTH][4] = {
		{ 63,1,63,8 },
		{ 0,2,63,9 },
		{ 1,3,63,10 },
		{ 2,4,63,11 },
		{ 3,5,63,12 },
		{ 4,6,63,13 },
		{ 5,7,63,14 },
		{ 6,63,63,15 },
		{ 63,9,0,16 },
		{ 8,10,1,17 },
		{ 9,11,2,18 },
		{ 10,12,3,19 },
		{ 11,13,4,20 },
		{ 12,14,5,21 },
		{ 13,15,6,22 },
		{ 14,63,7,23 },
		{ 63,17,8,24 },
		{ 16,18,9,25 },
		{ 17,19,10,26 },
		{ 18,20,11,27 },
		{ 19,21,12,28 },
		{ 20,22,13,29 },
		{ 21,23,14,30 },
		{ 22,63,15,31 },
		{ 63,25,16,63 },
		{ 24,26,17,63 },
		{ 25,27,18,63 },
		{ 26,28,19,63 },
		{ 27,29,20,63 },
		{ 28,30,21,63 },
		{ 29,31,22,63 },
		{ 30,63,23,63 }
	};

	BitBoard g_MOVEABLE_BITBOARD[g_CDC_MAX_LENGTH] =
	{
		BitBoard(258),
		BitBoard(517),
		BitBoard(1034),
		BitBoard(2068),
		BitBoard(4136),
		BitBoard(8272),
		BitBoard(16544),
		BitBoard(32832),
		BitBoard(66049),
		BitBoard(132354),
		BitBoard(264708),
		BitBoard(529416),
		BitBoard(1058832),
		BitBoard(2117664),
		BitBoard(4235328),
		BitBoard(8405120),
		BitBoard(16908544),
		BitBoard(33882624),
		BitBoard(67765248),
		BitBoard(135530496),
		BitBoard(271060992),
		BitBoard(542121984),
		BitBoard(1084243968),
		BitBoard(2151710720),
		BitBoard(33619968),
		BitBoard(84017152),
		BitBoard(168034304),
		BitBoard(336068608),
		BitBoard(672137216),
		BitBoard(1344274432),
		BitBoard(2688548864),
		BitBoard(1082130432),
	};

	//update state data
	void StateData::update(const State & state)
	{
		for (size_t x = 0; x < g_CDC_BOARD_WIDTH; x++)
		{
			for (size_t y = 0; y < g_CDC_BOARD_HEIGHT; y++)
			{
				_data[x][y] = PIECE_EMPTY;
			}
		}
		for (uint8_t piece_id = PIECE_UNKNOWN; piece_id < PIECE_EMPTY; piece_id++)
		{
			const BitBoard& board = state.piece_board(static_cast<PieceType>(piece_id));
			for (size_t index = 0; index < g_CDC_MAX_LENGTH; index++)
			{
				if (board.get(index))
				{
					_data[index % g_CDC_BOARD_WIDTH][index / g_CDC_BOARD_WIDTH] = static_cast<PieceType>(piece_id);
				}
			}
		}
	}

	//to next state.
	void State::to_next(const Action & action)
	{	
		BitBoard clear(0xFFFFFFFFFFFFFFFF);
		clear.reset(action.source);
		clear.reset(action.dest);
		for (size_t i = PIECE_UNKNOWN; i < PIECE_EMPTY; i++)
			_pieces[i] &= clear;

		_pieces[PIECE_EMPTY].set(action.source);
		_pieces[PIECE_EMPTY].reset(action.dest);
		_pieces[action.piece].set(action.dest);

		if (action.type == FLIPPED_RESULT_ACTION)
		{
			_hidden_pieces.decrease(action.piece);
		}
		else
		{
			exchange_player();
		}
#ifdef CDC_DEBUG_INFO
		_debug_data.to_next(action);
#endif
	}

	//get result.
	Result State::get_result() const
	{
		if (_no_capture_count > 20)
		{
			return RESULT_DRAW;
		}
		bool no_red =
			!exist_hidden_piece(PLAYER_RED) &&
			_pieces[PIECE_RED_PAWN].none() &&
			_pieces[PIECE_RED_CANNON].none() &&
			_pieces[PIECE_RED_KNIGHT].none() &&
			_pieces[PIECE_RED_ROOK].none() &&
			_pieces[PIECE_RED_MINISTER].none() &&
			_pieces[PIECE_RED_GUARD].none() &&
			_pieces[PIECE_RED_KING].none();
		if (no_red) { return RESUKT_BLACK_WIN; }

		bool no_black =
			!exist_hidden_piece(PLAYER_BLACK) &&
			_pieces[PIECE_BLACK_PAWN].none() &&
			_pieces[PIECE_BLACK_CANNON].none() &&
			_pieces[PIECE_BLACK_KNIGHT].none() &&
			_pieces[PIECE_BLACK_ROOK].none() &&
			_pieces[PIECE_BLACK_MINISTER].none() &&
			_pieces[PIECE_BLACK_GUARD].none() &&
			_pieces[PIECE_BLACK_KING].none();
		if (no_black) { return RESULT_RED_WIN; }
		return RESULT_UNFINISH;
	}

	//print board.
	namespace print
	{
		std::string PieceToStr(PieceType piece)
		{
			switch (piece)
			{
			case PIECE_UNKNOWN:
				return "●";
			case PIECE_RED_PAWN:
				return "兵";
			case PIECE_RED_CANNON:
				return "炮";
			case PIECE_RED_KNIGHT:
				return "马";
			case PIECE_RED_ROOK:
				return "车";
			case PIECE_RED_MINISTER:
				return "相";
			case PIECE_RED_GUARD:
				return "士";
			case PIECE_RED_KING:
				return "帅";
			case PIECE_BLACK_PAWN:
				return "卒";
			case PIECE_BLACK_CANNON:
				return "炮";
			case PIECE_BLACK_KNIGHT:
				return "马";
			case PIECE_BLACK_ROOK:
				return "车";
			case PIECE_BLACK_MINISTER:
				return "象";
			case PIECE_BLACK_GUARD:
				return "士";
			case PIECE_BLACK_KING:
				return "将";
			case PIECE_UNDECIDED:
				return "??";
			case PIECE_EMPTY:
				return "  ";
			default:
				return "  ";
			}
		}

		gadt::console::ConsoleColor PieceToColor(PieceType piece)
		{
			if (piece >= PIECE_RED_PAWN && piece <= PIECE_RED_KING)
			{
				return gadt::console::RED;
			}
			if (piece >= PIECE_BLACK_PAWN && piece <= PIECE_BLACK_KING)
			{
				return gadt::console::BLUE;
			}
			return gadt::console::DEFAULT;
		}

		std::string ActionTypeToStr(ActionType type)
		{
			switch (type)
			{
			case chinese_dark_chess::MOVE_ACTION:
				return "MOVE";
			case chinese_dark_chess::CAPTURE_ACTION:
				return "CAPTURE";
			case chinese_dark_chess::FLIPPING_ACTION:
				return "FLIPPING";
			case chinese_dark_chess::FLIPPED_RESULT_ACTION:
				return "FLIPPED";
			default:
				return "";
			}
			return "";
		}

		void PrintPiece(PieceType p)
		{
			console::Cprintf(PieceToStr(p), PieceToColor(p));
		}

		void PrintPlayer(PlayerIndex p)
		{
			switch (p)
			{
			case PLAYER_RED:
				console::Cprintf("红方", console::RED);
				break;
			case PLAYER_BLACK:
				console::Cprintf("黑方", console::BLUE);
				break;
			default:
				break;
			}
		}

		void PrintAction(const Action & act)
		{
			switch (act.type)
			{
			case MOVE_ACTION:
				PrintPiece(act.piece);
				cout << "MOVE FROM " << Location(act.source).str() << "To " << Location(act.dest).str();
				break;
			case CAPTURE_ACTION:
				PrintPiece(act.piece);
				cout << "CAPTURE FROM " << Location(act.source).str() << "To " << Location(act.dest).str();
				break;
			case FLIPPING_ACTION:
				cout << "FLIPPING PIECE IN " << Location(act.source).str();
				break;
			case FLIPPED_RESULT_ACTION:
				PrintPiece(act.piece);
				cout << "BE FIPPED IN " << Location(act.source).str();
				break;
			default:
				break;
			}
			std::cout << std::endl;
		}

		void PrintState(const State & s)
		{
			StateData data = s.data();
			gadt::table::ConsoleTable table(8, 4);
			for (size_t y = 0; y < g_CDC_BOARD_HEIGHT; y++)
			{
				for (size_t x = 0; x < g_CDC_BOARD_WIDTH; x++)
				{
					table.cell(x, y).str = PieceToStr(data.piece(x,y));
					table.cell(x, y).color = PieceToColor(data.piece(x, y));
				}
			}
			table.print();
			std::cout << "next player :";
			PrintPlayer(s.next_player());
			std::cout << std::endl;
		}

	}
}