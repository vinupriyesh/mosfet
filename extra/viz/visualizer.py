import pygame
import sys
import random

white = (255, 255, 255)
black = (0, 0, 0)
blue = (0, 0, 255)
red = (255, 0, 0)
gold_with_transparency = (255, 215, 0, 128)  # RGBA format with alpha value for transparency

class Visualizer:
    def __init__(self, game_state, response_queue):
        self.game_state = game_state
        self.response_queue = response_queue
        self.grid_width, self.grid_height = self.game_state.grid_size
        self.cell_size = 32
        self.screen_width = self.grid_width * self.cell_size
        self.screen_height = self.grid_height * self.cell_size
        self.button_rect = pygame.Rect(20, self.screen_height + 10, 100, 40)
        pygame.init()
        self.screen = pygame.display.set_mode((self.screen_width, self.screen_height + 60), pygame.SRCALPHA)
        pygame.display.set_caption('LuxS3 Visualizer')
        self.font = pygame.font.Font(None, 32)
        self.asteroid_image = pygame.image.load('img/asteroid.png')
        self.asteroid_image = pygame.transform.scale(self.asteroid_image, (self.cell_size, self.cell_size))
        self.nebula_image = pygame.image.load('img/nebula.png')
        self.nebula_image = pygame.transform.scale(self.nebula_image, (self.cell_size, self.cell_size))
        self.relic_image = pygame.image.load('img/relic.png')
        self.relic_image = pygame.transform.scale(self.relic_image, (self.cell_size, self.cell_size))
        self.vantage_point_image = pygame.image.load('img/vantage_point.png')
        self.vantage_point_image = pygame.transform.scale(self.vantage_point_image, (self.cell_size, self.cell_size))

    def update_display(self):
        self.screen.fill(white)
        self.draw_grid()
        self.draw_elements()
        self.draw_button()
        pygame.display.flip()

    def draw_grid(self):
        for x in range(0, self.screen_width, self.cell_size):
            for y in range(0, self.screen_height, self.cell_size):
                rect = pygame.Rect(x, y, self.cell_size, self.cell_size)
                pygame.draw.rect(self.screen, black, rect, 1)

    def draw_elements(self):
        for asteroid in self.game_state.asteroids:
            self.draw_asteroid(asteroid)        
        for shuttle in self.game_state.blue_shuttles:
            self.draw_shuttle(shuttle, blue)
        for shuttle in self.game_state.red_shuttles:
            self.draw_shuttle(shuttle, red)
        for nebula in self.game_state.nebula:
            self.draw_nebula(nebula)
        for relic in self.game_state.relics:
            self.draw_relic(relic)
        for halo_tile in self.game_state.halo_tiles:
            self.draw_halo(halo_tile, gold_with_transparency)    


    def draw_relic(self, position):
        x, y = position
        rect = self.relic_image.get_rect(center=(x * self.cell_size + self.cell_size // 2, y * self.cell_size + self.cell_size // 2))
        self.screen.blit(self.relic_image, rect.topleft)

    def draw_nebula(self, position):
        x, y = position
        rect = self.nebula_image.get_rect(center=(x * self.cell_size + self.cell_size // 2, y * self.cell_size + self.cell_size // 2))
        self.screen.blit(self.nebula_image, rect.topleft)

    def draw_asteroid(self, position):
        x, y = position
        rotated_image = pygame.transform.rotate(self.asteroid_image, random.randint(0, 360))
        rect = rotated_image.get_rect(center=(x * self.cell_size + self.cell_size // 2, y * self.cell_size + self.cell_size // 2))
        self.screen.blit(rotated_image, rect.topleft)

    def draw_shuttle(self, position, color):
        x, y = position
        rect = pygame.Rect(x * self.cell_size, y * self.cell_size, self.cell_size, self.cell_size)
        pygame.draw.rect(self.screen, color, rect)
    

    def draw_halo(self, position, color):
        x, y = position
        center = (x * self.cell_size + self.cell_size // 2, y * self.cell_size + self.cell_size // 2)
        radius = self.cell_size // 2

        # Create a temporary surface with per-pixel alpha
        halo_surface = pygame.Surface((self.cell_size, self.cell_size), pygame.SRCALPHA)
        pygame.draw.circle(halo_surface, color, (radius, radius), radius)

        # Blit the halo surface onto the main screen
        self.screen.blit(halo_surface, (x * self.cell_size, y * self.cell_size))

    def draw_button(self):
        pygame.draw.rect(self.screen, (0, 0, 255), self.button_rect)
        text_surface = self.font.render('Respond', True, (255, 255, 255))
        self.screen.blit(text_surface, (self.button_rect.x + 10, self.button_rect.y + 5))

    def run(self):
        while True:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()
                elif event.type == pygame.MOUSEBUTTONDOWN:
                    if self.button_rect.collidepoint(event.pos):
                        # Send a custom response and unblock the HTTP request
                        self.response_queue.put("Custom Response")
                    else:
                        self.handle_click(event.pos)
            pygame.time.wait(10)  # Adding a small delay to prevent high CPU usage

    def handle_click(self, position):
        x, y = position
        grid_x, grid_y = x // self.cell_size, y // self.cell_size
        print(f"Shuttle clicked at ({grid_x}, {grid_y})")

if __name__ == "__main__":
    # Assuming the game state is initialized somewhere else as per the main.py
    from game_state import GameState
    game_state = GameState((24, 24))  # Example grid size
    response_queue = Queue()
    visualizer = Visualizer(game_state, response_queue)
    visualizer.run()
