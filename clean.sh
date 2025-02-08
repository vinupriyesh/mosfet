#!/bin/bash

remove_file() {
    if [ -f "$1" ]; then
        rm "$1"
        echo "Removed $1"
    fi
}


files=(
    "*.tar.gz"
    "application.log"
    "custom_replay_0.json"
    "custom_replay_1.json"
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
