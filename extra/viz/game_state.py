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
        self.step = 0
        self.match_step = 0

    def update_state(self, data):
        if 'step' not in data:
            self.clear();
            return
        self.step = data.get('step', [])[0]
        self.match_step = data.get('match_step', [])[0]
        self.asteroids = data.get('asteroids', [])
        self.blue_shuttles = data.get('blue_shuttles', [])
        self.red_shuttles = data.get('red_shuttles', [])
        self.nebula = data.get('nebula', [])
        self.relics = data.get('relics', [])
        self.vantage_points = data.get('vantage_points', [])
        self.halo_tiles = data.get('halo_tiles', [])
        self.energy = data.get('energy', [])
