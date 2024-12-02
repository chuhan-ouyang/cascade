import csv
import argparse

def calculate_average_latency(file_path):
    latencies = []
    with open(file_path, 'r') as file:
        csv_reader = csv.reader(file)
        next(csv_reader)  # Skip the header
        for row in csv_reader:
            latencies.append(int(row[1]))
    average_latency = sum(latencies) / len(latencies) if latencies else 0
    return average_latency

def main():
    parser = argparse.ArgumentParser(description="Calculate the average latency from a CSV file.")
    parser.add_argument('csv_file', type=str, help="Path to the CSV file containing latency data.")
    args = parser.parse_args()
    
    average_latency = calculate_average_latency(args.csv_file)
    average_latency_us = average_latency / 1_000
    print(f"Average Latency: {average_latency_us:.2f} us")

if __name__ == '__main__':
    main()