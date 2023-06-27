#include "GameLogic_Utilities.h"


const std::unordered_map<int, std::string> GameLogic_Utilities::color_names = { {0, "blue"}, { 1, "green" }, { 2, "yellow" } };

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
        return (half_color[marker_id % 6] + (card_type % 3));
    }
    case  CARD_TYPE_TRIPLE_COLOR: {
        //returns (triple_color[id] + 0/1/2) depending on which of the three full color cards we have
        return (triple_color[marker_id % 6] + (card_type % 3));
    }
    default:
        return -1;
    }
}


// full_color card_type: 0, 1, 2
// half_color card_type: 3, 4, 5
// triple_color card_type: 6, 7, 8
// card_type = hex_number % 9
int GameLogic_Utilities::determine_card_type(int hex_id)
{
    return (hex_id % 9) / 3;
}


std::string GameLogic_Utilities::get_name_of_color_by_index(int idx)
{
    return getValue(color_names, idx, "color unnamed");
}
int GameLogic_Utilities::get_number_of_colors()
{
    return color_names.size();
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
void GameLogic_Utilities::saveValue(MapType& storage, int key, t value) {
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
t GameLogic_Utilities::getValue(const MapType& storage, int key, t defaultValue) {
    auto it = storage.find(key);
    if (it != storage.end()) {
        return it->second;
    }
    return defaultValue;
}

