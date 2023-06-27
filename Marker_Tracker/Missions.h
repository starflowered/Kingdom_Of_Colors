#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <tuple>
#include <functional>
#include <vector>

#include "GameLogic_Utilities.h"
#include "MarkerDetectionUtilities.h"


#define SCORE_PER_MISSION 100
class Missions
{
public:
	//tuples of the form <task formulation, number of function, chosen color>
	std::array<std::tuple<std::string, std::function<int(int)>, int> ,3> get_current_random_missions();
	int computeMissionScore();
	Missions();
	void update_tile_matches(const std::vector<std::tuple<marker, marker>>& matches, int max_hex_id);

private:
	std::unordered_map<int, std::array<bool, 6>> matches_of_tiles;
	std::array<std::tuple<std::string, std::function<int(int)>, int>, 3> current_missions;
	int three_tiles_in_a_row(int color);
	int three_half_color(int color);
	int seven_of_color( int color);

	std::function<int(int)> f_three_tiles= std::bind(&three_tiles_in_a_row, this, std::placeholders::_1);
	std::function<int(int)> f_triangle = std::bind(&three_half_color, this, std::placeholders::_1);
	std::function<int(int)> f_7_of_color= std::bind(&seven_of_color, this, std::placeholders::_1);
	
	
	//tuples of the form <task formulation, corresponding function>
	std::vector<std::tuple<std::string, std::function<int(int)>>> possible_missions{
		std::make_tuple("Place three tiles in a row, matching in the following color: ", f_three_tiles),
		std::make_tuple("Match all sides of three two-color tiles having the following color on one side: ",f_triangle),
		std::make_tuple("Match all sides of the following color of at least 7 different tiles: ",f_7_of_color)
	
	};
	


};

