import subprocess
import sys
import traceback

def read_input():
    """
    Reads input from stdin
    """
    try:
        return input()
    except EOFError as eof:
        raise SystemExit(eof)
    
def main():
    print("Staring the main.py", file=sys.stderr)
    # Start the C++ program 
    process = subprocess.Popen(['./build/mosfet'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    while True:
        # print("Iterating", file=sys.stderr)
        inputs = read_input()
        # print("Sending input {}".format(inputs), file=sys.stderr)
        process.stdin.write(inputs + "\n")
        process.stdin.flush()
        # print("Flushed", file=sys.stderr)

        output = process.stdout.readline()
        process.stdout.flush()
        # print("Received out", file=sys.stderr)
        print(output)

        error = process.stderr.readline()
        process.stderr.flush()
        # print("Received error", file=sys.stderr)
        if error:
            print(error, file=sys.stderr)

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print("Caught unexpected exception from the code", file=sys.stderr)
        traceback.print_exc()