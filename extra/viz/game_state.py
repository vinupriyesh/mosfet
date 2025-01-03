class GameState:
    def __init__(self, grid_size):
        self.grid_size = grid_size
        self.asteroids = []
        self.blue_shuttles = []
        self.red_shuttles = []
        self.nebula = []
        self.relics = []
        self.vantage_points = []
        self.halo_tiles = []

    def update_state(self, data):
        self.asteroids = data.get('asteroids', [])
        self.blue_shuttles = data.get('blue_shuttles', [])
        self.red_shuttles = data.get('red_shuttles', [])
        self.nebula = data.get('nebula', [])
        self.relics = data.get('relics', [])
        self.vantage_points = data.get('vantage_points', [])
        self.halo_tiles = data.get('halo_tiles', [])
