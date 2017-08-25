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
		ofs << "{" << std::endl;
		for (size_t i = 0; i < 32; i++)
		{
			ofs << "{";
			//chinese_dark_chess::BitBoard temp;
			if (i % 8 != 0)
			{
				ofs << i - 1 << ",";
			}
			else { ofs << 63 << ","; }
			if (i % 8 != 7)
			{
				ofs << i + 1 << ",";
			}
			else { ofs << 63 << ","; }
			if (i > 7)
			{
				ofs << i - 8 << ",";
			}
			else { ofs << 63 << ","; }
			if (i < 24)
			{
				ofs << i + 8 << "}," << std::endl;
			}
			else { ofs << 63 << "}," << std::endl; }
		}
	});

	game->AddFunction("show", "show state",[](State& state)->void{print::PrintState(state); });
	game->AddFunction("change", "change piece", [](State& state)->void {});
	game->AddFunction("all", "show all possible moves", [](State& state)->void {
		ActionGenerator action(state);
		action.print();
	});
	game->AddFunction("random", "take random action", [](State& state) {
		ActionGenerator all_action(state);
		auto act = all_action.random_action();
		print::PrintAction(act);
		state.to_next(act);
	});

	cdc.StartFromPage("root");
}

int main()
{
	chinese_dark_chess::State s;
	ShellDefine();
	return 0;
}