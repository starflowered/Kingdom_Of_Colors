// GameLogic_Tests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "../Marker_Tracker/GameLogic.h"

//using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void viewMissions(GameLogic& gamelogic)
{
    for (auto mission : gamelogic.get_current_missions())
    {
      // std::cout << "First mission: " << std::get<0>(mission) << std::endl;
    }
}
//writing normal methods because I dont wanna use Microsoft/Google products (Florence verwendet ja glaub ich auch clion)
int main()
{
    GameLogic gamelogic;
    viewMissions(gamelogic);
}



//static int determine_marker_color(int marker_id);

//void calculate_multipliers(int max_hex_id);
//int calculate_game_score(const vector<tuple<marker, marker>>& matches);
