#include "GameLogic.h"

/**
 * \brief Determines the color for a specific marker based on its ArUco marker ID.
 * \param marker_id ArUco marker ID
 * \return color value between 0-2 corresponding to the specific marker's hex card
 */
int GameLogic::determine_marker_color(const int marker_id)
{
    // full_color card_type: 0, 1, 2
    // half_color card_type: 3, 4, 5
    // triple_color card_type: 6, 7, 8
    int card_type = (marker_id / 6) % 9;

    switch (card_type / 3) {
        case 0: {
                //returns (full_color[id] + 0/1/2) depending on which of the three full color cards we have
                //full_color is always 0 because every Marker in that hexagon has same color
                return (card_type % 3);
        }
        case 1: {
                //returns (half_color[id] + 0/1/2) depending on which of the three full color cards we have
                return (half_color[marker_id%6] + (card_type % 3));
        }
        case 2: {
                //returns (triple_color[id] + 0/1/2) depending on which of the three full color cards we have
                return (triple_color[marker_id%6] + (card_type % 3));
        }
        default:
            return -1;
    }
}

template <typename MapType, typename t>
void GameLogic::saveValue(MapType& storage, int key, t value) {
    storage[key] = value;
}

template <typename MapType, typename t>
t GameLogic::getValue(const MapType& storage, int key) {
    auto it = storage.find(key);
    if (it != storage.end()) {
        return it->second;
    }
    return false;
}

int GameLogic::calc_single_multiplier(const bool boolList[], int card_type)
{
    switch (card_type / 3)
    {
    case 0: {
            if(boolList[0] && boolList[1] && boolList[2] && boolList[3] && boolList[4] && boolList[5])
                return 6;
            return 0;
    }
    case 1: {
            const bool first_half = boolList[0] && boolList[1] && boolList[2];
            const bool second_half = boolList[3] && boolList[4] && boolList[5];

            if (first_half && second_half)
                return 6;
            else if (first_half || second_half) 
                return 3;
            return 0;
    }
    case 2: {
            const bool first_third = boolList[0] && boolList[1];
            const bool second_third = boolList[2] && boolList[3];
            const bool third_third = boolList[4] && boolList[5];

            if (first_third && second_third && third_third)
                return 6;
            else if ((first_third && second_third) || (second_third && third_third) || (first_third && third_third))
                return 4;
            else if (first_third || second_third || third_third)
                return 2;
            return 0;
    }
    default:
        return -1;
    }
}

int GameLogic::calculate_multipliers(int max_hex_id)
{
    bool marker_bools[6] = {false, false, false, false, false, false};
    int card_id = -1;
    int marker_id = -1;
    
    for(int cur_hex = 0; cur_hex <= max_hex_id; cur_hex++)
    {
        card_id = (cur_hex * 6);
        fill_n(marker_bools, 6, false);
        
        for(int cur_marker = 0; cur_marker < 6; cur_marker++)
        {
            marker_id = card_id + cur_marker;
            if(getValue<unordered_map<int, bool>, bool>(marker_id_matches, marker_id))
                marker_bools[cur_marker] = true;
        }

        saveValue(marker_multipliers, cur_hex, calc_single_multiplier(marker_bools, (cur_hex % 9)));
    }
}

float GameLogic::calculate_game_score(const vector<tuple<marker, marker>>& matches)
{
    return 0;
}


