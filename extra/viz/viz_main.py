from time import sleep
import pygame
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

def run_server(port):
    global httpd
    server_address = ('', port)
    httpd = HTTPServer(server_address, RequestHandler)
    print(f'Starting server at port {port}...')
    httpd.serve_forever()

def draw_text(screen, text, position, font):
    text_surface = font.render(text, True, (0, 0, 0))
    screen.blit(text_surface, position)

def main():
    pygame.init()
    screen = pygame.display.set_mode((400, 200))
    pygame.display.set_caption('Enter Port Number')
    font = pygame.font.Font(None, 32)
    input_box = pygame.Rect(100, 50, 200, 32)
    color_inactive = pygame.Color('lightskyblue3')
    color_active = pygame.Color('dodgerblue2')
    color = color_inactive
    active = False
    text = ''
    done = False

    while not done:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN:
                if input_box.collidepoint(event.pos):
                    active = not active
                else:
                    active = False
                color = color_active if active else color_inactive
            if event.type == pygame.KEYDOWN:
                if active:
                    if event.key == pygame.K_RETURN:
                        port = int(text)
                        done = True
                    elif event.key == pygame.K_BACKSPACE:
                        text = text[:-1]
                    else:
                        text += event.unicode

        screen.fill((255, 255, 255))
        draw_text(screen, 'Port No:', (50, 50), font)
        txt_surface = font.render(text, True, color)
        width = max(200, txt_surface.get_width()+10)
        input_box.w = width
        screen.blit(txt_surface, (input_box.x+5, input_box.y+5))
        pygame.draw.rect(screen, color, input_box, 2)

        pygame.display.flip()

    global game_state, visualizer, response_queue
    game_state = GameState((24, 24))
    visualizer = Visualizer(game_state, response_queue)

    # Start HTTP server in a separate thread
    server_thread = Thread(target=run_server, args=(port,))
    server_thread.start()

    # Start the Pygame loop in the main thread
    visualizer.run()

if __name__ == "__main__":
    main()
