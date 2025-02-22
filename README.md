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

## Framework issues

### 01-framework-issue-points

-There is a framework bug.  Where the framework should have given 8 points, but gave only 7 points.   
    - Seed 12310
    - Step 173, player 0
    - From step 172 to 173, we should've got 8 points, but received only 7!

## Benchmarks

### GPU

```
start recording reset + for loop jax.step metrics
reset + for loop jax.step (5 trials)
AVG: 4035.529 steps/s, 63.055 parallel steps/s, 505.0 steps in 8.024s
STD: 173.418 steps/s, 2.710 parallel steps/s, 0.0 steps in 0.363s
    CPU mem: 831.195 MB, GPU mem: 0.000 MB
start recording reset + jax.lax.scan(jax.step) metrics
reset + jax.lax.scan(jax.step) (5 trials)
AVG: 74349.404 steps/s, 1161.709 parallel steps/s, 505.0 steps in 0.435s
STD: 1787.101 steps/s, 27.923 parallel steps/s, 0.0 steps in 0.010s
    CPU mem: 877.367 MB, GPU mem: 0.000 MB
start recording jit(reset + jax.lax.scan(jax.step)) metrics
jit(reset + jax.lax.scan(jax.step)) (5 trials)
AVG: 62159.862 steps/s, 971.248 parallel steps/s, 505.0 steps in 0.522s
STD: 4249.818 steps/s, 66.403 parallel steps/s, 0.0 steps in 0.035s
    CPU mem: 1066.133 MB, GPU mem: 0.000 MB
```


## Seeds

- 616838891 - Relics respawn in the same location
- 24959 - No vantage points in entire game.  2 Relics are there

- 931884472 - Late relic, bugged exploration
- 1784727651 - Short vision, long sap, king of the hill relic spawn
- 360140489 - Sensor range same as SAP range - 4