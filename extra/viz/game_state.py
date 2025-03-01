LOWEST_DECIMAL = 1e-9

class GameState:
    def __init__(self, grid_size):
        self.grid_size = grid_size
        self.clear()

    def clear(self):              
        self.asteroids = []
        self.blue_shuttles = []
        self.blue_shuttles_actions = []
        self.red_shuttles = []
        self.nebula = []
        self.relics = []
        self.vantage_points = []
        self.halo_tiles = []
        self.energy = []
        self.vision = []
        self.unexplored_frontier = []
        self.blue_shuttles_energy = []
        self.red_shuttles_energy = []
        self.red_points = 0
        self.blue_points = 0
        self.red_wins = 0
        self.blue_wins = 0
        self.step = 0
        self.match_step = 0
        self.unit_move_cost = 0
        self.unit_sap_cost = 0
        self.unit_sap_range = 0
        self.unit_sensor_range = 0        

        self.tracker_data = []
        self.selected_tracker_data = [[0 for _ in range(self.grid_size[0])] for _ in range(self.grid_size[1])]
        self.active_tracker_count = 0.0

    def update_state(self, data, tracker_data, shuttle_toggle_state):
        if 'step' not in data:
            self.clear()
            return
        self.step = data.get('step', [])[0]
        points_data  = data.get('points', [])
        if len(points_data) == 2:
            self.blue_points, self.red_points = points_data
        wins_data  = data.get('wins', [])
        if len(wins_data) == 2:
            self.blue_wins, self.red_wins = wins_data
        self.match_step = data.get('match_step', [])[0]
        self.unit_move_cost = data.get('unit_move_cost', [])
        self.unit_sap_cost = data.get('unit_sap_cost', [])
        self.unit_sap_range = data.get('unit_sap_range', [])
        self.unit_sensor_range = data.get('unit_sensor_range', [])
        self.asteroids = data.get('asteroids', [])
        self.blue_shuttles = data.get('blue_shuttles', [])
        self.blue_shuttles_actions = data.get('blue_shuttles_actions', [])
        self.red_shuttles = data.get('red_shuttles', [])
        self.nebula = data.get('nebula', [])
        self.relics = data.get('relics', [])
        self.vantage_points = data.get('vantage_points', [])
        self.halo_tiles = data.get('halo_tiles', [])
        self.energy = data.get('energy', [])
        self.vision = data.get('vision', [])
        self.unexplored_frontier = data.get('unexplored_frontier', [])
        self.blue_shuttles_energy = data.get('blue_shuttles_energy', [])
        self.red_shuttles_energy = data.get('red_shuttles_energy', [])

        self.tracker_data = tracker_data
        self.update_opponent_tracker_data(shuttle_toggle_state)

    def update_opponent_tracker_data(self, shuttle_toggle_state):

        shuttle_has_data = [False] * 16
        self.active_tracker_count = 0.0

        for x in range(0, self.grid_size[0]):
            for y in range(0, self.grid_size[1]):
                self.selected_tracker_data[x][y] = 0.0
                for s in range(0, 16):
                    if shuttle_toggle_state[s]:
                        self.selected_tracker_data[x][y] += self.tracker_data[s][x][y]
                        if not shuttle_has_data[s] and self.tracker_data[s][x][y] > LOWEST_DECIMAL :
                            self.active_tracker_count += 1.0
                            shuttle_has_data[s] = True
        

            