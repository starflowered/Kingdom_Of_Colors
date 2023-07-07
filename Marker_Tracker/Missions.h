#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <tuple>
#include <functional>
#include <vector>

#include "GameLogic_Utilities.h"
#include "MarkerDetectionUtilities.h"

// Forward declaration of the marker struct
struct marker;

#define SCORE_PER_MISSION 100

using Mission = std::tuple<std::string, std::function<int(int, std::unordered_map<int, std::array<bool, 6>>)>, int>;
class Missions
{
public:
	//tuples of the form <task formulation, number of function, chosen color>
	std::array<Mission ,3> get_current_random_missions();
	int computeMissionScore();
	Missions();
	void output_missions();
	std::string get_mission_status_as_string(int missionNr);
	

private:
	//state of current missions (has the player completed them or not?)
	std::array<bool, 3> finished_missions;

	//array with the current missions of this game's playthrough. stores their formulation, the function to check whether mission is successfully
	//achieved and the color index of the mission (e.g. mission uses red tiles -> stores color index for red)
	std::array<Mission, 3> current_missions;
	
	
	std::function<int(int, std::unordered_map<int, std::array<bool, 6>>)> f_three_tiles= std::bind(&three_tiles_in_a_row, std::placeholders::_1, std::placeholders::_2);
	std::function<int(int, std::unordered_map<int, std::array<bool, 6>>)> f_triangle = std::bind(&three_half_color,  std::placeholders::_1,  std::placeholders::_2);
	std::function<int(int, std::unordered_map<int, std::array<bool, 6>>)> f_7_of_color= std::bind(&seven_of_color,  std::placeholders::_1, std::placeholders::_2);
	
	
	//tuples of the form <task formulation, corresponding function>
	std::vector<std::tuple<std::string, std::function<int(int, std::unordered_map<int, std::array<bool, 6>>)>>> possible_missions{
		std::make_tuple("Place three tiles in a row, matching in the following color: ", f_three_tiles),
		std::make_tuple("Match all sides of three two-color tiles having the following color on one side: ",f_triangle),
		std::make_tuple("Match all sides of the following color of at least 7 different tiles: ",f_7_of_color)
	
	};


	static int three_tiles_in_a_row(int color, std::unordered_map<int, std::array<bool, 6>> matches_of_tiles);
	static int three_half_color(int color, std::unordered_map<int, std::array<bool, 6>> matches_of_tiles);
	static int seven_of_color(int color, std::unordered_map<int, std::array<bool, 6>> matches_of_tiles);
};

