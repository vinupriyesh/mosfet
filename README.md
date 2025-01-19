# Mosfet

A C++ bot for Lux AI Season 3.  See shell scripts.

- Kaggle page - https://www.kaggle.com/competitions/lux-ai-season-3/overview
- API structure - https://github.com/Lux-AI-Challenge/Lux-Design-S3/blob/main/kits/README.md

## Debug strings

- ERR: C++ bot crashed and falling back to Python dummy
- Fatal: C++ bot crashed in C++ layer
- Problem:  Something unexpected happened

## Requirements

See requirements.txt

```
pip3 install --upgrade luxai-s3
```

## Submitting to Kaggle

`submit.sh` or
`tar -czvf submission.tar.gz *`

## To fix

- Why Shuttle needs CC?  Why agent role needs CC? Fix it by separating data and control logic.
- There should only be the following data objects
  - gameMap
  - gameEnvConfigs (global)
  - gameState (Can be inside gameMap)

## To dive deep

- Seed 8466, 22638 - Points to in negative