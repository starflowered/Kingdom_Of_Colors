#pragma once
#include <vector>
#include <unordered_map>
#include "GameLogic_Utilities.h"
#include "Missions.h"

//it is kind of fucked we are including this, we should theoretically use pointers and forward declaration
#include "MarkerDetectionUtilities.h"

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



    void reset_maps();



    template <class MapType, class t>
    static void saveValue(MapType& storage, int key, t value);

    template <class MapType, class t>
    static t getValue(const MapType& storage, int key, t defaultValue);
    static int calc_single_multiplier(const bool boolList[], int card_type);
    void update_tile_matches_per_marker(int max_hex_id);
    void calculate_multipliers(int max_hex_id);
    int process_matches_of_next_frame(const vector<tuple<marker, marker>>& matches);
    
   
    
public:
    
    int calculate_game_score(const vector<tuple<marker, marker>>& matches);
    
    
};
