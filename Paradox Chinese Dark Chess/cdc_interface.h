#include "cdc_define.h"
#include "monte_carlo.h"
#include "../include/json11.hpp"

#pragma once

using json11::Json;
using gadt::log::ErrorLog;

namespace chinese_dark_chess
{
	namespace json_interface
	{
		PieceType JsonToPiece(Json json, ErrorLog& err);

		PlayerIndex JsonToPlayer(Json json, ErrorLog& err);

		Location JsonToLocation(Json json, ErrorLog& err);

		State JsonToState(Json json, ErrorLog& err);

		Json LocationToJson(Location loc);

		Json PlayerToJson(PlayerIndex p);

		Json PieceToJson(PieceType p);

		Json ActionToJson(const Action& action);

		Json StateToJson(const State& state);

		std::string ChineseDarkChessAI(std::string json_str, std::string log_dir, std::string err_dir);
	}
}