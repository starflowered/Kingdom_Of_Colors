#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include "GameLogic_Utilities.h"
#include "Missions.h"

using namespace std;


// Forward declaration of the marker struct
struct marker;

class GameLogic
{
private:


    unordered_map<int, bool> marker_id_matches;
    unordered_map<int, int> marker_multipliers;
    unordered_map<int, int> hex_tile_scores;
    Missions missions;


    static int calc_single_multiplier(const bool boolList[], int card_type);

    template <class MapType, class t>
    static void saveValue(MapType& storage, int key, t value);

    template <class MapType, class t>
    static t getValue(const MapType& storage, int key, t defaultValue);


public:
    std::array<std::tuple<std::string, std::function<int(int, std::unordered_map<int, std::array<bool, 6>>)>, int>, 3> get_current_missions();
    void reset_maps();
    void calculate_multipliers(int max_hex_id);
    int calculate_game_score(const vector<tuple<marker, marker>>& matches);
    
    
};
