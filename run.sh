# Check if a seed argument is provided
if [ -n "$1" ]; then
    seed="--seed=$1"
else
    seed=""
fi

# Run the command with the optional seed argument
luxai-s3 main.py main.py --output=replay.html $seed