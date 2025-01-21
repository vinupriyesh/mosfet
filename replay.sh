#!/bin/bash

if [ -z "$1" ]; then
    REPLAY_FILE="../../custom_replay_0.json"
elif [ "$1" == "1" ]; then
    REPLAY_FILE="../../custom_replay_1.json"
elif [ "$1" == "0" ]; then
    REPLAY_FILE="../../custom_replay_0.json"
fi

cd extra/viz
python3 viz_main.py replay $REPLAY_FILE
