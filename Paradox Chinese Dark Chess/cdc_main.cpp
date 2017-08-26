#include "cdc_define.h"
#include "cdc_policy.hpp"

using namespace gadt;
using namespace chinese_dark_chess;


void ShellDefine()
{
	std::ios::sync_with_stdio(false);
	using gadt::shell::GameShell;

	GameShell cdc("Chinese Dark Chess");
	cdc.SetDefaultInfoFunc([]()->void {
		console::Cprintf("=============================================\n", console::GRAY);
		console::Cprintf("       Chinese Dark Chess\n", console::YELLOW);
		console::Cprintf("       Copyright @ Junkai-Lu 2017\n", console::YELLOW);
		console::Cprintf("=============================================\n\n", console::GRAY);
	});
	auto* root = cdc.CreateShellPage("root");
	auto* game = cdc.CreateShellPage<State>("game");
	root->AddChildPage("game", "the game");
	root->AddFunction("move", "get move board", []() {
		std::ofstream ofs("move.txt");
		HiddenPiece hidden;
		for (size_t i = PIECE_RED_PAWN; i < PIECE_EMPTY; i++)
		{
			hidden.set(i, 1);
		}
		hidden.set(PIECE_RED_PAWN, 5);
		hidden.set(PIECE_BLACK_PAWN, 5);
		ofs << "hidden  " << hidden.to_ullong() << std::endl;

		HiddenPiece reserve_red;
		for (size_t i = PIECE_RED_PAWN; i <= PIECE_RED_KING; i++)
		{
			reserve_red.set(i, 15);
		}
		ofs << "reserve_red  " << reserve_red.to_ullong() << std::endl;
		HiddenPiece reserve_black;
		for (size_t i = PIECE_BLACK_PAWN; i <= PIECE_BLACK_KING; i++)
		{
			reserve_black.set(i, 15);
		}
		ofs << "reserve_black  " << reserve_black.to_ullong() << std::endl;
	});

	game->AddFunction("show", "show state",[](State& state)->void{print::PrintState(state); });
	game->AddFunction("change", "change piece", [](State& state)->void {});
	game->AddFunction("list", "list all possible moves", [](State& state)->void {
		ActionGenerator action(state);
		action.print();
	});
	game->AddFunction("random", "take random action", [](State& state) {
		ActionGenerator all_action(state);
		auto act = all_action.random_action();
		print::PrintAction(act);
		state.to_next(act);
		print::PrintState(state);
	});
	game->AddFunction("choose", "choose action to next.", [](State& state)->void {
		ActionGenerator action(state);
		action.print();
		std::cout << "input choose :";
		size_t i = 0;
		std::cin >> i;
		std::cout << "pick " << i;
		print::PrintAction(action[i]);
		state.to_next(action[i]);
		print::PrintState(state);
	});

	cdc.StartFromPage("root");
}

int main()
{
	chinese_dark_chess::State s;
	ShellDefine();
	return 0;
}