#pragma once
#include <unordered_map>
#include <string>

#define CARD_TYPE_ONE_COLOR 0
#define CARD_TYPE_HALF_COLOR 1
#define CARD_TYPE_TRIPLE_COLOR 2
#define MULTIPLIER_ONE_COLOR 12
#define MULTIPLIER_HALF_CARD 4
#define MULTIPLIER_TRIPLE_CARD 2
#define DEFAULT_MAX_HEX_ID 10
class GameLogic_Utilities
{
public: 

	static constexpr int half_color[6] = { 0, 0, 0, 1, 1, 1 };
	static constexpr int triple_color[6] = { 0, 0, 1, 1, 2, 2 };
	static const std::unordered_map<int, std::string> color_names;
	static int determine_card_type(const int marker_id);
	static int determine_marker_color(int marker_id);

	static std::string get_name_of_color_by_index(int idx);
	static int get_number_of_colors();

	template <class MapType, class t>
	static void saveValue(MapType& storage, int key, t value);

	template <class MapType, class t>
	static t getValue(const MapType& storage, int key, t defaultValue);

};

