#include "cdc_define.h"

void ShellDefine()
{
	std::ios::sync_with_stdio(false);
	using gadt::shell::GameShell;
	using gadt::shell::ShellPage;

	GameShell cdc("Chinese Dark Chess");
	auto* root = gadt::shell::CreateShellPage<int>(cdc, "root");
	cdc.RunPage("root");
}

int main()
{
	chinese_dark_chess::State s;
	ShellDefine();
	return 0;
}