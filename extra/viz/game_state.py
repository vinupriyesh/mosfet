class GameState:
    def __init__(self, grid_size):
        self.grid_size = grid_size
        self.asteroids = []
        self.blue_shuttles = []
        self.red_shuttles = []
        self.nebula = []

    def update_state(self, data):
        self.asteroids = data.get('asteroids', [])
        self.blue_shuttles = data.get('blue_shuttles', [])
        self.red_shuttles = data.get('red_shuttles', [])
