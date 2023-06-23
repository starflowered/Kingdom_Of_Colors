#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>

using namespace std;


// Forward declaration of the marker struct
struct marker;

class GameLogic
{
private:
    static constexpr int half_color[6] = {0, 0, 0, 1, 1, 1};
    static constexpr int triple_color[6] = {0, 0, 1, 1, 2, 2};

    unordered_map<int, bool> marker_id_matches;
    unordered_map<int, int> marker_multipliers;

    template <class MapType, typename t>
    void saveValue(MapType& storage, int key, t value);
    
    template <class MapType, typename t>
    t getValue(const MapType& storage, int key);
    
public:
    // returns color 0-2 for marker determined by hexagon
    int determine_marker_color(const int marker_id);

    static int calc_single_multiplier(const bool boolList[], int card_type);

    int calculate_multipliers(int max_hex_id);
    float calculate_game_score(const vector<tuple<marker, marker>>& matches);
};
