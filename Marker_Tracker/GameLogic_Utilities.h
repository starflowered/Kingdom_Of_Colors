#pragma once
#include <unordered_map>
#include <string>
#include <array>
#include "Structs.h"

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
	static int determine_card_type(const int marker_id);
	static int determine_marker_color(int marker_id);

	static std::string get_name_of_color_by_index(int idx);
	static int get_number_of_colors();
	static const std::unordered_map<int, std::array<bool, 6>> get_current_matched_markers_per_tiles() ;
	static void set_current_matched_markers_per_tiles(const std::unordered_map<int, std::array<bool, 6>> current_matches);
	static color determine_marker_color_values(const int marker_id);
private:
	static std::unordered_map<int, std::array<bool, 6>> current_matched_markers_per_tiles;

};

