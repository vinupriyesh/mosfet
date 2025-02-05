seed=""
debug_mode=0
vs="main.py"

# Parse arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --seed=*) seed="$1"; shift ;;
        -s) seed="--seed=$2"; shift; shift ;;
        --debug|-d) debug_mode=1; shift ;;
        --vs|-vs) vs="$2"; shift; shift ;; 
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
done

# Generate a random seed if not provided
if [ -z "$seed" ]; then
    random_seed=$((RANDOM))
    seed="--seed=$random_seed"
    echo "Generated random seed: $random_seed"
fi

# Create DEBUG file if in debug mode
if [ $debug_mode -eq 1 ]; then
    touch DEBUG
fi

# Run the command with the optional seed argument
luxai-s3 main.py $vs --output=replay.html $seed

# Remove DEBUG file if it was created
if [ $debug_mode -eq 1 ]; then
    rm DEBUG
fi

# Run the chart
python3 plot.py