import pygame
import sys
import random
import time

white = (255, 255, 255)
black = (0, 0, 0)
very_light_grey = (220, 220, 200)
light_grey = (200, 200, 190)
blue = (0, 0, 255)
light_blue = (100, 149, 255)
red = (255, 0, 0)
gold_with_transparency = (255, 215, 0, 128)
grey_with_transparency = (128, 128, 128, 110)
energy_green = (0, 136, 41)
energy_red = (166, 16, 0)

bottom_dialog_offset = 60
bottom_dialog_padding = 2


def draw_dotted_line(surface, color, start_pos, end_pos, width=4, dot_length=6, space_length=4):
    x1, y1 = start_pos
    x2, y2 = end_pos
    dx = x2 - x1
    dy = y2 - y1
    length = (dx ** 2 + dy ** 2) ** 0.5
    dx /= length
    dy /= length

    for i in range(0, int(length), dot_length + space_length):
        start_dot = (x1 + dx * i, y1 + dy * i)
        end_dot = (x1 + dx * (i + dot_length), y1 + dy * (i + dot_length))
        pygame.draw.line(surface, color, start_dot, end_dot, width)

def colorize(base_color, energy_value):
    # Assuming energy_value is between 0 and 1 for positive energy and 0 and -1 for negative energy
    alpha = int(15 * abs(energy_value))  # Calculate alpha based on energy value
    return base_color + (alpha,)

class Visualizer:
    def __init__(self, game_state, response_queue):
        self.game_state = game_state
        self.response_queue = response_queue
        self.grid_width, self.grid_height = self.game_state.grid_size
        self.cell_size = 32
        self.screen_width = self.grid_width * self.cell_size
        self.screen_height = self.grid_height * self.cell_size        
        self.bottom_dialog_rect = pygame.Rect(bottom_dialog_padding, self.screen_height + bottom_dialog_padding, self.screen_width - 2 * bottom_dialog_padding, bottom_dialog_offset)
        pygame.init()
        self.screen = pygame.display.set_mode((self.screen_width, self.screen_height + bottom_dialog_offset + bottom_dialog_padding), pygame.SRCALPHA)
        pygame.display.set_caption('LuxS3 Visualizer')
        self.mouse_pos_x = 0
        self.mouse_pos_y = 0
        self.selected_shuttle_id = -1
        self.selected_shuttle_team  = -1 # 0 -> Blue, 1 -> Red, -1 -> None
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
        self.update_interval = 0.2  # Interval in seconds

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
        self.draw_tool_bar()
        self.draw_score()
        pygame.display.flip()

    def get_energy(self, x: int, y: int) -> int:        
        if (len(self.game_state.energy)) == 0:
            return -1        
        return self.game_state.energy[x * self.grid_width + y]

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
                if len(self.game_state.vision) > 0:
                    vision_value = self.game_state.vision[idx]
                    if vision_value == 0:
                        temp_surface = pygame.Surface((self.cell_size, self.cell_size), pygame.SRCALPHA)
                        pygame.draw.rect(temp_surface, grey_with_transparency, temp_surface.get_rect())
                        self.screen.blit(temp_surface, rect.topleft)
                pygame.draw.rect(self.screen, light_grey, rect, 1)
                idx += 1

    def draw_elements(self):
        self.selected_shuttle_id = -1
        self.selected_shuttle_team = -1
        for asteroid in self.game_state.asteroids:
            self.draw_asteroid(asteroid)
        for frontier in self.game_state.unexplored_frontier:
            self.draw_frontier(frontier)
        for sid, shuttle in enumerate(self.game_state.blue_shuttles):
            self.draw_shuttle(sid, shuttle, blue)
        for sid, shuttle in enumerate(self.game_state.red_shuttles):
            self.draw_shuttle(sid, shuttle, red)
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

    def draw_shuttle(self, sid, position, color):
        if color == blue:
            team_blue = True            
            energy_value = self.game_state.blue_shuttles_energy[sid]            
        else:
            team_blue = False            
            energy_value = self.game_state.red_shuttles_energy[sid]            
        
        x, y = position        

        cell_size = 16 + int (energy_value * 16.0 / 400.0)
        offset = (self.cell_size - cell_size) / 2

        rect = pygame.Rect(x * self.cell_size + offset, y * self.cell_size + offset, cell_size, cell_size)
        pygame.draw.rect(self.screen, color, rect)

        if self.mouse_pos_x == x and self.mouse_pos_y == y:
            sap_radius_rect = pygame.Rect((x - self.game_state.unit_sap_range) * self.cell_size,
                                          (y - self.game_state.unit_sap_range) * self.cell_size,
                                          self.cell_size * 2 * self.game_state.unit_sap_range + self.cell_size,
                                          self.cell_size * 2 * self.game_state.unit_sap_range + self.cell_size)            
            pygame.draw.rect(self.screen, color, sap_radius_rect, 1)

            sensor_radius_rect = pygame.Rect((x - self.game_state.unit_sensor_range) * self.cell_size,
                                          (y - self.game_state.unit_sensor_range) * self.cell_size,
                                          self.cell_size * 2 * self.game_state.unit_sensor_range + self.cell_size,
                                          self.cell_size * 2 * self.game_state.unit_sensor_range + self.cell_size)
            pygame.draw.rect(self.screen, color, sensor_radius_rect, 3)

            
            self.selected_shuttle_id = sid

            if team_blue:
                self.selected_shuttle_team = 0
            else:
                self.selected_shuttle_team = 1
        
        if team_blue:
            action = self.game_state.blue_shuttles_actions[sid]            
            self.draw_move_action_arrow(x, y, offset, action, light_blue)
    
    def draw_move_action_arrow(self, x, y, offset, action, color):
        if action[0] == 0:
            return

        arrow_offset_neg = 16 - 2
        arrow_offset_pos = 16 + 1

        if action[0] == 1:
            # Move up
            sx = arrow_offset_neg
            sy = offset
            ex = arrow_offset_neg
            ey = -arrow_offset_neg
        elif action[0] == 2:
            # Move right
            sx = self.cell_size - offset
            sy = arrow_offset_neg
            ex = self.cell_size + arrow_offset_neg
            ey = arrow_offset_neg
        elif action[0] == 3:
            # Move down
            sx = self.cell_size - arrow_offset_pos
            sy = self.cell_size - offset
            ex = self.cell_size - arrow_offset_pos
            ey = self.cell_size + arrow_offset_pos
        elif action[0] == 4:
            # Move left            
            sx = 0 + offset
            sy = self.cell_size - arrow_offset_pos
            ex = -arrow_offset_pos
            ey = self.cell_size - arrow_offset_pos

        if action[0] != 5:
            pygame.draw.line(self.screen, color,
                            (x * self.cell_size + sx, y * self.cell_size + sy),
                            (x * self.cell_size + ex, y * self.cell_size + ey),
                            2)
        else:
            target_x = x + action[1]
            target_y = y + action[2]
            draw_dotted_line(self.screen, color, (x * self.cell_size + 16, y * self.cell_size + 16),
                              (target_x * self.cell_size + 16, target_y * self.cell_size + 16))

    def draw_halo(self, position, color):
        x, y = position
        center = (x * self.cell_size + self.cell_size // 2, y * self.cell_size + self.cell_size // 2)
        radius = self.cell_size // 2

        # Create a temporary surface with per-pixel alpha
        halo_surface = pygame.Surface((self.cell_size, self.cell_size), pygame.SRCALPHA)
        pygame.draw.circle(halo_surface, color, (radius, radius), radius)

        # Blit the halo surface onto the main screen
        self.screen.blit(halo_surface, (x * self.cell_size, y * self.cell_size))

    def draw_tool_bar(self):
        pygame.draw.rect(self.screen, very_light_grey, self.bottom_dialog_rect)        

    def draw_score(self):
        text_surface = self.font.render(f'step: {self.game_state.step}/{self.game_state.match_step}, ' +
                                        f'points: {self.game_state.blue_points}/{self.game_state.red_points}, ' +
                                        f'wins: {self.game_state.blue_wins}/{self.game_state.red_wins}',
                                          True, black)
        self.screen.blit(text_surface, (self.bottom_dialog_rect.x + 5, self.bottom_dialog_rect.y + 5))

        line_2_text = f'({self.mouse_pos_x}, {self.mouse_pos_y}), energy: {self.get_energy(self.mouse_pos_x, self.mouse_pos_y)}'
        if self.selected_shuttle_id != -1:
            if self.selected_shuttle_team == 0:
                energy_value = self.game_state.blue_shuttles_energy[self.selected_shuttle_id]
            else:
                energy_value = self.game_state.red_shuttles_energy[self.selected_shuttle_id]

            line_2_text += f' | unit: {self.selected_shuttle_id}, energy: {energy_value}'
        line_2 = self.font.render(line_2_text, True, black)
        self.screen.blit(line_2, (self.bottom_dialog_rect.x + 5, self.bottom_dialog_rect.y + 10 + text_surface.get_height()))

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if self.bottom_dialog_rect.collidepoint(event.pos):
                    # Send a custom response and unblock the HTTP request
                    self.response_queue.put("Custom Response")
                else:
                    self.handle_click(event.pos)
            elif event.type == pygame.MOUSEMOTION:
                self.handle_hover(event.pos)
            elif event.type == pygame.KEYDOWN:
               self.handle_key_down(event.key)
            elif event.type == pygame.KEYUP:
               self.handle_key_up(event.key)

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
    
    def handle_hover(self, position):
        x, y = position
        grid_x, grid_y = x // self.cell_size, y // self.cell_size
        if grid_x >= 0 and grid_x < self.grid_width and grid_x != self.mouse_pos_x:
            self.mouse_pos_x = grid_x
        if grid_y >= 0 and grid_y < self.grid_height and grid_y != self.mouse_pos_y:
            self.mouse_pos_y = grid_y

    def handle_click(self, position):
        x, y = position
        grid_x, grid_y = x // self.cell_size, y // self.cell_size
        print(f"Shuttle clicked at ({grid_x}, {grid_y})")
    
    def handle_key_down(self, key):
        if key == pygame.K_RIGHT:
            self.forward_key_held = True
        elif key == pygame.K_LEFT:
            self.backward_key_held = True
        elif key == pygame.K_PAGEUP:
            if self.replay_handler:
                if pygame.key.get_mods() & pygame.KMOD_SHIFT:
                    self.replay_handler.forward(100)
                else:
                    self.replay_handler.forward(10)
        elif key == pygame.K_PAGEDOWN:
            if self.replay_handler:
                if pygame.key.get_mods() & pygame.KMOD_SHIFT:
                    self.replay_handler.backward(100)
                else:
                    self.replay_handler.backward(10)
        elif key == pygame.K_HOME:
            if self.replay_handler:
                self.replay_handler.goto_start()
        elif key == pygame.K_END:
            if self.replay_handler:
                self.replay_handler.goto_end()
            
    def handle_key_up(self, key):
        if key == pygame.K_RIGHT:
            self.forward_key_held = False
        elif key == pygame.K_LEFT:
            self.backward_key_held = False    

