#include "Testing.h"

void viewMissions(GameLogic& game_logic)
{
	for (auto mission : game_logic.get_current_missions())
	{
		std::cout << std::get<0>(mission) << std::endl;
	}
}

void marker_color_output()
{
	for (int i = 0; i < 100; i++)
	{
		std::cout << i << ": " << GameLogic_Utilities::get_name_of_color_by_index(GameLogic_Utilities::determine_marker_color(i)) 
			<< " (" << GameLogic_Utilities::determine_marker_color(i) <<" )" << std::endl;
	}
	
}

void testCalcSingleMultiplier()
{

}

int main()
{
	GameLogic game_logic;
	//viewMissions(game_logic);
	//marker_color_output();
}