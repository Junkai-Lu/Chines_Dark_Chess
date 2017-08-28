#include "cdc_interface.h"

using json11::Json;
using gadt::log::ErrorLog;

/*
Json:
{
data:[[],[]...(8)]
next_player:
hidden_pieces:
}

*/

namespace chinese_dark_chess
{
	namespace  json_interface
	{
		const char*	g_AI_VERSION = "7.5.01";		//版本号
		const char*	g_LOG_SUFFIX = ".log";			//文件后缀名
		const bool	g_PRINT_RESPOND = false;		//打印结果
		const bool	g_ENABLE_REQUEST_LOG = true;	//开启请求日志
		const bool  g_ENABLE_ERROR_LOG = true;		//开启错误日志

		PieceType JsonToPiece(Json json, ErrorLog & err)
		{
			if (json.is_string())
			{
				std::string str = json.string_value();
				switch (str[0])
				{
				case '?':
					return PIECE_UNKNOWN;
				case 'P':
					return PIECE_RED_PAWN;
				case 'C':
					return PIECE_RED_CANNON;
				case 'N':
					return PIECE_RED_KNIGHT;
				case 'R':
					return PIECE_RED_ROOK;
				case 'M':
					return PIECE_RED_MINISTER;
				case 'G':
					return PIECE_RED_GUARD;
				case 'K':
					return PIECE_RED_KING;
				case 'p':
					return PIECE_BLACK_PAWN;
				case 'c':
					return PIECE_BLACK_CANNON;
				case 'n':
					return PIECE_BLACK_KNIGHT;
				case 'r':
					return PIECE_BLACK_ROOK;
				case 'm':
					return PIECE_BLACK_MINISTER;
				case 'g':
					return PIECE_BLACK_GUARD;
				case 'k':
					return PIECE_BLACK_KING;
				case ' ':
					return PIECE_EMPTY;
				default:
					break;
				}
			}
			err.add("JsonToPiece failed, value = " + json.string_value());
			return PieceType();
		}

		PlayerIndex JsonToPlayer(Json json, ErrorLog & err)
		{
			if (json.is_string())
			{
				std::string str = json.string_value();
				if (str == "RED")
				{
					return PLAYER_RED;
				}
				if (str == "BLACK")
				{
					return PLAYER_BLACK;
				}
			}
			err.add("JsonToPlayer failed, value = " + json.string_value());
			return PlayerIndex();
		}

		Location JsonToLocation(Json json, ErrorLog& err)
		{
			if (json.is_array())
			{
				if (json.array_items().size() == 2)
				{
					if (json.array_items()[0].is_number() && json.array_items()[1].is_number())
					{
						return Location(size_t(json.array_items()[0].int_value()), size_t(json.array_items()[1].int_value()));
					}
					else { err.add("JsonToLocation arr is not number"); }
				}
				else { err.add("JsonToLocation array size is not 2"); }
			}
			else { err.add("JsonToLocation failed"); }
			return Location(0);
		}

		State JsonToState(Json json, ErrorLog & err)
		{
			if (json.is_object())
			{
				std::vector<std::vector<PieceType>> data(8, { PIECE_EMPTY,PIECE_EMPTY,PIECE_EMPTY,PIECE_EMPTY });
				const Json& data_json = json["data"];
				if (data_json.is_array())
				{
					if (data_json.array_items().size() == g_CDC_BOARD_WIDTH)
					{
						for (size_t x = 0; x < data_json.array_items().size(); x++)
						{
							const Json& arr = data_json.array_items()[x];
							if (arr.is_array())
							{
								if (arr.array_items().size() == g_CDC_BOARD_HEIGHT)
								{
									for (size_t y = 0; y < arr.array_items().size(); y++)
									{
										(data[x])[y] = JsonToPiece(arr.array_items()[y], err);
									}
								}
								else { err.add("json array size is not 4."); }
							}
							else { err.add("arr is not array."); }
						}
					}
					else { err.add("json array size is not 8."); }
				}
				else { err.add("json is not array."); }

				std::vector<PieceType> hidden_pieces;
				const Json& hidden_pieces_json = json["hidden_pieces"];
				if (hidden_pieces_json.is_array())
				{
					for (auto j : hidden_pieces_json.array_items())
					{
						hidden_pieces.push_back(JsonToPiece(j, err));
					}
				}

				PlayerIndex next_player = JsonToPlayer(json["next_player"], err);
				return StateData(data, hidden_pieces, next_player).to_state();
			}
			else { err.add("JsonToState, json is not object"); }
			return State();
		}

		Json LocationToJson(Location loc)
		{
			Json::array arr = {
				Json((int)loc.x),
				Json((int)loc.y)
			};
			return arr;
		}

		Json PieceToJson(PieceType p)
		{
			switch (p)
			{
			case PIECE_UNKNOWN:
				return Json("?");
			case PIECE_UNDECIDED:
				GADT_CHECK_WARNING(true, true, "find undecied state");
				return Json(" ");
			case PIECE_RED_PAWN:
				return Json("R");
			case PIECE_RED_CANNON:
				return Json("C");
			case PIECE_RED_KNIGHT:
				return Json("N");
			case PIECE_RED_ROOK:
				return Json("R");
			case PIECE_RED_MINISTER:
				return Json("M");
			case PIECE_RED_GUARD:
				return Json("G");
			case PIECE_RED_KING:
				return Json("K");
			case PIECE_BLACK_PAWN:
				return Json("p");
			case PIECE_BLACK_CANNON:
				return Json("c");
			case PIECE_BLACK_KNIGHT:
				return Json("n");
			case PIECE_BLACK_ROOK:
				return Json("r");
			case PIECE_BLACK_MINISTER:
				return Json("m");
			case PIECE_BLACK_GUARD:
				return Json("g");
			case PIECE_BLACK_KING:
				return Json("k");
			case PIECE_EMPTY:
				return Json(" ");
			default:
				break;
			}
			return Json(" ");
		}

		Json ActionToJson(const Action & action)
		{
			Json::object obj = {
				{ "from", LocationToJson(Location(action.source)) },
				{ "to", LocationToJson(Location(action.dest)) },
				{ "piece", PieceToJson(action.piece) }
			};
			return obj;
		}

		Json StateToJson(const State & state)
		{
			StateData data(state);
			return Json();
		}

		std::string ChineseDarkChessAI(std::string json_str, std::string log_dir, std::string err_dir)
		{
			ErrorLog err;
			gadt::timer::TimePoint tm;

			const std::string default_respond = "null";
			const size_t mc_times = 10000;

			std::string parse_err;
			Json request_json = Json::parse(json_str, parse_err);
			
			State state = JsonToState(request_json, err);

			//return null if exist any error.
			if (!err.is_empty())
			{
				std::ofstream err_log(err_dir + "error_" + gadt::timer::TimeString("%m-%d-%H") + g_LOG_SUFFIX, std::ios::app);
				if (g_PRINT_RESPOND)
				{
					std::cout << "FzmError: Request Failed! " << std::endl;
				}
				if (g_ENABLE_ERROR_LOG)
				{
					err_log << "Error = {" << std::endl;
					err_log << "  request = " << json_str << std::endl;
					err_log << "  error = " << err.output() << std::endl;
					err_log << "  time = " << gadt::timer::TimeString() << std::endl << "}" << std::endl << std::endl;
				}
				return default_respond;
			}

			MonteCarlo mc(state);
			Action act = mc.DoMonteCarlo(mc_times);
			std::string respond_str = ActionToJson(act).dump();

			//write logs.
			if (g_PRINT_RESPOND)
			{
				std::cout << respond_str << std::endl;
			}

			if (g_ENABLE_REQUEST_LOG)
			{
				std::ofstream log(log_dir + "Req_" + gadt::timer::TimeString("%m-%d-%H") + g_LOG_SUFFIX, std::ios::app);
				log << "{" << std::endl;
				log << "  \"request\":" << json_str << "," << std::endl;
				log << "  \"respond\":" << respond_str << "," << std::endl;
				log << "  \"time\":\"" << gadt::timer::TimeString() << "\"" << std::endl << "}," << std::endl << std::endl;
			}
			return respond_str;
		}

	}
}


