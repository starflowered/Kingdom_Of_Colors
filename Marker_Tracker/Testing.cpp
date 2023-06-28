#include "Testing.h"
void viewMissions(GameLogic& gamelogic)
{
	for (auto mission : gamelogic.get_current_missions())
	{
		std::cout << std::get<0>(mission) << std::endl;
	}
}

void TESTING()
{
	GameLogic gamelogic;
	viewMissions(gamelogic);
}

int main()
{
	TESTING();
}