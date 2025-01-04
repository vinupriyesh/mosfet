# Check if a seed argument is provided
if [ -n "$1" ]; then
    seed="--seed=$1"
else
    # Generate a random integer
    random_seed=$((RANDOM))
    seed="--seed=$random_seed"
    echo "Generated random seed: $random_seed"
fi

# Run the command with the optional seed argument
luxai-s3 main.py main.py --output=replay.html $seed