#include "Leaderboard.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstdio>

Leaderboard::Leaderboard() {
}

void Leaderboard::load() {
    entries.clear();
    
    std::ifstream inFile(getFilename());
    
    if (!inFile.is_open()) {
        return;  // No leaderboard file yet
    }
    
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string content = buffer.str();
    inFile.close();
    
    // Simple JSON parsing
    size_t pos = 0;
    while ((pos = content.find('{', pos)) != std::string::npos) {
        size_t endPos = content.find('}', pos);
        if (endPos == std::string::npos) break;
        
        std::string entry = content.substr(pos, endPos - pos + 1);
        LeaderboardEntry le;
        
        // Parse name
        size_t nameStart = entry.find("\"name\"");
        if (nameStart != std::string::npos) {
            nameStart = entry.find(':', nameStart);
            nameStart = entry.find('"', nameStart + 1);
            size_t nameEnd = entry.find('"', nameStart + 1);
            if (nameStart != std::string::npos && nameEnd != std::string::npos) {
                le.name = entry.substr(nameStart + 1, nameEnd - nameStart - 1);
            }
        }
        
        // Parse time
        size_t timeStart = entry.find("\"time\"");
        if (timeStart != std::string::npos) {
            timeStart = entry.find(':', timeStart);
            if (timeStart != std::string::npos) {
                le.time = std::stof(entry.substr(timeStart + 1));
            }
        }
        
        // Parse deaths
        size_t deathsStart = entry.find("\"deaths\"");
        if (deathsStart != std::string::npos) {
            deathsStart = entry.find(':', deathsStart);
            if (deathsStart != std::string::npos) {
                le.deaths = std::stoi(entry.substr(deathsStart + 1));
            }
        }
        
        if (!le.name.empty()) {
            entries.push_back(le);
        }
        
        pos = endPos + 1;
    }
    
    // Sort by time (ascending)
    std::sort(entries.begin(), entries.end(),
              [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
                  return a.time < b.time;
              });
    
    // Limit to first 100 entries
    if (entries.size() > 100) {
        entries.resize(100);
    }
}

void Leaderboard::save(const std::string& playerName, float time, int deaths) {
    // Use a default name if empty
    std::string name = playerName.empty() ? "Anonymous" : playerName;
    
    // Read existing leaderboard
    std::ifstream inFile(getFilename());
    std::string content = "";
    
    if (inFile.is_open()) {
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        content = buffer.str();
        inFile.close();
    }
    
    // Simple JSON writing (manual to avoid dependencies)
    std::ofstream outFile(getFilename());
    if (!outFile.is_open()) {
        std::cerr << "Failed to open leaderboard file for writing!" << std::endl;
        return;
    }
    
    // Parse existing entries or start fresh
    std::vector<std::string> existingEntries;
    
    if (!content.empty() && content.find('[') != std::string::npos) {
        // Extract existing entries (simple parsing)
        size_t start = content.find('[');
        size_t end = content.rfind(']');
        if (start != std::string::npos && end != std::string::npos) {
            std::string arrayContent = content.substr(start + 1, end - start - 1);
            
            // Find each object { ... }
            size_t objStart = 0;
            while ((objStart = arrayContent.find('{', objStart)) != std::string::npos) {
                size_t objEnd = arrayContent.find('}', objStart);
                if (objEnd != std::string::npos) {
                    existingEntries.push_back(arrayContent.substr(objStart, objEnd - objStart + 1));
                    objStart = objEnd + 1;
                } else {
                    break;
                }
            }
        }
    }
    
    // Add new entry
    char newEntry[256];
    snprintf(newEntry, sizeof(newEntry), 
             "    {\n"
             "        \"name\": \"%s\",\n"
             "        \"time\": %.3f,\n"
             "        \"deaths\": %d\n"
             "    }",
             name.c_str(), time, deaths);
    existingEntries.push_back(newEntry);
    
    // Write JSON array
    outFile << "[\n";
    for (size_t i = 0; i < existingEntries.size(); i++) {
        outFile << existingEntries[i];
        if (i < existingEntries.size() - 1) {
            outFile << ",";
        }
        outFile << "\n";
    }
    outFile << "]\n";
    
    outFile.close();
    
    std::cout << "Leaderboard saved: " << name << " - " << time << "s, " << deaths << " deaths" << std::endl;
}
