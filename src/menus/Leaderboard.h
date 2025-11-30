#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <string>
#include <vector>

// Single leaderboard entry
struct LeaderboardEntry {
    std::string name;
    float time;
    int deaths;
};

// Leaderboard data management
class Leaderboard {
public:
    Leaderboard();
    
    // Load/save operations
    void load();
    void save(const std::string& playerName, float time, int deaths);
    
    // Access entries
    const std::vector<LeaderboardEntry>& getEntries() const { return entries; }
    size_t size() const { return entries.size(); }
    
    // File path
    static const char* getFilename() { return "leaderboard.json"; }
    
private:
    std::vector<LeaderboardEntry> entries;
};

#endif // LEADERBOARD_H
