#include "cdc_define.h"

using namespace gadt;
using chinese_dark_chess::State;

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
	auto* game = cdc.CreateShellPage<chinese_dark_chess::State>("game");
	root->AddChildPage("game", "the game");
	root->AddFunction("move", "get move board", []() {
		std::ofstream ofs("move.txt");
		for (size_t i = 0; i < 32; i++)
		{
			chinese_dark_chess::BitBoard temp;
			if (i % 8 != 0)
			{
				temp.set(i - 1);
			}
			if (i % 8 != 7)
			{
				temp.set(i + 1);
			}
			if (i > 7)
			{
				temp.set(i - 8);
			}
			if (i < 24)
			{
				temp.set(i + 8);
			}
			ofs << "BitBoard(" << temp.to_ullong() << ")," << std::endl;
		}
	});

	game->AddFunction("show", "show state",[](State& state)->void{chinese_dark_chess::print::PrintState(state); });
	game->AddFunction("change", "change piece", [](State& state)->void {
	});

	cdc.StartFromPage("root");
}

int main()
{
	chinese_dark_chess::State s;
	ShellDefine();
	return 0;
}