/*
* General monte carlo tree search algorithm lib.
*
* a search framework for perfect information sequential games, which allow
* user to insert extra function or redefine default functions to custom the
* search algorithm.
*/

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
#include "visual_tree.h"
#include "gadtstl.hpp"

#pragma once

namespace gadt
{
	namespace mcts_new
	{
		//allow check warning if it is true.
		constexpr const bool g_MCTS_NEW_ENABLE_WARNING = true;

		//the method for parallel mcts
		enum MctsParalleMethod: uint8_t
		{
			NO_PARALLELIZATION = 0,
			LEAF_PARALLELIZATION = 1,
			ROOT_PARALLELIZATION = 2,
			TREE_PARALLELIZATION = 3
		};

		//AgentIndex is the type index of each player, default is int8_t.
		using AgentIndex		= int8_t;
		using UcbValue			= double;

		namespace policy
		{
			inline UcbValue UCB1(UcbValue average_reward, UcbValue overall_time, UcbValue played_time, UcbValue c = 1)
			{
				return average_reward + c * static_cast<UcbValue>(sqrt(2 * log10(overall_time) / played_time));
			}
		}

		/*
		* MctsNode is the node class in the monte carlo tree search.
		*
		* [State] is the game-state class, which is defined by the user.
		* [Action] is the game-action class, which is defined by the user.
		* [Result] is the game-result class, which stand for a terminal state of the game.
		* [is_debug] means some debug info would not be ignored if it is true. this may result in a little degradation of performance.
		*/
		template<typename State, typename Action, typename Result, bool _is_debug>
		class MctsNode
		{
		public:											
			using pointer       = MctsNode<State, Action, Result, _is_debug>*;
			using reference     = MctsNode<State, Action, Result, _is_debug>&;
			using Node			= MctsNode<State, Action, Result, _is_debug>;		//MctsNode
			using Allocator		= gadt::stl::Allocator<Node, _is_debug>;			//Allocate 
			using ActionSet		= std::vector<Action>;								//ActionSet is the set of Action.
			using NodePtrSet	= std::vector<pointer>;							//ChildSet is the set of ptrs to child nodes.
			


			//function package
			struct FuncPackage
			{
			public:
				using GetNewStateFunc		= std::function<State(const State&, const Action&)>;
				using MakeActionFunc		= std::function<void(const State&, ActionSet&)>;	
				using DetemineWinnerFunc	= std::function<AgentIndex(const State&)>;			
				using StateToResultFunc		= std::function<Result(const State&, AgentIndex)>;	
				using AllowUpdateValueFunc	= std::function<bool(const State&, const Result&)>;	
				using TreePolicyValueFunc	= std::function<UcbValue(const Node&, const Node&)>;
				using DefaultPolicyFunc		= std::function<const Action&(const ActionSet&)>;	
				using AllowExtendFunc		= std::function<bool(const Node&)>;					
				using AllowExcuteGcFunc		= std::function<bool(const Node&)>;					
				using ValueForRootNodeFunc	= std::function<UcbValue(const Node&)>;				

			public:
				//necessary functions.
				const GetNewStateFunc		GetNewState;		//get a new state from previous state and action.
				const MakeActionFunc		MakeAction;			//the function which create action set by the state.
				const DetemineWinnerFunc	DetemineWinner;		//return no_winner_index if a state is not terminal state.
				const StateToResultFunc		StateToResult;		//get a result from state and winner.
				const AllowUpdateValueFunc	AllowUpdateValue;	//update values in the node by the result.

				//default functions.
				TreePolicyValueFunc			TreePolicyValue;	//value of child node in selection process. the highest would be seleced.
				DefaultPolicyFunc			DefaultPolicy;		//the default policy to select action.
				AllowExtendFunc				AllowExtend;		//allow node to extend child node.
				AllowExcuteGcFunc			AllowExcuteGc;		//the condition to excute gc in a node.
				ValueForRootNodeFunc		ValueForRootNode;	//select best action of root node after iterations finished.

			public:
				FuncPackage(
					GetNewStateFunc			_GetNewState,
					MakeActionFunc			_MakeAction,
					DetemineWinnerFunc		_DetemineWinner,
					StateToResultFunc		_StateToResult,
					AllowUpdateValueFunc	_AllowUpdateValue,
					TreePolicyValueFunc		_TreePolicyValue,
					DefaultPolicyFunc		_DefaultPolicy,
					AllowExtendFunc			_AllowExtend,
					AllowExcuteGcFunc		_AllowExcuteGc,
					ValueForRootNodeFunc	_ValueForRootNode
				):
					GetNewState			(_GetNewState),
					MakeAction			(_MakeAction),
					DetemineWinner		(_DetemineWinner),
					StateToResult		(_StateToResult),
					AllowUpdateValue	(_AllowUpdateValue),
					TreePolicyValue		(_TreePolicyValue),
					DefaultPolicy		(_DefaultPolicy),
					AllowExtend			(_AllowExtend),
					AllowExcuteGc		(_AllowExcuteGc),
					ValueForRootNode	(_ValueForRootNode)
				{
				}

				FuncPackage(
					GetNewStateFunc			_GetNewState,
					MakeActionFunc			_MakeAction,
					DetemineWinnerFunc		_DetemineWinner,
					StateToResultFunc		_StateToResult,
					AllowUpdateValueFunc	_AllowUpdateValue
				):
					GetNewState			(_GetNewState),
					MakeAction			(_MakeAction),
					DetemineWinner		(_DetemineWinner),
					StateToResult		(_StateToResult),
					AllowUpdateValue	(_AllowUpdateValue)
				{
				}
			};

		private:
			State			_state;				//state of this node.
			AgentIndex		_winner_index;		//the winner index of the state.
			uint32_t		_visited_time;		//how many times that this node had been visited.
			uint32_t		_win_time;			//win time accmulated by the simulation.
			uint8_t			_next_action_index;	//the index of next action.
			ActionSet		_action_set;		//action set of this node.
			NodePtrSet		_child_nodes;		//the ptr of child nodes.

		public:
			const State&		state()					const { return _state; }
			const AgentIndex	winner_index()			const { return _winner_index; }
			const uint32_t		visited_time()			const { return _visited_time; }
			const uint32_t		win_time()				const { return _win_time; }
			const uint8_t		next_action_index()		const { return _next_action_index; }
			const size_t		child_num()				const { return _child_nodes.size(); }
			const NodePtrSet&	child_set()				const { return _child_nodes; }
			const MctsNode*		child_node(size_t i)	const { return _child_nodes[i]; }
			const size_t		action_num()			const { return _action_set.size(); }
			const ActionSet&	action_set()			const { return _action_set; }
			const Action&		action(size_t i)		const { return _action_set[i]; }

		private:
			//a value means no winner, which is differ from any other AgentIndex.
			static const AgentIndex _no_winner_index = 0;
			static const size_t		_default_policy_warning_length = 1000;

			//exist unactived action in the action set.
			inline bool exist_unactivated_action() const
			{
				return _next_action_index < _action_set.size();
			}

			//get next action.
			inline const Action& next_action()
			{
				return _action_set[_next_action_index];
			}

			//set the ptr of next child.
			inline void set_next_child(Node* ptr)
			{
				_child_nodes[_next_action_index] = ptr;
			}

			//move the cursor to next action.
			inline void to_next_action()
			{
				_next_action_index++;
			}

			//increase visited time.
			inline void incr_visited_time()
			{
				_visited_time++;
			}

			//increase win time.
			inline void incr_win_time()
			{
				_win_time++;
			}

			//free the node from allocator.
			void FreeFromAllocator(Allocator& allocator)
			{
				//free all child node if possible.
				for (auto p : _child_nodes)
				{
					if (p != nullptr)
					{
						p->FreeFromAllocator(allocator);
					}
				}

				//free the node itself.
				if (is_debug())
				{
					bool b = allocator.free(this);
					GADT_CHECK_WARNING(g_MCTS_NEW_ENABLE_WARNING, b == false, "MCTS105: free child node failed.");
				}
				else
				{
					allocator.free(this);
				}
			}

		public:
			MctsNode(const State& state, const FuncPackage& func) :
				_state(state),
				_winner_index(func.DetemineWinner(state)),
				_visited_time(1),
				_win_time(0),
				_next_action_index(0)
			{
				if (!is_end_state())
				{
					func.MakeAction(_state, _action_set);
					_child_nodes.resize(_action_set.size(), nullptr);
				}
			}

			MctsNode(const MctsNode&) = delete;

			//TODO
			//TODO
			//TODO free child node from allocator by index. return true if free successfully.
			bool FreeChildNode(size_t index, Allocator& allocator)
			{
				//TODO
				return true;
			}

			//4.the simulation result is back propagated through the selected nodes to update their statistics.
			void BackPropagation(const Result& result, const FuncPackage& func)
			{
				if (func.AllowUpdateValue(_state, result))
				{
					incr_win_time();
				}
			}

			//3.simulation is run from the new node according to the default policy to produce a result.
			void SimulationProcess(Result& result, const FuncPackage& func)
			{
				State state = _state;	//copy
				ActionSet actions;
				for (size_t i = 0;;i++)
				{
					if (is_debug()){GADT_CHECK_WARNING(g_MCTS_NEW_ENABLE_WARNING, i > _default_policy_warning_length, "MCTS103: out of default policy process max length.");}
					AgentIndex winner = func.DetemineWinner(state);
					if (winner != _no_winner_index)
					{
						result = func.StateToResult(state, winner);
						break;
					}
					actions.clear();
					func.MakeAction(state, actions);
					const Action& action = func.DefaultPolicy(actions);
					state = func.GetNewState(state, action);
				}

				BackPropagation(result, func);		//update the new value itself.
			}

			//2.one child node would be added to expand the tree, acccording to the available actions.
			void Expandsion(Result& result, Allocator& allocator ,const FuncPackage& func)
			{
				if (is_end_state())
				{
					result = func.StateToResult(_state, _winner_index);//return the result of this node.
					return;
				}
				else
				{
					Node* new_node = allocator.construct(func.GetNewState(_state, next_action()),func);
					set_next_child(new_node);
					to_next_action();
					new_node->SimulationProcess(result, func);
				}
			}

			//1. select the most urgent expandable node,and get the result to update statistic.
			void Selection(Result& result, Allocator& allocator,const FuncPackage& func)
			{
				incr_visited_time();

				if (is_end_state())
				{
					func.StateToResult(_state, result);
				}
				else
				{
					if (exist_unactivated_action())
					{
						Expandsion(result, allocator, func);
					}
					else
					{
						if (is_debug()) { GADT_CHECK_WARNING(g_MCTS_NEW_ENABLE_WARNING, _child_nodes.size() == 0, "MCTS106: empty action set during tree policy."); }
						Node* max_ucb_child_node = _child_nodes[0];
						UcbValue max_ucb_value = 0;
						for (size_t i = 0; i < _child_nodes.size(); i++)
						{
							if (_child_nodes[i] != nullptr)
							{
								UcbValue child_node_ucb_value = func.TreePolicyValue(*this, *_child_nodes[i]);
								if (child_node_ucb_value > max_ucb_value)
								{
									max_ucb_child_node = _child_nodes[i];
									max_ucb_value = child_node_ucb_value;
								}
							}
						}
						if (is_debug()) { GADT_CHECK_WARNING(g_MCTS_NEW_ENABLE_WARNING, max_ucb_child_node == nullptr, "MCTS108: best child node pointer is nullptr."); }
						max_ucb_child_node->Selection(result, allocator, func);
					}
				}

				//backpropagation process for this node.update value;
				BackPropagation(result, func);
			}

			//return true if the state is the terminal-state of the game.
			inline bool is_end_state() const
			{
				return _winner_index != _no_winner_index;
			}

			//get info of this node.
			std::string info() const
			{
				std::stringstream ss;
				double avg = static_cast<double>(win_time()) / static_cast<double>(visited_time());
				ss << "{ visited:" << visited_time() << " win:" << win_time() <<" avg:" << avg << " child";
				if (exist_unactivated_action())
				{
					ss << next_action_index() << "/" << action_set().size();
				}
				else
				{
					ss << child_set().size() << "/" << child_set().size();
				}
				ss << " }";
				return ss.str();
			}

			//return the value of _is_debug.
			constexpr inline bool is_debug() const
			{
				return _is_debug;
			}
		};

		//convert mcts search tree to json.
		template<typename State, typename Action, typename Result, bool _is_debug>
		class MctsToJson
		{
		public:
			using SearchNode     = MctsNode<State, Action, Result, _is_debug>;
			using VisualTree     = visual_tree::VisualTree;
			using VisualNode	 = visual_tree::VisualNode;
			using StateToStrFunc = std::function<std::string(const State& state)>;
			using CustomInfoFunc = std::function<void(const SearchNode&, VisualNode&)>;

		private:
			const char* DEPTH_NAME           = "depth";
			const char* COUNT_NAME           = "tree_size";
			const char* STATE_NAME           = "state";
			const char* WINNER_INDEX_NAME    = "winner";
			const char* VISITED_TIME_NAME    = "visited_time";
			const char* WIN_TIME_NAME        = "win_time";
			const char* CHILD_NUM_NAME       = "child_number";
			const char* IS_TERMIANL_NAME     = "is_terminal";
			
		private:
			SearchNode*    _mcts_root_node;
			VisualTree     _json_tree;
			bool           _include_state;
			StateToStrFunc _StateToStr;
			CustomInfoFunc _CustomInfo;

		private:
			//search node convert to json node.
			void convert_node(const SearchNode& search_node, VisualNode& visual_node)
			{
				visual_node.add_value(DEPTH_NAME, visual_node.depth());
				visual_node.add_value(WINNER_INDEX_NAME, search_node.winner_index());
				visual_node.add_value(VISITED_TIME_NAME, search_node.visited_time());
				visual_node.add_value(WIN_TIME_NAME, search_node.win_time());
				visual_node.add_value(CHILD_NUM_NAME, search_node.child_num());
				visual_node.add_value(IS_TERMIANL_NAME, search_node.is_end_state());
				if (_include_state)
				{
					visual_node.add_value(STATE_NAME, _StateToStr(search_node.state()));
				}
				_CustomInfo(search_node, visual_node);
				for (size_t i = 0;i<search_node.child_num();i++)
				{
					auto node_ptr = search_node.child_node(i);
					if (node_ptr != nullptr)
					{
						visual_node.create_child();
						convert_node(*search_node.child_node(i), *visual_node.last_child());
					}
				}
			}

			//add count
			void AddCount(VisualNode& node)
			{
				node.add_value(COUNT_NAME, node.count());
			}

			//search tree convert to json tree.
			void ConvertToVisualTree(SearchNode* mcts_root_node,VisualTree& visual_tree)
			{
				visual_tree.clear();
				convert_node(*mcts_root_node, *visual_tree.root_node());//generate new visual tree.
				visual_tree.traverse_nodes([&](VisualNode& node)->void {AddCount(node); });	//refresh count value.
			}

		public:
			//constructor function.
			MctsToJson(SearchNode* mcts_root_node, StateToStrFunc StateToStr, CustomInfoFunc CustomInfo = [](const SearchNode&, VisualNode&)->void {}) :
				_mcts_root_node(mcts_root_node),
				_json_tree(),
				_include_state(true),
				_StateToStr(StateToStr),
				_CustomInfo(CustomInfo)
			{
				ConvertToVisualTree(mcts_root_node, _json_tree);
			}

			MctsToJson(SearchNode* mcts_root_node):
				_mcts_root_node(mcts_root_node),
				_json_tree(),
				_include_state(false),
				_StateToStr([](const State&)->std::string { return ""; }),
				_CustomInfo([](const SearchNode&, VisualNode&)->void {})
			{
				ConvertToVisualTree(mcts_root_node, _json_tree);
			}

			//refresh json tree by mcts search tree.
			inline void refresh()
			{
				_json_tree = ConvertToVisualTree(_mcts_root_node);
			}

			//set custom info function.
			inline void set_custom_info(CustomInfoFunc CustomInfo)
			{
				_CustomInfo = CustomInfo;
			}

			//output json.
			inline void output_json(std::ostream& os)
			{
				_json_tree.output_json(os);
			}
		};

		/*
		* MctsSearch is a template of monte carlo tree search.
		*
		* [State] is the game-state class, which is defined by the user.
		* [Action] is the game-action class, which is defined by the user.
		* [Result] is the game-result class, which stand for a terminal state of the game.
		* [_is_debug] means some debug info would not be ignored if it is true. this may result in a little degradation of performance.
		*/
		template<typename State, typename Action, typename Result, bool _is_debug = false>
		class MctsSearch
		{
		public:
			using Node       = MctsNode<State, Action, Result, _is_debug>;	  //searcg node.	
			using VisualTree = MctsToJson<State, Action, Result, _is_debug>; //json tree
			using Allocator  = typename Node::Allocator;							  //allocator of nodes
			using ActionSet  = typename Node::ActionSet;							  //set of Action
			using NodePtrSet = typename Node::NodePtrSet;							  //set of ptrs to child nodes.
			
		private:
			using FuncPackage	= typename Node::FuncPackage;
			struct DefaultFuncPackage
			{
				typename FuncPackage::TreePolicyValueFunc		TreePolicyValue;
				typename FuncPackage::DefaultPolicyFunc			DefaultPolicy;
				typename FuncPackage::AllowExtendFunc			AllowExtend;
				typename FuncPackage::AllowExcuteGcFunc			AllowExcuteGc;
				typename FuncPackage::ValueForRootNodeFunc		ValueForRootNode;
			};
			struct LogFuncPackage
			{
				using StateToStrFunc	= std::function<std::string(const State& state)>;
				using ActionToStrFunc	= std::function<std::string(const Action& action)>;
				using ResultToStrFunc	= std::function<std::string(const Result& result)>;

				StateToStrFunc		StateToStr;
				ActionToStrFunc		ActionToStr;
				ResultToStrFunc		ResultToStr;
			};

		private:
			FuncPackage		_func_package;			//function package of the search.
			Allocator&		_allocator;				//the allocator for the search.
			const bool		_private_allocator;		//use private allocator.
			double			_timeout;				//set timeout (seconds).
			size_t			_max_iteration;			//set max iteration times.
			bool			_enable_gc;				//allow garbage collection if the tree run out of memory.
			bool			_enable_log;			//enable log visualization.
			std::ostream*	_log_ptr;				//pointer to log ostream.

			const char* MCTS_JSON_LOG_NAME = "MctsJsonLog.txt";

		public:
			//package of default functions.
			const DefaultFuncPackage DefaultFunc;
			LogFuncPackage LogFunc;

		private:

			//get reference of log ostream
			inline std::ostream& log()
			{
				return *_log_ptr;
			}

			//get info of this search.
			std::string info() const
			{
				std::stringstream ss;
				ss << std::boolalpha << "{" << std::endl
					<< "    allocator: " << _allocator.info() << std::endl
					<< "    is_private_allocator: " << _private_allocator << std::endl
					<< "    timeout: " << _timeout << std::endl
					<< "    max_iteration: " << _max_iteration << std::endl
					<< "    enable_gc: " << _enable_gc << std::endl
					<< "    enable_log: " << _enable_log << std::endl
					<< "}" << std::endl;
				return ss.str();
			}

			//enable log
			inline bool enable_log() const
			{
				return _enable_log;
			}

			//enable gc
			inline bool enable_gc() const
			{
				return _enable_gc;
			}

			//define functions as default.
			DefaultFuncPackage DefaultFuncInit()
			{
				auto TreePolicyValue = [](const Node& parent, const Node& child)->UcbValue{
					UcbValue avg = static_cast<UcbValue>(child.win_time()) / static_cast<UcbValue>(child.visited_time());
					return policy::UCB1(avg, static_cast<UcbValue>(parent.visited_time()), static_cast<UcbValue>(child.visited_time()));
				};
				auto DefaultPolicy = [](const ActionSet& actions)->const Action&{
					if (_is_debug) 
					{
						GADT_CHECK_WARNING(g_MCTS_NEW_ENABLE_WARNING, actions.size() == 0, "MCTS104: empty action set during default policy.");
					}
					return actions[rand() % actions.size()];
				};
				auto AllowExtend = [](const Node& node)->bool {
					return true; 
				};
				auto AllowExcuteGc = [](const Node& node)->bool{
					if (node.visited_time() < 10)
					{
						return true;
					}
					return false;
				};
				auto ValueForRootNode = [](const Node& node)->UcbValue{
					return static_cast<UcbValue>(node.visited_time());
				};
				return {
					TreePolicyValue,
					DefaultPolicy,
					AllowExtend,
					AllowExcuteGc,
					ValueForRootNode
				};
			}

			//function package initilize.
			void FuncInit()
			{
				_func_package.TreePolicyValue	= DefaultFunc.TreePolicyValue;
				_func_package.DefaultPolicy		= DefaultFunc.DefaultPolicy;
				_func_package.AllowExtend		= DefaultFunc.AllowExtend;
				_func_package.AllowExcuteGc		= DefaultFunc.AllowExcuteGc;
				_func_package.ValueForRootNode	= DefaultFunc.ValueForRootNode;
			}

			//excute iteration function.
			Action ExcuteMCTS(State root_state, double timeout, size_t max_iteration, bool enable_gc)
			{
				if (_enable_log)
				{
					log() << "[MCTS] start excute monte carlo tree search..." << std::endl
						<< "[MCTS] info = " << info() << std::endl;
				}
				Node* root_node = _allocator.construct(root_state, _func_package);
				ActionSet root_actions = root_node->action_set();
				timer::TimePoint start_tp;
				size_t iteration_time = 0;
				for (iteration_time = 0; iteration_time < max_iteration; iteration_time++)
				{
					//stop search if timout.
					if (start_tp.time_since_created() > timeout && is_debug() == false)
					{
						break;//timeout, stop search.
					}

					//excute garbage collection if need.
					if (_allocator.is_full())
					{
						if (enable_gc)
						{
							//do garbage collection.

							if (_allocator.is_full())
							{
								break;//stop search if gc failed.
							}
						}
						else
						{
							break;//run out of memory, stop search.
						}
					}

					Result new_result;
					root_node->Selection(new_result, _allocator, _func_package);
				}

				//return the best result
				if (is_debug()) { GADT_CHECK_WARNING(g_MCTS_NEW_ENABLE_WARNING, root_actions.size() == 0, "MCTS101: root node do not exist any available action."); }
				UcbValue max_value = 0;
				size_t max_value_node_index = 0;
				if (_enable_log) 
				{ 
					VisualTree json_tree(root_node, LogFunc.StateToStr);
					std::ofstream os(MCTS_JSON_LOG_NAME);
					json_tree.output_json(os);
					log() << "[MCTS] iteration finished." << std::endl
						<< "[MCTS] actions = {" << std::endl;
				}
				for (size_t i = 0; i < root_actions.size(); i++)
				{
					auto child_ptr = root_node->child_node(i);
					if (is_debug()) { GADT_CHECK_WARNING(g_MCTS_NEW_ENABLE_WARNING, root_node->child_node(0) == nullptr, "MCTS107: empty child node under root node."); }
					if (_enable_log)
					{
						log() << "action " << i << ": "<< LogFunc.ActionToStr(root_actions[i])<<", value: ";
					}
					if (child_ptr != nullptr)
					{
						UcbValue child_value = _func_package.ValueForRootNode(*child_ptr);
						if (child_value > max_value)
						{
							max_value = child_value;
							max_value_node_index = i;
						}
						if (_enable_log)
						{
							log() << "[" << child_value << "]" << std::endl;
						}
					}
					else
					{
						if (_enable_log)
						{
							log() << "[ deleted ]" << std::endl;
						}
					}
				}
				if (_enable_log)
				{
					log() << "[MCTS] best action index: "<< max_value_node_index << std::endl;
				}
				if (is_debug()) { GADT_CHECK_WARNING(g_MCTS_NEW_ENABLE_WARNING, root_actions.size() == 0, "MCTS102: best value for root node equal to 0."); }
				return root_actions[max_value_node_index];
			}

		public:
			//use private allocator.
			MctsSearch(
				typename FuncPackage::GetNewStateFunc		_GetNewState,
				typename FuncPackage::MakeActionFunc		_MakeAction,
				typename FuncPackage::DetemineWinnerFunc	_DetemineWinner,
				typename FuncPackage::StateToResultFunc		_StateToResult,
				typename FuncPackage::AllowUpdateValueFunc	_AllowUpdateValue,
				size_t max_node
			):
				_func_package(
					_GetNewState,
					_MakeAction,
					_DetemineWinner,
					_StateToResult,
					_AllowUpdateValue
				),
				_allocator(*(new Allocator(max_node))),
				_private_allocator(true),
				_enable_log(false),
				_log_ptr(&std::cout),
				DefaultFunc(DefaultFuncInit())
			{
				FuncInit();
			}

			//use public allocator.
			MctsSearch(
				typename FuncPackage::GetNewStateFunc		_GetNewState,
				typename FuncPackage::MakeActionFunc		_MakeAction,
				typename FuncPackage::DetemineWinnerFunc	_DetemineWinner,
				typename FuncPackage::StateToResultFunc		_StateToResult,
				typename FuncPackage::AllowUpdateValueFunc	_AllowUpdateValue,
				Allocator allocator
			):
				_func_package(
					_GetNewState,
					_MakeAction,
					_DetemineWinner,
					_StateToResult,
					_AllowUpdateValue
				),
				_allocator(allocator),
				_private_allocator(false), 
				_enable_log(false),
				_log_ptr(&std::cout),
				DefaultFunc(DefaultFuncInit())
			{
				FuncInit();
			}

			//deconstructor function
			~MctsSearch()
			{
				if (_private_allocator)
				{
					delete &_allocator;
				}
			}

			//do search with default parameters.
			Action DoMcts(const State root_state)
			{
				return ExcuteMCTS(root_state, _timeout, _max_iteration, _enable_gc);
			}

			//do search with custom parameters.
			Action DoMcts(const State root_state, double timeout, size_t max_iteration, bool enable_gc)
			{
				return ExcuteMCTS(root_state, timeout, max_iteration, enable_gc);
			}

			//enable log output to ostream.
			inline void EnableLog(
				typename LogFuncPackage::StateToStrFunc     StateToStr,
				typename LogFuncPackage::ActionToStrFunc    ActionToStr,
				typename LogFuncPackage::ResultToStrFunc    ResultToStr,
				std::ostream& log = std::cout
			)
			{
				LogFunc = { StateToStr,ActionToStr,ResultToStr };
				_enable_log = true;
				_log_ptr = &log;
			}

			inline void DisableLog()
			{
				_enable_log = false;
			}

			//return the value of _is_debug.
			constexpr inline bool is_debug() const
			{
				return _is_debug;
			}
		};

	}
}