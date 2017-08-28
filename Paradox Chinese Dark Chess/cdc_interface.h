#include "cdc_define.h"
#include "monte_carlo.h"
#include "./include/json11.hpp"

#pragma once

using json11::Json;
using gadt::log::ErrorLog;

namespace chinese_dark_chess
{
	namespace json_interface
	{
		PieceType JsonToPiece(Json json, ErrorLog& err);

		PlayerIndex JsonToPlayer(Json json, ErrorLog& err);

		State JsonToState(Json json, ErrorLog& err);

		Json StateToJson(const State& state);

		Json ActionToJson(const Action& action);
	}
}