import requests

base_url = "https://www.kaggle.com/api/i/competitions.EpisodeService/"
get_url = base_url + "GetEpisodeReplay"
re = requests.post(get_url, json = {"episodeId": 58639402})

print(re)

# POST on below will get the details about the game
# https://www.kaggle.com/api/i/competitions.EpisodeService/ListEpisodes
# {"ids":[70723780]}

# POST on below will give all the episodes under a submission
# https://www.kaggle.com/api/i/competitions.EpisodeService/ListEpisodes
# {"ids":[],"submissionId":43152191}