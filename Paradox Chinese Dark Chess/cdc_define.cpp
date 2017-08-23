#include "cdc_define.h"

using namespace gadt;
using std::cout;
using std::endl;

namespace chinese_dark_chess
{
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
		for (uint8_t piece_id = 0; piece_id < 15; piece_id++)
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

	BitBoard g_MOVE_BOARD[32] = {
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
		BitBoard(1082130432)
	};


	ActionData Action::data(const State & s) const
	{
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
	}

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

		void PrintAction(const Action & act, const State& s)
		{
			ActionData data = act.data(s);
			switch (data.type)
			{
			case MOVE_ACTION:
				PrintPiece(data.new_piece);
				cout << "MOVE FROM " << data.source_loc.str() << "To " << data.dest_loc.str();
				break;
			case CAPTURE_ACTION:
				PrintPiece(data.new_piece);
				cout << "CAPTURE FROM " << data.source_loc.str() << "To " << data.dest_loc.str();
				break;
			case FLIPPING_ACTION:
				cout << "FLIPPING PIECE IN " << data.source_loc.str();
				break;
			case FLIPPED_RESULT_ACTION:
				PrintPiece(data.new_piece);
				cout << "BE FIPPED IN " << data.source_loc.str();
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


