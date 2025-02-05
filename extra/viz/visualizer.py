import pygame
import sys
import random
import time

white = (255, 255, 255)
black = (0, 0, 0)
light_grey = (200, 200, 190)
blue = (0, 0, 255)
red = (255, 0, 0)
gold_with_transparency = (255, 215, 0, 128)
energy_green = (0, 136, 41)
energy_red = (136, 16, 0)

def colorize(base_color, energy_value):
    # Assuming energy_value is between 0 and 1 for positive energy and 0 and -1 for negative energy
    alpha = int(13 * abs(energy_value))  # Calculate alpha based on energy value
    return base_color + (alpha,)

class Visualizer:
    def __init__(self, game_state, response_queue):
        self.game_state = game_state
        self.response_queue = response_queue
        self.grid_width, self.grid_height = self.game_state.grid_size
        self.cell_size = 32
        self.screen_width = self.grid_width * self.cell_size
        self.screen_height = self.grid_height * self.cell_size
        self.button_rect = pygame.Rect(0, self.screen_height , 100, 40)
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
        self.frontier_tile_image = pygame.image.load('img/frontier.png')
        self.frontier_tile_image = pygame.transform.scale(self.frontier_tile_image, (self.cell_size, self.cell_size))
        self.vantage_point_image = pygame.transform.scale(self.vantage_point_image, (self.cell_size, self.cell_size))
        self.replay_handler = None
        self.running = True
        self.forward_key_held = False
        self.backward_key_held = False

        self.asteroid_images = {}
        self.last_update_time = 0
        self.update_interval = 0.1  # Interval in seconds

    def get_asteroid_image(self, x, y):
        key = f"{x}_{y}"
        if key not in self.asteroid_images:
            self.asteroid_images[key] = pygame.transform.rotate(self.asteroid_image, random.randint(0, 360))
        return self.asteroid_images[key]

    def register_replay_handler(self, replay_handler):
        self.replay_handler = replay_handler

    def update_display(self):
        self.screen.fill(white)
        self.draw_grid()
        self.draw_elements()
        self.draw_score()
        pygame.display.flip()

    def draw_grid(self):
        idx = 0
        for x in range(0, self.screen_width, self.cell_size):
            for y in range(0, self.screen_height, self.cell_size):
                rect = pygame.Rect(x, y, self.cell_size, self.cell_size)
                if len(self.game_state.energy) > 0:
                    energy_value = self.game_state.energy[idx]
                    if energy_value != 0:
                        color = colorize(energy_green if energy_value > 0 else energy_red, energy_value)
                        # Create a temporary surface with per-pixel alpha
                        temp_surface = pygame.Surface((self.cell_size, self.cell_size), pygame.SRCALPHA)
                        pygame.draw.rect(temp_surface, color, temp_surface.get_rect())
                        self.screen.blit(temp_surface, rect.topleft)
                pygame.draw.rect(self.screen, light_grey, rect, 1)
                idx += 1

    def draw_elements(self):
        for asteroid in self.game_state.asteroids:
            self.draw_asteroid(asteroid)
        for frontier in self.game_state.unexplored_frontier:
            self.draw_frontier(frontier)
        for shuttle in self.game_state.blue_shuttles:
            self.draw_shuttle(shuttle, blue)
        for shuttle in self.game_state.red_shuttles:
            self.draw_shuttle(shuttle, red)
        for nebula in self.game_state.nebula:
            self.draw_nebula(nebula)
        for vantage_point in self.game_state.vantage_points:
            self.draw_vantage_point(vantage_point)
        for relic in self.game_state.relics:
            self.draw_relic(relic)
        for halo_tile in self.game_state.halo_tiles:
            self.draw_halo(halo_tile, gold_with_transparency)    


    def draw_vantage_point(self, position):
        x, y = position
        rect = self.vantage_point_image.get_rect(center=(x * self.cell_size + self.cell_size // 2, y * self.cell_size + self.cell_size // 2))
        self.screen.blit(self.vantage_point_image, rect.topleft)
                         
    def draw_relic(self, position):
        x, y = position
        rect = self.relic_image.get_rect(center=(x * self.cell_size + self.cell_size // 2, y * self.cell_size + self.cell_size // 2))
        self.screen.blit(self.relic_image, rect.topleft)

    def draw_nebula(self, position):
        x, y = position
        rect = self.nebula_image.get_rect(center=(x * self.cell_size + self.cell_size // 2, y * self.cell_size + self.cell_size // 2))
        self.screen.blit(self.nebula_image, rect.topleft)

    def draw_frontier(self, position):
        x, y = position
        rect = self.frontier_tile_image.get_rect(center=(x * self.cell_size + self.cell_size // 2, y * self.cell_size + self.cell_size // 2))
        self.screen.blit(self.frontier_tile_image, rect.topleft)

    def draw_asteroid(self, position):
        x, y = position
        rotated_image = self.get_asteroid_image(x, y)
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

    def draw_score(self):
        text_surface = self.font.render(f'step: {self.game_state.step}/{self.game_state.match_step}, ' +
                                        f'points: {self.game_state.blue_points}/{self.game_state.red_points}, ' +
                                        f'wins: {self.game_state.blue_wins}/{self.game_state.red_wins}',
                                          True, black)
        self.screen.blit(text_surface, (self.button_rect.x + 5, self.button_rect.y + 5))        

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if self.button_rect.collidepoint(event.pos):
                    # Send a custom response and unblock the HTTP request
                    self.response_queue.put("Custom Response")
                else:
                    self.handle_click(event.pos)
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RIGHT:
                    self.forward_key_held = True
                elif event.key == pygame.K_LEFT:
                    self.backward_key_held = True
                elif event.key == pygame.K_PAGEUP:
                    if self.replay_handler:
                        if pygame.key.get_mods() & pygame.KMOD_SHIFT:
                            self.replay_handler.forward(100)
                        else:
                            self.replay_handler.forward(10)
                elif event.key == pygame.K_PAGEDOWN:
                    if self.replay_handler:
                        if pygame.key.get_mods() & pygame.KMOD_SHIFT:
                            self.replay_handler.backward(100)
                        else:
                            self.replay_handler.backward(10)
                elif event.key == pygame.K_HOME:
                    if self.replay_handler:
                        self.replay_handler.goto_start()
                elif event.key == pygame.K_END:
                    if self.replay_handler:
                        self.replay_handler.goto_end()
            elif event.type == pygame.KEYUP:
                if event.key == pygame.K_RIGHT:
                    self.forward_key_held = False
                elif event.key == pygame.K_LEFT:
                    self.backward_key_held = False            

    def update(self):
        current_time = time.time()
        if current_time - self.last_update_time >= self.update_interval:
            if self.forward_key_held and self.replay_handler:
                self.replay_handler.forward()
                self.last_update_time = current_time
            if self.backward_key_held and self.replay_handler:
                self.replay_handler.backward()
                self.last_update_time = current_time
        self.update_display()

    def run(self):
        while self.running:
            self.handle_events()
            self.update()                        
            pygame.time.wait(10) 

    def handle_click(self, position):
        x, y = position
        grid_x, grid_y = x // self.cell_size, y // self.cell_size
        print(f"Shuttle clicked at ({grid_x}, {grid_y})")

