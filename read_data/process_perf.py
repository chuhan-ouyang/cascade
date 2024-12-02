import pandas as pd
import sys

def calculate_latency(file_path):
    # Read the CSV file into a DataFrame
    df = pd.read_csv(file_path, delim_whitespace=True, header=None, names=['Col1', 'Col2', 'Iteration', 'Time', 'Col5'])

    # Filter for relevant columns: Iteration and Time
    df = df[['Iteration', 'Time']]

    # Calculate latency for each iteration
    latency_results = {}
    for iteration in df['Iteration'].unique():
        times = df[df['Iteration'] == iteration]['Time'].values
        if len(times) == 2:  # Ensure there is a start and end time
            latency = times[1] - times[0]
            latency_results[iteration] = latency

    # Convert results to a DataFrame
    latency_df = pd.DataFrame(list(latency_results.items()), columns=['Iteration', 'Latency'])

    # Calculate average latency
    average_latency = latency_df['Latency'].mean()
    variance_latency = latency_df['Latency'].var()

    # Print iteration vs latency
    print("Iteration vs Latency:")
    print(latency_df)

    # Print average latency
    print("\nAverage Latency (ms):")
    print(average_latency / 1000)

    print("\nVariance of Latency (ms):")
    print(variance_latency / (1000**2))  # Convert from microseconds squared to milliseconds squared


if __name__ == "__main__":
    # Check if the file path is provided as a command-line argument
    if len(sys.argv) < 2:
        print("Usage: python latency_calculator.py <input_file.csv>")
        sys.exit(1)

    # Get the file path from the command-line argument
    input_file = sys.argv[1]

    # Call the function to calculate latency
    calculate_latency(input_file)
