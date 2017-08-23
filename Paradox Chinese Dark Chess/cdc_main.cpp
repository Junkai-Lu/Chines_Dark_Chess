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