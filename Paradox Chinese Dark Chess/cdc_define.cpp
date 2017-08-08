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

	namespace print
	{
		void piece(PieceType p)
		{
			switch (p)
			{
			case PIECE_UNKNOWN:
				console::Cprintf("��", console::DEFAULT);
				break;
			case PIECE_RED_PAWN:
				console::Cprintf("��", console::RED);
				break;
			case PIECE_RED_CANNON:
				console::Cprintf("��", console::RED);
				break;
			case PIECE_RED_KNIGHT:
				console::Cprintf("��", console::RED);
				break;
			case PIECE_RED_ROOK:
				console::Cprintf("��", console::RED);
				break;
			case PIECE_RED_MINISTER:
				console::Cprintf("��", console::RED);
				break;
			case PIECE_RED_GUARD:
				console::Cprintf("ʿ", console::RED);
				break;
			case PIECE_RED_KING:
				console::Cprintf("˧", console::RED);
				break;
			case PIECE_BLACK_PAWN:
				console::Cprintf("��", console::BLUE);
				break;
			case PIECE_BLACK_CANNON:
				console::Cprintf("��", console::BLUE);
				break;
			case PIECE_BLACK_KNIGHT:
				console::Cprintf("��", console::BLUE);
				break;
			case PIECE_BLACK_ROOK:
				console::Cprintf("��", console::BLUE);
				break;
			case PIECE_BLACK_MINISTER:
				console::Cprintf("��", console::BLUE);
				break;
			case PIECE_BLACK_GUARD:
				console::Cprintf("ʿ", console::BLUE);
				break;
			case PIECE_BLACK_KING:
				console::Cprintf("��", console::BLUE);
				break;
			case PIECE_EMPTY:
				console::Cprintf("  ", console::DEFAULT);
				break;
			default:
				break;
			}
		}

		void player(PlayerIndex p)
		{
			switch (p)
			{
			case PLAYER_RED:
				console::Cprintf("�췽", console::RED);
				break;
			case PLAYER_BLACK:
				console::Cprintf("�ڷ�", console::RED);
				break;
			default:
				break;
			}
		}

		void action(const Action & act, const State& s)
		{
			
		}

		void state(const State & s)
		{
		}


	}
}


