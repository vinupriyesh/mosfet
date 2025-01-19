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

- Move Pathing to gameMap reference instead of a pointer

## To dive deep

- seed 946202263, 31903 - This seed identifies wrong vantage points.  Investigate this!
- [Fixed] Seed 8466, 22638, 3313 - Points to in negative