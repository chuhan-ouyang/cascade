import os
import pandas as pd
import matplotlib.pyplot as plt

# Ensure the 'perf' directory exists
output_dir = 'perf'
os.makedirs(output_dir, exist_ok=True)

# Step 1: Load the data
columns = ['timestampTag', 'info1', 'runNumber', 'timestamp', 'info2']
fuse_perftest = pd.read_csv('fuse_perftest_logger.csv', delim_whitespace=True, names=columns, usecols=['timestampTag', 'runNumber', 'timestamp'])
fuse_client = pd.read_csv('fuse_client_logger.csv', delim_whitespace=True, names=columns, usecols=['timestampTag', 'runNumber', 'timestamp'])

# Step 2: Combine the data
combined_data = pd.concat([fuse_perftest, fuse_client])

# Prepare a dictionary to store phase differences for each run
phases_diff = {i: [] for i in range(1, 5)} # For phase 1 to 4
run_numbers = []

# Step 3: Calculate phase differences for each run
for run_number in sorted(combined_data['runNumber'].unique()):
    run_data = combined_data[combined_data['runNumber'] == run_number]
    timestamps = {}
    for tag in [1000, 1001, 1002, 1003, 1004]:
        timestamp = run_data[run_data['timestampTag'] == tag]['timestamp'].min()
        timestamps[tag] = timestamp

    # Store phase differences in microseconds
    for i in range(1, 5):
        phase_diff = (timestamps[1000 + i] - timestamps[1000 + i - 1]) / 1000  # Convert ns to microseconds
        phases_diff[i].append(phase_diff)
    
    run_numbers.append(run_number)

# Step 4: Plot and save
for i in range(1, 5):
    plt.figure(figsize=(10, 6))
    plt.scatter(run_numbers, phases_diff[i], label=f'Phase {i} Difference')
    plt.title(f'Phase {i} Difference Across Runs')
    plt.xlabel('Run Number')
    plt.ylabel('Time Difference (microseconds)')
    plt.legend()
    plt.tight_layout()
    
    # Saving plot
    plt.savefig(f'{output_dir}/phase_{i}_perf_plot.png')
    plt.close()
