#include "Testing.h"



void marker_color_output()
{
	for (int i = 0; i < 100; i++)
	{
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

int main()
{
	GameLogic game_logic;
	//testCalcSingleMultiplier();
	//viewMissions(game_logic);
	//marker_color_output();
}