#include "Testing.h"



void marker_color_output()
{
	for (int i = 0; i < 150; i++)
	{
		if (i % 6 == 0)
			std::cout << (i/6) << " new hex ---------" << std::endl;
		std::cout << i << ": " << GameLogic_Utilities::get_name_of_color_by_index(GameLogic_Utilities::determine_marker_color(i)) 
			<< " (" << GameLogic_Utilities::determine_marker_color(i) <<" )" << std::endl;
	}
	
}
/*
void testCalcSingleMultiplier()
{
	bool b1[] = {true,true,true,true,true,true};
	bool b2[] = { true, true, true,true, true, false };
	bool b3[] = { true, true, false,true, true, false };
	bool b4[] = { false, true, false,true, true, false };

	for (int i = 0; i <= 12; i++)
	{
		std::cout << "card type " << i << ", b1: " << GameLogic::calc_single_multiplier(b1, i) << std::endl;
		std::cout << "card type " << i << ", b2: " << GameLogic::calc_single_multiplier(b2, i) << std::endl;
		std::cout << "card type " << i << ", b3: " << GameLogic::calc_single_multiplier(b3, i) << std::endl;
		std::cout << "card type " << i << ", b4: " << GameLogic::calc_single_multiplier(b4, i) << std::endl;

		if ((i+1) % 3 == 0)
			std::cout << "---------------------------------"<< std::endl;
	}

}*/

/*
void test_calc_multipliers()
{
	GameLogic game_logic;
	game_logic.marker_id_matches[0] = true;
	game_logic.marker_id_matches[1] = true;
	game_logic.marker_id_matches[2] = true;
	game_logic.marker_id_matches[3] = true;
	game_logic.marker_id_matches[4] = true;
	game_logic.marker_id_matches[18] = true;
	game_logic.marker_id_matches[19] = true;
	game_logic.marker_id_matches[20] = true;
	game_logic.marker_id_matches[21] = true;
	game_logic.marker_id_matches[22] = true;
	game_logic.marker_id_matches[23] = true;
	game_logic.marker_id_matches[36] = true;
	game_logic.marker_id_matches[37] = true;
	game_logic.marker_id_matches[38] = true;
	game_logic.marker_id_matches[39] = true;
	game_logic.update_tile_matches_per_marker(8);
	auto result = GameLogic_Utilities::get_current_matched_markers_per_tiles();
	for (auto m : result)
	{
		std::cout << m.first << ": " << m.second[0] << ", " << m.second[1] << ", "
			<< m.second[2] << ", "
			<< m.second[3] << ", "
			<< m.second[4] << ", "
			<< m.second[5] << ", " << std::endl;
	}
	game_logic.calculate_multipliers(8);
	for (auto m : game_logic.marker_multipliers)
	{
		std::cout << m.first << ": " << m.second << std::endl;
	}
	}
*/
/*
void test_missions()
{
	GameLogic game_logic;
	game_logic.marker_id_matches[0] = true;
	game_logic.marker_id_matches[1] = true;
	game_logic.marker_id_matches[2] = true;
	game_logic.marker_id_matches[3] = true;
	game_logic.marker_id_matches[4] = true;
	game_logic.marker_id_matches[5] = true;
	game_logic.marker_id_matches[6] = true;
	game_logic.marker_id_matches[7] = true;
	game_logic.marker_id_matches[8] = true;
	game_logic.marker_id_matches[9] = true;
	game_logic.marker_id_matches[10] = true;
	game_logic.marker_id_matches[11] = true;
	game_logic.marker_id_matches[24] = true;
	game_logic.marker_id_matches[25] = true;
	game_logic.marker_id_matches[26] = true;
	game_logic.marker_id_matches[27] = true;
	game_logic.marker_id_matches[28] = true;
	game_logic.marker_id_matches[29] = true;
	game_logic.marker_id_matches[30] = true;
	game_logic.marker_id_matches[31] = true;
	game_logic.marker_id_matches[32] = true;
	game_logic.marker_id_matches[33] = true;
	game_logic.marker_id_matches[34] = true;
	game_logic.marker_id_matches[35] = true;
	game_logic.marker_id_matches[18] = true;
	game_logic.marker_id_matches[19] = true;
	game_logic.marker_id_matches[20] = true;
	game_logic.marker_id_matches[21] = true;
	game_logic.marker_id_matches[22] = true;
	game_logic.marker_id_matches[23] = true;
	game_logic.marker_id_matches[78] = true;
	game_logic.marker_id_matches[79] = true;
	game_logic.marker_id_matches[82] = true;
	game_logic.marker_id_matches[60] = true;
	game_logic.marker_id_matches[61] = true;
	game_logic.marker_id_matches[62] = true;
	game_logic.marker_id_matches[63] = true;
	game_logic.marker_id_matches[64] = true;
	game_logic.marker_id_matches[65] = true;
	game_logic.marker_id_matches[75] = true;
	game_logic.marker_id_matches[76] = true;
	game_logic.marker_id_matches[77] = true;
	game_logic.marker_id_matches[92] = true;
	game_logic.marker_id_matches[93] = true;
	game_logic.marker_id_matches[97] = true;
	game_logic.marker_id_matches[96] = true;
	
	game_logic.update_tile_matches_per_marker(20);
	auto result = GameLogic_Utilities::get_current_matched_markers_per_tiles();
	for (auto m : result)
	{
		std::cout << m.first << ": " << m.second[0] << ", " << m.second[1] << ", "
			<< m.second[2] << ", "
			<< m.second[3] << ", "
			<< m.second[4] << ", "
			<< m.second[5] << ", " << std::endl;
	}
	std::cout <<"mission score:"<< game_logic.missions.computeMissionScore() << std::endl;

}*/

void test_processing_of_matches()
{
	vector<tuple<marker, marker>> matches;
	marker m1, m2, m3, m4,m5;
	m1.hexagon_id = 0;
	m1.marker_id = 0;
	m2.marker_id = 1;
	m2.hexagon_id = 0;
	m3.hexagon_id = 1;
	m3.marker_id = 7;
	m4.hexagon_id = 9;
	m4.marker_id = 56;
	m5.marker_id = 19;
	m5.hexagon_id = 6;
	matches.push_back(std::make_tuple(m1, m2));
	matches.push_back(std::make_tuple(m1, m3));
	matches.push_back(std::make_tuple(m1, m4));
	matches.push_back(std::make_tuple(m4, m5));
	GameLogic game_logic;
	std::cout << "Max hex id " << game_logic.process_matches_of_next_frame(matches) << std::endl;

	for (auto m : game_logic.marker_id_matches)
	{
		std::cout << "Tile " << m.first << " matched " << std::endl;
	}
	for (auto m : game_logic.hex_tile_scores)
	{
		std::cout << "Hex Score of " <<m.first <<": " << m.second << std::endl;
	}
}

int main()
{
	
	//testCalcSingleMultiplier();
	//viewMissions(game_logic);
	//test_missions();
	test_processing_of_matches();
	marker_color_output();
	
}