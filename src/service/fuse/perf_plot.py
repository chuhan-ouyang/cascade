import pandas as pd
import matplotlib.pyplot as plt
import os

# Load the datasets with space as the separator
fuse_perftest_data = pd.read_csv('fuse_perftest_logger.csv', header=None, names=['Label', 'Unknown', 'RunID', 'Timestamp', 'Extra'], sep=' ')
fuse_client_data = pd.read_csv('fuse_client_logger.csv', header=None, names=['Label', 'Unknown', 'RunID', 'Timestamp', 'Extra'], sep=' ')

# Combine the data
combined_data = pd.concat([fuse_perftest_data, fuse_client_data])

# Convert timestamps to numeric for plotting
combined_data['Timestamp'] = pd.to_numeric(combined_data['Timestamp'])

# Create a directory for the plots if it doesn't exist
plot_directory = "plots"
if not os.path.exists(plot_directory):
    os.makedirs(plot_directory)

# Plot a scatter plot for each run
for run_id in combined_data['RunID'].unique():
    # Filter data for the current run
    run_data = combined_data[combined_data['RunID'] == run_id]

    # Create scatter plot
    plt.figure(figsize=(10, 6))
    plt.scatter(run_data['Label'], run_data['Timestamp'], alpha=0.6, edgecolors='w', linewidth=0.5)

    # Set plot title and labels
    plt.title(f'Scatter Plot for Run {run_id}')
    plt.xlabel('Label')
    plt.ylabel('Timestamp (ns)')

    # Set x-ticks for labels
    plt.xticks([1000, 1001, 1002, 1003, 1004])

    # Show plot
    plt.grid(True)
    # Instead of plt.show(), save the figure to a file in the plots directory
    filename = os.path.join(plot_directory, f'scatter_plot_run_{run_id}.png')
    plt.savefig(filename)
    # Clear the current figure to prevent overlap with next plots
    plt.clf()

print(f"All plots saved in the '{plot_directory}' directory.")
