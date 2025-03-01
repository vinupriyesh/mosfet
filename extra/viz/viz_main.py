import argparse
from time import sleep
import pygame
import struct
import sys
import json
from http.server import BaseHTTPRequestHandler, HTTPServer
from threading import Thread
from queue import Queue
from visualizer import Visualizer
from game_state import GameState

# Global variables for game state, visualizer, response queue, and HTTP server
game_state = None
visualizer = None
response_queue = Queue()
httpd = None
replay_handler = None

class RequestHandler(BaseHTTPRequestHandler):
    def _set_headers(self):
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()

    def do_POST(self):
        global game_state, visualizer, response_queue

        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        data = json.loads(post_data)

        game_state.update_state(data)
        visualizer.update_display()

        # Wait for the user's input before sending a response
        # response = response_queue.get()
        sleep(0.25)
        response = 'success'

        self._set_headers()
        self.wfile.write(json.dumps({'status': response}).encode('utf-8'))


class ReplayHandler:

    def __init__(self, file):
        self.file = file
        self.data = self.load_data()["data"]
        self.tracker_data = self.load_opponent_tracker_data()
        self.current_frame = 0

    def load_data(self):
        with open(self.file, 'r') as f:
            data = json.load(f)
        return data
    
    def load_opponent_tracker_data(self):
        if "custom_replay_0.json" in self.file:
            filename = self.file.replace("custom_replay_0.json", "opponent_tracker_0.bin")
        else:
            filename = self.file.replace("custom_replay_1.json", "opponent_tracker_1.bin")

        data = []
        with open(filename, "rb") as file:
            while True:
                # Read the index (first dimension)
                index_data = file.read(8)
                if not index_data:  # End of file
                    break
                index = struct.unpack("Q", index_data)[0]

                # Read dimensions of the 3D array
                dim1 = struct.unpack("Q", file.read(8))[0]
                matrix = []
                for _ in range(dim1):
                    dim2 = struct.unpack("Q", file.read(8))[0]
                    sub_matrix = []
                    for _ in range(dim2):
                        dim3 = struct.unpack("Q", file.read(8))[0]
                        row = struct.unpack(f"{dim3}d", file.read(dim3 * 8))
                        sub_matrix.append(row)
                    matrix.append(sub_matrix)
                data.append(matrix)
        return data

    def forward(self, increment=1):
        if self.current_frame < len(self.data) - increment - 1:            
            self.current_frame += increment
            self.update_state_and_display()
    
    def backward(self, increment=1):
        if self.current_frame - increment >= 0:
            self.current_frame -= increment
            self.update_state_and_display()
    
    def goto_start(self):
        self.current_frame = 0
        self.update_state_and_display()
    
    def goto_end(self):
        self.current_frame = len(self.data) - 2
        self.update_state_and_display()
        
    
    def update_state_and_display(self):
        game_state.update_state(self.data[self.current_frame], self.tracker_data[self.current_frame], visualizer.shuttle_toggle_state)
        visualizer.update_display()

def run_server(port):
    global httpd
    server_address = ('', port)
    httpd = HTTPServer(server_address, RequestHandler)
    print(f'Starting server at port {port}...')
    httpd.serve_forever()
        
def live_mode(port: int):    

    # Start HTTP server in a separate thread
    server_thread = Thread(target=run_server, args=(port,))
    server_thread.start()

def replay_mode(file: str, visualizer: Visualizer):
    global replay_handler

    replay_handler = ReplayHandler(file)
    visualizer.register_replay_handler(replay_handler)
    replay_handler.update_state_and_display()
    
def main():
    parser = argparse.ArgumentParser(description='Vizualization CLI')
    
    subparsers = parser.add_subparsers(dest='mode', required=True)
    
    live_parser = subparsers.add_parser('live', help='Run in live mode')
    live_parser.add_argument('port', type=int, help='Port number for live mode')
    
    replay_parser = subparsers.add_parser('replay', help='Run in replay mode')
    replay_parser.add_argument('file', type=str, help='JSON file for replay mode')
    
    args = parser.parse_args()

    pygame.init()        

    global game_state, visualizer, response_queue
    game_state = GameState((24, 24))
    visualizer = Visualizer(game_state, response_queue)

    if args.mode == 'live':
        live_mode(args.port)
    elif args.mode == 'replay':
        replay_mode(args.file, visualizer)

    # Start the Pygame loop in the main thread
    visualizer.run()


if __name__ == "__main__":
    main()
