class GameState:
    def __init__(self, grid_size):
        self.grid_size = grid_size
        self.clear()

    def clear(self):              
        self.asteroids = []
        self.blue_shuttles = []
        self.red_shuttles = []
        self.nebula = []
        self.relics = []
        self.vantage_points = []
        self.halo_tiles = []
        self.energy = []
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

    def update_state(self, data):
        if 'step' not in data:
            self.clear();
            return
        self.step = data.get('step', [])[0]
        self.blue_points, self.red_points  = data.get('points', [])
        self.blue_wins, self.red_wins  = data.get('wins', [])
        self.match_step = data.get('match_step', [])[0]
        self.unit_move_cost = data.get('unit_move_cost', [])
        self.unit_sap_cost = data.get('unit_sap_cost', [])
        self.unit_sap_range = data.get('unit_sap_range', [])
        self.unit_sensor_range = data.get('unit_sensor_range', [])
        self.asteroids = data.get('asteroids', [])
        self.blue_shuttles = data.get('blue_shuttles', [])
        self.red_shuttles = data.get('red_shuttles', [])
        self.nebula = data.get('nebula', [])
        self.relics = data.get('relics', [])
        self.vantage_points = data.get('vantage_points', [])
        self.halo_tiles = data.get('halo_tiles', [])
        self.energy = data.get('energy', [])
        self.unexplored_frontier = data.get('unexplored_frontier', [])
        self.blue_shuttles_energy = data.get('blue_shuttles_energy', [])
        self.red_shuttles_energy = data.get('red_shuttles_energy', [])
