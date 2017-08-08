/* Copyright (c) 2017 Junkai Lu <junkai-lu@outlook.com>.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include "gadtlib.h"

#pragma once

namespace gadt
{
	namespace shell
	{
		//Shell Page Interface
		class GameShell;
		class ShellPageBase
		{
			friend class GameShell;
		private:
			GameShell* _belonging_shell;	//the game shell this page belong to.
			std::string _name;				//name, each name correspond to one page in a game shell.
			size_t _index;					//page index, each page have a unique index.
			ShellPageBase* _call_source;	//call source point to the page that call this page.

		protected:
			std::function<void()> _info_func;			//info function. would be called before show menu.
			std::function<void()> _constructor_func;	//constructor func. would be called when enter the page.
			std::function<void()> _destructor_func;		//destructror func. would be called when quit the page.

			//static paramaters.
			static const char*  g_SHELL_HELP_COMMAND_STR;			//help command, default is 'help'
			static const char*  g_SHELL_EXIT_COMMAND_STR;			//exit page command, default is 'return'
			static const char*  g_SHELL_CLEAN_COMMAND_STR;			//clean screen command, default is 'clear'
			static const size_t g_SHELL_MAX_COMMAND_LENGTH;		//max length of the command. 

		protected:

			inline void set_info_func(std::function<void()> info_func)
			{
				_info_func = info_func;
			}

			inline void set_constructor_func(std::function<void()> constructor_func)
			{
				_constructor_func = constructor_func;
			}

			inline void set_destructor_func(std::function<void()> destructor_func)
			{
				_destructor_func = destructor_func;
			}

			inline void set_call_source(ShellPageBase* call_source)
			{
				_call_source = call_source;
			}

			//get belonging shell.
			inline GameShell* belonging_shell() const
			{
				return _belonging_shell;
			}

			//get call source.
			inline ShellPageBase* call_source() const
			{
				return _call_source;
			}

			//get page name.
			inline std::string name() const
			{
				return _name;
			}

			//get page index.
			inline size_t index() const
			{
				return _index;
			}

			//print path.
			void ShowPath() const;

			//To be focus.
			void BeFocus();

			//allocate page index.
			static inline size_t AllocPageIndex()
			{
				static size_t page_index = 0;
				return page_index++;
			}

			//create a new page.
			ShellPageBase(GameShell* belonging_shell, std::string name);

			//copy constructor function is disallowed.
			ShellPageBase(ShellPageBase& sb) = delete;

			//a virtural function.
			virtual void Run(ShellPageBase* call_source) = 0;

		public:
			void CleanScreen() const;
			virtual ~ShellPageBase() = default;

		};

		//Shell
		class GameShell final
		{
			friend ShellPageBase;

		private:
			//global variable
			static GameShell* _g_focus_game;	//focus page, that is used for show path.

												//page table
			std::map<std::string, std::shared_ptr<ShellPageBase>> _page_table;
			std::string _name;
			ShellPageBase* _focus_page;

		public:
			//global function
			static inline GameShell* focus_game()
			{
				return _g_focus_game;
			}

			//print input tips.
			static inline void InputTip(std::string tip = "")
			{
				if (focus_game() != nullptr)
				{
					if (focus_game()->focus_page() != nullptr)
					{
						focus_game()->focus_page()->ShowPath();
						if (tip != "")
						{
							std::cout << "/";
						}
						console::Cprintf(tip, console::GREEN);
						std::cout << ": >> ";
					}
					else
					{
						console::Cprintf("ERROR: focus page not exist", console::PURPLE);
					}
				}
				else
				{
					console::Cprintf("ERROR: focus game not exist", console::PURPLE);
				}
			}

			//get input line by string format.
			static inline std::string GetInput()
			{
				char buffer[50];
				std::cin.getline(buffer, 50);
				return std::string(buffer);
			}

			//public function
			GameShell(std::string name);

			//copy constructor is disallowed.
			GameShell(GameShell&) = delete;

			//get name of shell.
			inline std::string name() const
			{
				return _name;
			}

			//get focus_page of this shell.
			inline ShellPageBase* focus_page() const
			{
				return _focus_page;
			}

			//return true if the page name exist.
			inline bool page_exist(std::string name) const
			{
				return _page_table.count(name) > 0;
			}

			//add new page to this shell.
			inline void AddPage(std::string name, std::shared_ptr<ShellPageBase> new_page)
			{
				_page_table[name] = new_page;
			}

			//to be the focus.
			inline void BeFocus()
			{
				_g_focus_game = this;
			}

			//run page.
			inline void RunPage(std::string name, ShellPageBase* call_source = nullptr)
			{
				if (page_exist(name))
				{
					_page_table[name]->Run(call_source);
				}
				else
				{
					std::cerr << "page '" << name << "' not exist" << std::endl;
				}
			}


		};

		//Shell Page
		template<typename datatype>
		class ShellPage final :public ShellPageBase
		{
			friend class ShellCreator;
		private:
			std::map<std::string, std::function<void(datatype&)> > _func;			//dict of commands.
			std::map<std::string, std::string> _des;								//dict of describes.
			std::set<std::string> _child;											//dict of child pages
			std::function<bool(std::string, datatype&)> _custom_command_check_func;	//custom command check func
			datatype _data;															//data of the page.

		private:
			inline void ShellInit()
			{
				//add exit describe
				AddDescript(g_SHELL_EXIT_COMMAND_STR, "return to previous menu.");

				//add help command.
				AddFunction(g_SHELL_HELP_COMMAND_STR, [&](datatype& data)->void {
					std::cout << std::endl << ">> ";
					console::Cprintf("[ COMMAND LIST ]\n\n", console::YELLOW);
					for (auto command : _des)
					{
						std::cout << "   '";
						console::Cprintf(command.first, console::RED);
						std::cout << "'" << std::string(g_SHELL_MAX_COMMAND_LENGTH,' ').substr(0, g_SHELL_MAX_COMMAND_LENGTH - command.first.length()) 
							<< command.second << std::endl;
					}
					std::cout << std::endl << std::endl;
				}, "get command list");

				//add clean command.
				AddFunction(g_SHELL_CLEAN_COMMAND_STR, [&](datatype& data)->void {
					this->CleanScreen();
				}, "clean screen.");
			}

			//data operator
			inline datatype& data()
			{
				return _data;
			}

			//return true if the func name exist
			inline bool func_exist(std::string command) const
			{
				return _func.count(command) > 0;
			}

			//return true if the child page exist.
			inline bool child_exist(std::string command) const
			{
				return _child.count(command) > 0;
			}

			//return true if the describe exist.
			inline bool des_exist(std::string command) const
			{
				return _des.count(command) > 0;
			}

			//get func by name.
			inline std::function<void(datatype&)> get_func(std::string command)
			{
				if (!func_exist(command))
				{
					std::cerr << "function '" << command << "' in page '" << name() << "'not exist" << std::endl;
					console::SystemPause();
				}
				return _func[command];
			}

			//get describe by name.
			inline std::string get_des(std::string command) const
			{
				if (!func_exist(command))
				{
					std::cerr << "descript '" << command << "' in page '" << name() << "'not exist" << std::endl;
					console::SystemPause();
				}
				return _des.at(command);
			}

			//run page.
			void Run(ShellPageBase* call_source) override
			{
				set_call_source(call_source);
				BeFocus();
				CleanScreen();

				//excute constructor func.
				_constructor_func();

				for (;;)
				{
					/*
					* command check order
					* 1.return
					* 2.function
					* 3.child shell
					* 4.extra command
					*/
					BeFocus();	//ensure this page is always the focus.
					GameShell::InputTip();
					std::string command;

					//get command that is not empty.
					for (;;)
					{
						command = GameShell::GetInput();
						if (command != "")
						{
							break;
						}
					}

					//return command
					if (command == g_SHELL_EXIT_COMMAND_STR)
					{
						if (call_source != nullptr)
						{
							call_source->CleanScreen();
						}
						break;
					}

					//function command check
					if (func_exist(command))
					{
						get_func(command)(_data);
						continue;
					}

					//child shell check
					if (child_exist(command))
					{
						belonging_shell()->RunPage(command, this);
						continue;
					}

					//extra command check
					if (_custom_command_check_func(command, _data))
					{
						continue;
					}

					//error
					console::Cprintf("ERROR: ", console::PURPLE);
					std::cout << "command not found." << std::endl;
				}

				_destructor_func();	//excute destructor func.
			}

			//curtail command if the length of command is out of max length.
			inline std::string curtail_command(std::string command)
			{
				if (command.size() > g_SHELL_MAX_COMMAND_LENGTH)
				{
					return command.substr(0, g_SHELL_MAX_COMMAND_LENGTH);
				}
				return command;
			}

		public:
			//default function.
			ShellPage(GameShell* belonging_shell, std::string name) :
				ShellPageBase(belonging_shell, name),
				_custom_command_check_func([](std::string a, datatype& b)->bool {return false; })
			{
				ShellInit();
			}

			//create a new shell page.
			ShellPage(GameShell* belonging_shell, std::string name, datatype data) :
				ShellPageBase(belonging_shell, name),
				_custom_command_check_func([](std::string a, datatype& b)->bool {return false; }),
				_data(data)
			{
				ShellInit();
			}

			//copy constructor is disallowed.
			ShellPage(ShellPage&) = delete;

			//deconstor function, DO NOT EXECUTE ALONE.
			~ShellPage()
			{

			}

			//add child page of this page,the name page should exist in same shell and the name is also the command to enter this page.
			inline void AddChildPage(std::string child_name, std::string des)
			{
				child_name = curtail_command(child_name);
				if (child_name != name())
				{
					_des[child_name] = des;
					_child.insert(child_name);
				}
				else
				{
					console::Cprintf("INIT ERROR: ", console::PURPLE);
					console::Cprintf(" page ", console::GRAY);
					console::Cprintf(name(), console::RED);
					console::Cprintf(" add itself as child page!\n", console::GRAY);
					console::SystemPause();
				}
			}

			//add a function that can be execute by command and is allowed to visit the data binded in this page.
			inline void AddFunction(std::string command, std::function<void(datatype&)> func, std::string des)
			{
				command = curtail_command(command);
				_func[command] = func;
				_des[command] = des;
			}

			//add/cover descript, the command should be added in other place.
			inline void AddDescript(std::string command, std::string des)
			{
				command = curtail_command(command);
				_des[command] = des;
			}

			//use a extend function to check and execute command, if the command is legal that the function should return 'true'.
			inline void AddCustomCommandCheck(std::function<bool(std::string, datatype&)> func, std::string command, std::string des)
			{
				AddDescript(command, des);
				_custom_command_check_func = func;
			}

			//add info function about this page, the func would be execute before the page works. 
			inline void AddInfoFunc(std::function<void()> info_func)
			{
				set_info_func(info_func);
			}

			//add constructor func, it would be called when enter page. 
			inline void AddConstructorFunc(std::function<void()> constructor_func)
			{
				set_constructor_func(constructor_func);
			}

			//add destructor func, it would be called when quit page.
			inline void AddDestructorFunc(std::function<void()> destructor_func)
			{
				set_destructor_func(destructor_func);
			}
		};

		//Create Page
		template<typename datatype>
		ShellPage<datatype>* CreateShellPage(GameShell& belonging_shell, std::string name)
		{
			std::shared_ptr<ShellPage<datatype>> ptr(new ShellPage<datatype>(&belonging_shell, name));
			belonging_shell.AddPage(name, ptr);
			return ptr.get();
		}

		template<typename datatype>
		ShellPage<datatype>* CreateShellPage(GameShell& belonging_shell, std::string name, datatype data)
		{
			std::shared_ptr<ShellPage<datatype>> ptr(new ShellPage<datatype>(&belonging_shell, name, data));
			belonging_shell.AddPage(name, ptr);
			return ptr.get();
		}
	}
}
