import subprocess
import random
import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import re
import json

def parse_response(data: str):
    time_elapsed = None
    rewards_json = None
    # Extract Time Elapsed as a float
    time_elapsed_match = re.search(r"Time Elapsed:\s+([\d.]+)", data)
    if time_elapsed_match:
        time_elapsed = float(time_elapsed_match.group(1))
        # print("Time Elapsed:", time_elapsed)

    # Extract Rewards as a dictionary and convert to JSON
    rewards_match = re.search(r"Rewards:\s+({.*})", data)
    if rewards_match:
        rewards_str = rewards_match.group(1)
        # Replace numpy array representation with list for valid JSON
        rewards_str = rewards_str.replace("array", "").replace("(dtype=int32)", "").replace("(", "[").replace(")", "]").replace(", dtype=int32", "")
        rewards_dict = eval(rewards_str)  # Safely evaluate the string representation of the dictionary
        rewards_json = json.dumps(rewards_dict, indent=4)
        # print("Rewards JSON:", rewards_json)
    
    return time_elapsed, rewards_json


def run_game(agent1, agent2, seed, gameId, verbose):
    log_level = 0
    if verbose:
        log_level = 2
    process = subprocess.Popen(
        ["luxai-s3", agent1, agent2, f"--seed={seed}", f"--verbose={log_level}"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    # print(f"Game: {gameId}, Seed: {seed}")

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
        # print(stdout_output.strip())
        time_taken, resp = parse_response(stdout_output.strip())
        return time_taken, resp, seed

def main():
    parser = argparse.ArgumentParser(description="Run Lux AI game multiple times.")
    parser.add_argument("agent1", type=str, help="First agent file.")
    parser.add_argument("agent2", type=str, help="Second agent file.")
    parser.add_argument("-n", "--num_runs", type=int, default=1, help="Number of total runs. Default is 1.")
    parser.add_argument("-t", "--threads", type=int, default=1, help="Number of parallel threads. Default is 1.")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose mode.")    
    
    args = parser.parse_args()

    print(f"Args - {args}")

    total_runs = args.num_runs
    max_workers = args.threads
    futures = []
    completed_runs = 0

    idx = 0

    time_taken_samples = []
    reponse_samples = []
    seeds = []

    player1_wins = 0
    player2_wins = 0

    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        while completed_runs < total_runs:
            # Submit new tasks if there are fewer than max_workers running
            while len(futures) < max_workers and completed_runs < total_runs:
                seed = random.randint(1, 999_999_999)
                idx+=1
                futures.append(executor.submit(run_game, args.agent1, args.agent2, seed, idx, args.verbose))
                completed_runs += 1
            
            # Process completed futures
            for future in as_completed(futures):
                futures.remove(future)
                time_taken, resp_json, returning_seed = future.result()  # Ensure each future has completed
                time_taken_samples.append(time_taken)
                reponse_samples.append(resp_json)
                seeds.append(returning_seed)
                if resp_json:
                    rewards = json.loads(resp_json)                    
                    if rewards["player_0"] > rewards["player_1"]:
                        print(f"P1 wins seed {returning_seed}")
                        player1_wins += 1
                    elif rewards["player_1"] > rewards["player_0"]:
                        print(f"P2 wins seed {returning_seed}")
                        player2_wins += 1
                    print(f"  --- P1: {player1_wins}, P2: {player2_wins}")

if __name__ == "__main__":
    main()
