// ReSharper disable All
#include "GameLogic.h"

#include "MarkerDetectionUtilities.h"

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

/**
 * \brief saves value to specified map via key
 * \tparam MapType template map as unordered_map<key_type, value_type>
 * \tparam t value_type for value
 * \param storage unordered_map to save key and value in
 * \param key key to save value as
 * \param value value to save
 */
template <typename MapType, typename t>
void GameLogic::saveValue(MapType& storage, int key, t value) {
    storage[key] = value;
}

/**
 * \brief returns value from map via specified key; if value is not found returns defaultValue
 * \tparam MapType template map as unordered_map<key_type, value_type>
 * \tparam t value_type for value and return
 * \param storage unordered_map to check key
 * \param key key for specified value
 * \param defaultValue default return value in case key doesnt exist
 * \return either matching value for key or default value if not found
 */
template <typename MapType, typename t>
t GameLogic::getValue(const MapType& storage, int key, t defaultValue) {
    auto it = storage.find(key);
    if (it != storage.end()) {
        return it->second;
    }
    return defaultValue;
}

/**
 * \brief calculates a single multiplier score for one hex-tile
 * \param boolList list of 6 bools for each hex-tile edge; every edge with match is True
 * \param card_type id of (current hex-tile % 9) to determine if/which full/half/triple color card
 * \return multiplier as int
 */
int GameLogic::calc_single_multiplier(const bool boolList[], int card_type)
{
    switch (card_type / 3)
    {
    case 0: {
            // full color card
            if(boolList[0] && boolList[1] && boolList[2] && boolList[3] && boolList[4] && boolList[5])
                return 6;
            return 1;
    }
    case 1: {
            // half color card
            const bool first_half = boolList[0] && boolList[1] && boolList[2];
            const bool second_half = boolList[3] && boolList[4] && boolList[5];

            if (first_half && second_half)
                return 6;
            else if (first_half || second_half) 
                return 3;
            return 1;
    }
    case 2: {
            // triple color card
            const bool first_third = boolList[0] && boolList[1];
            const bool second_third = boolList[2] && boolList[3];
            const bool third_third = boolList[4] && boolList[5];

            if (first_third && second_third && third_third)
                return 6;
            else if ((first_third && second_third) || (second_third && third_third) || (first_third && third_third))
                return 4;
            else if (first_third || second_third || third_third)
                return 2;
            return 1;
    }
    default:
        return -1;
    }
}

/**
 * \brief Iterates through all hex-tiles, up to max_hex_id and saves all multipliers to marker_multipliers
 * \param max_hex_id max hex tile id up to which we iterate
 */
void GameLogic::calculate_multipliers(int max_hex_id)
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
            if(getValue<unordered_map<int, bool>, bool>(marker_id_matches, marker_id, false))
                marker_bools[cur_marker] = true;
        }

        saveValue(marker_multipliers, cur_hex, calc_single_multiplier(marker_bools, (cur_hex % 9)));
    }
}

/**
 * \brief calculates Game Score based on matching marker list
 * \param matches list of matching markers
 * \return current Game Score
 */
int GameLogic::calculate_game_score(const vector<tuple<marker, marker>>& matches)
{
    // variable declarations for loop overwriting
    marker mark_1, mark_2;
    int id_1, id_2;
    int hex_1, hex_2;
    
    int color_1, color_2;
    int max_hex_id = -1;
    
    for (auto match : matches)
    {
        mark_1 = get<0>(match);
        mark_2 = get<1>(match);

        id_1 = mark_1.marker_id;
        id_2 = mark_2.marker_id;

        hex_1 = mark_1.hexagon_id;
        hex_2 = mark_2.hexagon_id;
        
        color_1 = determine_marker_color(id_1);
        color_2 = determine_marker_color(id_2);

        // check if current match has a higher hex id than current max so we dont unnecessarily iterate in calculate multipliers later
        if(hex_1> max_hex_id)
            max_hex_id = hex_1;
        else if(hex_2 > max_hex_id)
            max_hex_id = hex_2;

        // Actual Match!
        if(color_1 == color_2)
        {
            // true (as in "has a match") for the specific AR Marker
            saveValue(marker_id_matches, id_1, true);
            saveValue(marker_id_matches, id_2, true);

            // get Current Value for one hex tile and add +1 for every additional match found in this tile
            saveValue(hex_tile_scores, hex_1, 1 + getValue<unordered_map<int, int>, int>(hex_tile_scores, hex_1, 0));
            saveValue(hex_tile_scores, hex_2, 1 + getValue<unordered_map<int, int>, int>(hex_tile_scores, hex_2, 0));
        }
    }

    // calculate unordered map for all multipliers
    calculate_multipliers(max_hex_id);

    int GameScore = 0;

    //for each hex tile that has any points, get the relevant multiplier and add to GameScore
    for (const auto& pair : hex_tile_scores) {
        int key = pair.first;
        int score = pair.second;

        GameScore += (score * getValue<unordered_map<int, int>, int>(marker_multipliers, key, 1));
    }
    
    return GameScore;
}


