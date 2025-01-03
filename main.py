import json
from subprocess import Popen, PIPE
from threading import Thread
from queue import Queue, Empty
from collections import defaultdict
import atexit
import os
import sys
from argparse import Namespace
import random

agent_processes = defaultdict(lambda: None)
t = None
q_stderr = None
q_stdout = None
N = 16

verbose = False

def generate_no_action_string(N):
    actions = [[0, 0, 0] for _ in range(N)]
    return {"action": actions}    

def generate_random_action_string(N):
    actions = [[random.randint(0, 4), 0, 0] for _ in range(N)]
    return {"action": actions}    

def cleanup_process():
    global agent_processes
    for agent_key in agent_processes:
        proc = agent_processes[agent_key]
        if proc is not None:
            proc.kill()

def enqueue_output(out, queue):
    for line in iter(out.readline, b''):
        queue.put(line)
    out.close()

def agent(observation, configuration):    
    global agent_processes, t, q_stderr, q_stdout, N
    try:        
        if configuration is not None and "max_units" in configuration:
            N = configuration["max_units"]
            print("Set N to {}".format(N), file=sys.stderr)       
                 
        agent_process = agent_processes[observation.player]
        if agent_process is None:
            if "__raw_path__" in configuration:
                cwd = os.path.dirname(configuration["__raw_path__"])
            else:
                cwd = os.path.dirname(__file__)
            os.chmod(os.path.join(cwd, "mosfet"),0o755)
            command = [os.path.join(cwd, "mosfet")]
            if verbose:
                command.append("config-test.properties")
            agent_process = Popen(command, stdin=PIPE, stdout=PIPE, stderr=PIPE, cwd=cwd)
            agent_processes[observation.player] = agent_process
            atexit.register(cleanup_process)

            q_stderr = Queue()
            t = Thread(target=enqueue_output, args=(agent_process.stderr, q_stderr))
            t.daemon = True
            t.start()
        data = json.dumps(dict(obs=json.loads(observation.obs), step=observation.step, remainingOverageTime=observation.remainingOverageTime, player=observation.player, info=configuration))
        agent_process.stdin.write(f"{data}\n".encode())
        agent_process.stdin.flush()

        agent_res = (agent_process.stdout.readline()).decode()
        while True:
            try:
                line = q_stderr.get_nowait()
            except Empty:
                break
            else:
                print(line.decode(), file=sys.stderr, end='')
        if agent_res == "":
            raise Exception("Agent response is empty")
        return json.loads(agent_res)
    except Exception as e:
        print("ERR:{}".format(repr(e)), file=sys.stderr)
        
        # Trying to get the exit code
        try:            
            agent_process = agent_processes[observation.player]
            if agent_process is not None:
                agent_process.wait(1)
                exit_code = agent_process.returncode
                print("ERR:Exit code:{}".format(exit_code), file=sys.stderr)
            
        except Exception as e2:
            print("ERR2:{}".format(repr(e2)), file=sys.stderr)

        if verbose:
            raise e        
        return generate_random_action_string(N)


# Run the agent locally.  Kaggle will NOT call this, it will directly call the agent function.
if __name__ == "__main__":
    
    def read_input():
        """
        Reads input from stdin
        """
        try:
            return input()
        except EOFError as eof:
            raise SystemExit(eof)
    step = 0
    player_id = 0
    configurations = None
    i = 0
    verbose = True
    while True:
        inputs = read_input()
        obs = json.loads(inputs)
        
        observation = Namespace(**dict(step=obs["step"], obs=json.dumps(obs["obs"]), remainingOverageTime=obs["remainingOverageTime"], player=obs["player"], info=obs["info"]))
        if i == 0:
            configurations = obs["info"]["env_cfg"]
        i += 1
        actions = agent(observation, dict(env_cfg=configurations))
        # send actions to engine
        print(json.dumps(actions))