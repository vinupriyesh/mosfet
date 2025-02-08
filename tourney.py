import subprocess
import random
import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed

def run_game(agent1, agent2, seed, gameId):
    process = subprocess.Popen(
        ["luxai-s3", agent1, agent2, f"--seed={seed}"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    print(f"Game: {gameId}, Seed: {seed}")

    while True:
        error = process.stderr.readline()
        if error and not "An NVIDIA GPU may be present on this machine" in error:
            print(f"Seed: {seed}, Err: {error.strip()}")
        
        # If the process has finished, break the loop
        if process.poll() is not None:
            break

    # Read the remaining output from stdout
    stdout_output = process.stdout.read()
    if stdout_output:
        print(stdout_output.strip())

def main():
    parser = argparse.ArgumentParser(description="Run Lux AI game multiple times.")
    parser.add_argument("agent1", type=str, help="First agent file.")
    parser.add_argument("agent2", type=str, help="Second agent file.")
    parser.add_argument("num_runs", type=int, help="Number of total runs.")
    parser.add_argument("threads", type=int, help="Number of parallel threads.")
    
    args = parser.parse_args()

    print(f"Args - {args}")

    total_runs = args.num_runs
    max_workers = args.threads
    futures = []
    completed_runs = 0

    idx = 0

    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        while completed_runs < total_runs:
            # Submit new tasks if there are fewer than max_workers running
            while len(futures) < max_workers and completed_runs < total_runs:
                seed = random.randint(1, 999_999_999)
                idx+=1
                futures.append(executor.submit(run_game, args.agent1, args.agent2, seed, idx))
                completed_runs += 1
            
            # Process completed futures
            for future in as_completed(futures):
                futures.remove(future)
                future.result()  # Ensure each future has completed

if __name__ == "__main__":
    main()
