#include "GameLogic_Utilities.h"

const static std::unordered_map<int, std::string> color_names = { {0, "blue"}, { 1, "yellow" }, { 2, "red" } };
std::unordered_map<int, std::array<bool, 6>> GameLogic_Utilities::current_matched_markers_per_tiles;
const static std::unordered_map<int, color> color_rgbas = { {0, color(0.24f, 0.28f, 0.42f, 1.0f)}, {1, color(0.87f, 0.8f, 0.43f, 1.0f)},
    {2, color(0.87f, 0.8f, 0.43f, 1.0f)} };


/**
 * \brief Determines the color for a specific marker based on its ArUco marker ID.
 * \param marker_id ArUco marker ID
 * \return color value between 0-2 corresponding to the specific marker's hex card
 */
int GameLogic_Utilities::determine_marker_color(const int marker_id)
{
    int card_type = (marker_id / 6) % 9;
    switch (card_type / 3) {
    case CARD_TYPE_ONE_COLOR: {
        //returns (full_color[id] + 0/1/2) depending on which of the three full color cards we have
        //full_color is always 0 because every Marker in that hexagon has same color
        return (card_type % 3);
    }
    case CARD_TYPE_HALF_COLOR: {
        //returns (half_color[id] + 0/1/2) depending on which of the three full color cards we have
        return (half_color[marker_id % 6] + (card_type % 3))%3;
    }
    case  CARD_TYPE_TRIPLE_COLOR: {
        //returns (triple_color[id] + 0/1/2) depending on which of the three full color cards we have
        return (triple_color[marker_id % 6] + (card_type % 3)) %3;
    }
    default:
        return -1;
    }
}

/**
 * \brief Determines the color values for a specific marker based on its ArUco marker ID.
 * \param marker_id ArUco marker ID
 * \return color value between 0-2 corresponding to the specific marker's hex card
 */
color GameLogic_Utilities::determine_marker_color_values(const int marker_id)
{
    int color_idx = GameLogic_Utilities::determine_marker_color(marker_id);
    if (color_idx < 0)
        return color(0, 0, 0, 0);
    else
        return color_rgbas.at(color_idx);

}


// full_color card_type: 0, 1, 2
// half_color card_type: 3, 4, 5
// triple_color card_type: 6, 7, 8
// card_type = hex_number % 9
int GameLogic_Utilities::determine_card_type(int hex_id)
{
    return (hex_id % 9) / 3;
}

// alternative (really ugly) function, since for some odd reason the getValue function didnt work
std::string GameLogic_Utilities::get_name_of_color_by_index(int idx)
{
    if (color_names.contains(idx))
        return color_names.at(idx);
    return "no color name found";
   
}

int GameLogic_Utilities::get_number_of_colors()
{
    return color_names.size();
}

const std::unordered_map<int, std::array<bool, 6>> GameLogic_Utilities::get_current_matched_markers_per_tiles()
{
    return current_matched_markers_per_tiles;
}
void GameLogic_Utilities::set_current_matched_markers_per_tiles(const std::unordered_map<int, std::array<bool, 6>> current_matches)
{
    current_matched_markers_per_tiles = current_matches;
}



