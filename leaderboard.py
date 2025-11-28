#!/usr/bin/env python3
"""
Leaderboard viewer for 3D Parkour Game
Reads leaderboard.json and displays top 100 (or fewer) entries
"""

import json
import os
import sys

def format_time(seconds):
    """Convert seconds to MM:SS.ms format"""
    minutes = int(seconds // 60)
    secs = int(seconds % 60)
    ms = int((seconds % 1) * 100)
    return f"{minutes:02d}:{secs:02d}.{ms:02d}"

def load_leaderboard(filepath):
    """Load leaderboard from JSON file"""
    if not os.path.exists(filepath):
        print(f"Error: Leaderboard file not found: {filepath}")
        return None
    
    try:
        with open(filepath, 'r') as f:
            data = json.load(f)
        return data
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON in leaderboard file: {e}")
        return None
    except Exception as e:
        print(f"Error reading leaderboard: {e}")
        return None

def display_leaderboard(entries, max_entries=100):
    """Display the leaderboard in a nice format"""
    if not entries:
        print("No entries in the leaderboard yet!")
        return
    
    # Sort by time (fastest first)
    sorted_entries = sorted(entries, key=lambda x: x.get('time', float('inf')))
    
    # Limit to max_entries
    display_entries = sorted_entries[:max_entries]
    
    # Calculate column widths
    rank_width = len(str(len(display_entries)))
    name_width = max(len(e.get('name', 'Anonymous')) for e in display_entries)
    name_width = max(name_width, 4)  # Minimum width for "Name"
    
    # Header
    print()
    print("=" * (rank_width + name_width + 30))
    print(f"{'🏆 LEADERBOARD 🏆':^{rank_width + name_width + 30}}")
    print("=" * (rank_width + name_width + 30))
    print()
    
    # Column headers
    print(f"{'#':<{rank_width}}  {'Name':<{name_width}}  {'Time':>10}  {'Deaths':>6}")
    print("-" * (rank_width + name_width + 30))
    
    # Entries
    for i, entry in enumerate(display_entries, 1):
        name = entry.get('name', 'Anonymous')
        time = entry.get('time', 0)
        deaths = entry.get('deaths', 0)
        
        # Add medal emoji for top 3
        if i == 1:
            medal = "🥇"
        elif i == 2:
            medal = "🥈"
        elif i == 3:
            medal = "🥉"
        else:
            medal = "  "
        
        time_str = format_time(time)
        print(f"{i:<{rank_width}}  {name:<{name_width}}  {time_str:>10}  {deaths:>6}  {medal}")
    
    print("-" * (rank_width + name_width + 30))
    print(f"Total entries: {len(entries)}")
    if len(entries) > max_entries:
        print(f"(Showing top {max_entries})")
    print()

def main():
    # Default path - check both build directory and current directory
    possible_paths = [
        "build/leaderboard.json",
        "leaderboard.json",
        os.path.join(os.path.dirname(__file__), "build", "leaderboard.json"),
        os.path.join(os.path.dirname(__file__), "leaderboard.json"),
    ]
    
    # Allow custom path as argument
    if len(sys.argv) > 1:
        filepath = sys.argv[1]
    else:
        # Find first existing file
        filepath = None
        for path in possible_paths:
            if os.path.exists(path):
                filepath = path
                break
        
        if not filepath:
            print("Leaderboard file not found!")
            print("Searched in:")
            for path in possible_paths:
                print(f"  - {path}")
            print("\nUsage: python leaderboard.py [path/to/leaderboard.json]")
            sys.exit(1)
    
    print(f"Loading leaderboard from: {filepath}")
    
    entries = load_leaderboard(filepath)
    if entries is not None:
        display_leaderboard(entries, max_entries=100)

if __name__ == "__main__":
    main()
