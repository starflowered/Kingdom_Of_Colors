#include "GameLogic.h"

/**
 * \brief resets unordered maps for every game logic calculation
 */
void GameLogic::reset_maps()
{
    marker_multipliers.clear();
    hex_tile_scores.clear();
    marker_id_matches.clear();
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
    // full_color card_type: 0, 1, 2
    // half_color card_type: 3, 4, 5
    // triple_color card_type: 6, 7, 8
    switch (card_type / 3)
    {
    case CARD_TYPE_ONE_COLOR: {
            // full color card
            if(boolList[0] && boolList[1] && boolList[2] && boolList[3] && boolList[4] && boolList[5])
                return MULTIPLIER_ONE_COLOR;
            return 1;
    }
    case CARD_TYPE_HALF_COLOR: {
            // half color card
            const bool first_half = boolList[0] && boolList[1] && boolList[2];
            const bool second_half = boolList[3] && boolList[4] && boolList[5];

            if (first_half && second_half)
                return MULTIPLIER_HALF_CARD*2;
            else if (first_half || second_half) 
                return MULTIPLIER_HALF_CARD;
            return 1;
    }
    case CARD_TYPE_TRIPLE_COLOR: {
            // triple color card
            const bool first_third = boolList[0] && boolList[1];
            const bool second_third = boolList[2] && boolList[3];
            const bool third_third = boolList[4] && boolList[5];

            if (first_third && second_third && third_third)
                return MULTIPLIER_TRIPLE_CARD*3;
            else if ((first_third && second_third) || (second_third && third_third) || (first_third && third_third))
                return MULTIPLIER_TRIPLE_CARD*2;
            else if (first_third || second_third || third_third)
                return MULTIPLIER_TRIPLE_CARD;
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
 
    for(int cur_hex = 0; cur_hex <= max_hex_id; cur_hex++)
    {
        std::array<bool, 6> defaultValue = { false,false,false,false,false,false };
        std::array<bool,6> marker_bools = getValue<unordered_map<int, std::array<bool, 6>>, std::array<bool, 6>>
            (GameLogic_Utilities::get_current_matched_markers_per_tiles(), cur_hex, defaultValue);
        bool c_style_array[6];
        std::copy(std::begin(marker_bools), std::end(marker_bools), std::begin(c_style_array));
        saveValue(marker_multipliers, cur_hex, calc_single_multiplier(c_style_array, (cur_hex % 9)));
    }
}

/**
 * \brief calculates Game Score based on matching marker list
 * \param matches list of matching markers
 * \return current Game Score
 */
int GameLogic::calculate_game_score(const vector<tuple<marker, marker>>& matches)
{
    reset_maps();
    int max_hex_id = process_matches_of_next_frame(matches);
    update_tile_matches_per_marker(max_hex_id);
    // calculate unordered map for all multipliers
    calculate_multipliers(max_hex_id);

    int GameScore = 0;

    //for each hex tile that has any points, get the relevant multiplier and add to GameScore
    for (const auto& pair : hex_tile_scores) {
        int key = pair.first;
        int score = pair.second;

        GameScore += (score * getValue<unordered_map<int, int>, int>(marker_multipliers, key, 1));
    }
    GameScore += missions.computeMissionScore();
    return GameScore;
}

/*
* \brief processes the matches that we get per frame, checks whether they are "actual matches" (same color)
* \param matches list of matching markers
* \return highest hexagon id that got a match
*/
int GameLogic::process_matches_of_next_frame(const vector<tuple<marker, marker>>& matches)
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

        if (id_1 / 6 != hex_1 || id_2 / 6 != hex_2)
        {
            std::cerr << "Issue between marker id and hexagon ids " << std::endl;
            std::cerr<< "Marker 1: " << id_1 << ", supposed hex: " << (id_1 / 6) << ", actual hex: " << hex_1 << std::endl;
            std::cerr << "Marker 2: " << id_2 << ", supposed hex: " << (id_2 / 6) << ", actual hex: " << hex_2 << std::endl;
        }
        

        color_1 = GameLogic_Utilities::determine_marker_color(id_1);
        color_2 = GameLogic_Utilities::determine_marker_color(id_2);

        // check if current match has a higher hex id than current max so we dont unnecessarily iterate in calculate multipliers later
        if (hex_1 > max_hex_id)
            max_hex_id = hex_1;
        if (hex_2 > max_hex_id)
            max_hex_id = hex_2;

        if (hex_1 == hex_2)
            continue;

        // Actual Match!
        if (color_1 == color_2)
        {
            // true (as in "has a match") for the specific AR Marker
            saveValue(marker_id_matches, id_1, true);
            saveValue(marker_id_matches, id_2, true);

            // get Current Value for one hex tile and add +1 for every additional match found in this tile
            saveValue(hex_tile_scores, hex_1, 1 + getValue<unordered_map<int, int>, int>(hex_tile_scores, hex_1, 0));
            saveValue(hex_tile_scores, hex_2, 1 + getValue<unordered_map<int, int>, int>(hex_tile_scores, hex_2, 0));
        }
    }
    return max_hex_id;
}

/**
 * \brief creates an unordered map <int, std::array<bool, 6>> that stores which markers are matched per hexagon
 * \param max_hex_id current maximal hexagon id on the game field
 *
 * */
void GameLogic::update_tile_matches_per_marker(int max_hex_id)
{
    std::array<bool,6> marker_bools = { false, false, false, false, false, false };
    int first_marker_of_card_id = -1;
    int marker_id = -1;
    std::unordered_map<int, std::array<bool, 6>> result;

    for (int cur_hex = 0; cur_hex <= max_hex_id; cur_hex++)
    {
        first_marker_of_card_id = (cur_hex * 6);
        marker_bools = { false,false,false,false,false,false };

        for (int cur_marker = 0; cur_marker < 6; cur_marker++)
        {
            marker_id = first_marker_of_card_id + cur_marker;
            if (getValue<unordered_map<int, bool>, bool>(marker_id_matches, marker_id, false))
                marker_bools[cur_marker] = true;
        }
        result[cur_hex] = marker_bools;
       
    }
    GameLogic_Utilities::set_current_matched_markers_per_tiles(result);
}

Missions& GameLogic::get_missions()
{
    return missions;
}