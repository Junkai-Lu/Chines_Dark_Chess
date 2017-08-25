#include "cdc_define.h"

using namespace gadt;
using std::cout;
using std::endl;

namespace chinese_dark_chess
{
	//global variable.
	size_t g_MOVEABLE_INDEX[32][4] = {
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
		GADT_CHECK_WARNING(g_CDC_DEFINE_CHECK, action.type != FLIPPING_ACTION, "excute flipping act");
		BitBoard clear(0xFFFFFFFF);
		clear.reset(action.source);
		clear.reset(action.dest);

		_pieces[PIECE_EMPTY].reset(action.source);
		_pieces[PIECE_EMPTY].set(action.dest);

		for (size_t i = PIECE_UNKNOWN; i < PIECE_EMPTY; i++)
			_pieces[i] &= clear;

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
		void PrintPiece(PieceType p)
		{
			switch (p)
			{
			case PIECE_UNKNOWN:
				console::Cprintf("●", console::DEFAULT);
				break;
			case PIECE_RED_PAWN:
				console::Cprintf("兵", console::RED);
				break;
			case PIECE_RED_CANNON:
				console::Cprintf("炮", console::RED);
				break;
			case PIECE_RED_KNIGHT:
				console::Cprintf("马", console::RED);
				break;
			case PIECE_RED_ROOK:
				console::Cprintf("车", console::RED);
				break;
			case PIECE_RED_MINISTER:
				console::Cprintf("相", console::RED);
				break;
			case PIECE_RED_GUARD:
				console::Cprintf("士", console::RED);
				break;
			case PIECE_RED_KING:
				console::Cprintf("帅", console::RED);
				break;
			case PIECE_BLACK_PAWN:
				console::Cprintf("卒", console::BLUE);
				break;
			case PIECE_BLACK_CANNON:
				console::Cprintf("炮", console::BLUE);
				break;
			case PIECE_BLACK_KNIGHT:
				console::Cprintf("马", console::BLUE);
				break;
			case PIECE_BLACK_ROOK:
				console::Cprintf("车", console::BLUE);
				break;
			case PIECE_BLACK_MINISTER:
				console::Cprintf("象", console::BLUE);
				break;
			case PIECE_BLACK_GUARD:
				console::Cprintf("士", console::BLUE);
				break;
			case PIECE_BLACK_KING:
				console::Cprintf("将", console::BLUE);
				break;
			case PIECE_UNDECIDED:
				console::Cprintf("??", console::DEFAULT);
				break;
			case PIECE_EMPTY:
				console::Cprintf("  ", console::DEFAULT);
				break;
			default:
				break;
			}
		}

		void PrintPlayer(PlayerIndex p)
		{
			switch (p)
			{
			case PLAYER_RED:
				console::Cprintf("红方", console::RED);
				break;
			case PLAYER_BLACK:
				console::Cprintf("黑方", console::RED);
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
				PrintPiece(act.new_piece);
				cout << "MOVE FROM " << Location(act.source).str() << "To " << Location(act.dest).str();
				break;
			case CAPTURE_ACTION:
				PrintPiece(act.new_piece);
				cout << "CAPTURE FROM " << Location(act.source).str() << "To " << Location(act.dest).str();
				break;
			case FLIPPING_ACTION:
				cout << "FLIPPING PIECE IN " << Location(act.source).str();
				break;
			case FLIPPED_RESULT_ACTION:
				PrintPiece(act.new_piece);
				cout << "BE FIPPED IN " << Location(act.source).str();
				break;
			default:
				break;
			}
		}

		void PrintState(const State & s)
		{
			StateData data = s.data();
			cout << endl << "    ";
			for (size_t x = 0; x < g_CDC_BOARD_WIDTH; x++)
			{
				cout << char(x + 'a') << "   ";
			}
			
			cout << endl << "  ┏";
			for (size_t x = 0; x < g_CDC_BOARD_WIDTH; x++) 
			{
				if (x != g_CDC_BOARD_WIDTH - 1)
					cout << "━┳";
				else
					cout << "━┓" << endl;
			}
			for (size_t y = 0;y < g_CDC_BOARD_HEIGHT; y++)
			{
				cout << y + 1 << " ┃";
				for (size_t x = 0; x < g_CDC_BOARD_WIDTH; x++)
				{
					PrintPiece(data.piece(x, y));
					cout << "┃";
				}
				std::cout << y + 1 << std::endl;

				if (y != g_CDC_BOARD_HEIGHT - 1)
				{
					cout << "  ┣";
					for (size_t x = 0; x < g_CDC_BOARD_WIDTH; x++)
					{
						if (x != g_CDC_BOARD_WIDTH - 1)
						{
							cout << "━╋";
						}
						else
						{
							cout << "━┫" << endl;
						}
					}
				}
				else
				{
					cout << "  ┗";
					for (size_t x = 0; x < g_CDC_BOARD_WIDTH; x++)
					{
						if (x != g_CDC_BOARD_WIDTH - 1)
							cout << "━┻";
						else
							cout << "━┛" << endl;
					}
				}
			}

			cout << "    ";
			for (size_t x = 0; x < g_CDC_BOARD_WIDTH; x++)
			{
				cout << char(x + 'a') << "   ";
			}
			std::cout << std::endl << std::endl;
		}

	}
}