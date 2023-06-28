#include "Testing.h"
void viewMissions(GameLogic& gamelogic)
{
	for (auto mission : gamelogic.get_current_missions())
	{
		std::cout << std::get<0>(mission) << std::endl;
	}
}


void missionScore()
{
	
}
void marker_color_output()
{
	for (int i = 0; i < 100; i++)
	{
		std::cout << i << ": " << GameLogic_Utilities::get_name_of_color_by_index(GameLogic_Utilities::determine_marker_color(i)) 
			<< " (" << GameLogic_Utilities::determine_marker_color(i) <<" )" << std::endl;
	}
	
}
void TESTING()
{
	GameLogic gamelogic;
	viewMissions(gamelogic);
	marker_color_output();
}

int main()
{
	TESTING();
}