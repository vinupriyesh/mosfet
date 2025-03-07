import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os
import glob
from matplotlib.ticker import MaxNLocator
import json
from matplotlib.ticker import ScalarFormatter

def delete_all_files(folder_path):
    # Get a list of all files in the folder
    files = glob.glob(os.path.join(folder_path, '*.png'))
    
    # Iterate through the list and delete each file
    for file in files:
        try:
            os.remove(file)
            print(f"Deleted {file}")
        except Exception as e:
            print(f"Error deleting {file}: {e}")


def plot(df, images, dimensions, ylabel='Value', xlabel='Timestep', output_folder='output', sum_values=False):
    # Check if dimensions is a string and convert it to a list
    if isinstance(dimensions, str):
        df_filtered = df[df['dimension'] == dimensions]
        graph_name = dimensions
        style = None
    else:
        df_filtered = df[df['dimension'].isin(dimensions)]
        graph_name = f'{"-".join(dimensions)}'
        style = 'dimension'

    if sum_values:
        # Sum the values per timestep if the flag is set
        df_filtered = df_filtered.groupby(['timestep', 'player_id', 'dimension']).agg({'value': 'sum'}).reset_index()

    # Create a seaborn lineplot
    fig, ax = plt.subplots(figsize=(14, 8))
    sns.lineplot(data=df_filtered, x='timestep', y='value', hue='player_id', style=style, estimator=None)
    
    # Add plot title and labels
    ax.set_title(f'{graph_name}', fontsize=16)
    ax.set_xlabel(xlabel, fontsize=14)
    ax.set_ylabel(ylabel, fontsize=14)

    # Format y-axis to show regular decimal numbers
    ax.yaxis.set_major_formatter(ScalarFormatter(useOffset=False))
    ax.yaxis.set_major_formatter(plt.FuncFormatter(lambda x, _: f'{x:.1f}'))

    fig.autofmt_xdate(rotation=45, ha='right')
    ax.xaxis.set_major_locator(MaxNLocator(integer=True))

    plt.tight_layout(pad=2.0)

    # Show the plot
    filename = output_folder + "/" + graph_name + ".png"
    plt.savefig(filename)
    plt.close()

    images.append(filename)

    print(f"Created {graph_name}")

def create_heading(name:str, json_data: dict):
    images = []
    heading = {
        "title": name,
        "items": images
        }
    json_data["headings"].append(heading) 
    return images

def prepare_charts(df):

    json_data = {"headings": []}

    # Performance    
    images = create_heading("Performance", json_data)
    plot(df, images, "step_duration", "ms")
    plot(df, images, "memory", "mb")

    # Value
    images = create_heading("Value", json_data)
    plot(df, images, "points")
    plot(df, images, "opponentPoints")
    # plot(df, "energy")

    # Energy Details
    images = create_heading("Energy Details", json_data)
    plot(df, images, "energy_lost_in_collision", "points")
    plot(df, images, "movement_loss", "points")
    plot(df, images, "energy_fields", "points")
    plot(df, images, "sap_loss", "points")
    plot(df, images, "melee_loss", "points")
    plot(df, images, "nebula_loss", "points")

    # Battle information
    images = create_heading("Battle information", json_data)
    plot(df, images, "total_sos_issued", "points")    

    # Relic exploration
    images = create_heading("Relic Exploration", json_data)
    plot(df, images, "constraint_set_size", "size")    
    plot(df, images, 'phased_out_constraints', "size", sum_values=True)
    
    # Relic exploitation
    images = create_heading("Relic Exploitation", json_data)
    plot(df, images, "unexploited_vantage_points")

    # Planning
    images = create_heading("Planning", json_data)
    plot(df, images, "jobs_created", "count")
    plot(df, images, "job_applications", "count")
    plot(df, images, "declined_job_applications", "count")

    # Timing
    images = create_heading("Timing", json_data)
    plot(df, images, "shuttle_energy_tracking", "ms")
    plot(df, images, "visualizer_overhead", "ms")    
    # plot(df, images, "act_duration", "ms")
    plot(df, images, "update_duration", "ms")
    plot(df, images, "plan_duration", "ms")
    plot(df, images, "pathing_duration", "ms")
    plot(df, images, 'add_constraint_duration', "micro seconds")
    
    return json.dumps(json_data, indent=4)

# Example usage
# plot_seaborn('metrics.csv', 'desired_dimension')
if __name__ == "__main__":
    print("Charting..")
    
    delete_all_files("output")

    df = pd.read_csv('metrics.csv')
    df['value'] = pd.to_numeric(df['value'], errors='coerce')

    json_data = prepare_charts(df)

    # Write to file
    with open('dashboard.template', 'r') as html_file:
        html_content = html_file.read()

        html_content = html_content.replace('%%PLACEHOLDER%%', json_data)

        with open('dashboard.html', 'w') as output_file:
            output_file.write(html_content)