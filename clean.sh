#!/bin/bash

remove_file() {
    for file in $1; do
        if [ -f "$file" ]; then
            rm "$file"
            echo "Removed $file"
        fi
    done
}

files=(
    "*.tar.gz"
    "application_0.log"
    "application_1.log"
    "output/custom_replay_0.json"
    "output/custom_replay_1.json"
    "output/opponent_tracker_0.bin"
    "output/opponent_tracker_1.bin"
    "output/opponent_energy_0.bin"
    "output/opponent_energy_1.bin"
    "output/atleast_one_shuttle_0.bin"
    "output/atleast_one_shuttle_1.bin"
    "test.log"
    "metrics.csv"
    "replay.html"
    "dashboard.html"
    "mosfet"
    "DEBUG"
    "valgrind_report_player_0.txt"
    "valgrind_report_player_1.txt"
)

for file in "${files[@]}"; do
    remove_file "$file"
done
