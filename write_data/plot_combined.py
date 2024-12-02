import matplotlib.pyplot as plt
import argparse

def plot_read_write_performance(size_labels, read_latencies, write_latencies):
    # Convert size labels to numerical values for plotting
    sizes = [int(size.replace("KB", "").replace("MB", "000")) for size in size_labels]
    
    plt.figure(figsize=(12, 6))
    
    # Plot Read Performance
    plt.plot(sizes, read_latencies, marker='o', linestyle='-', color='blue', label="Read Performance")
    
    # Plot Write Performance
    plt.plot(sizes, write_latencies, marker='o', linestyle='-', color='green', label="Write Performance")
    
    # Add plot details
    plt.title("Filesystem Read and Write Latency for Each File Size")
    plt.xlabel("File Size")
    plt.ylabel("Average Latency across 10 Runs (µs)")
    plt.xticks(sizes, size_labels, rotation=45)
    plt.grid(visible=True, linestyle='--', alpha=0.7)
    plt.legend()
    plt.tight_layout()
    plt.show()
    plt.savefig("read_write_performance.png")

def main():
    parser = argparse.ArgumentParser(description="Plot Read and Write Performance as trendlines.")
    parser.add_argument('--data', type=str, default=None,
                        help="Path to a CSV file containing Read and Write performance data. Uses default dataset if not specified.")
    args = parser.parse_args()

    # Default datasets
    size_labels = ["1KB", "100KB", "200KB", "300KB", "400KB", "500KB",
                   "600KB", "700KB", "800KB", "900KB", "1MB"]
    read_latencies = [87, 256, 325, 619, 614, 693, 853, 1049, 1141, 1367, 1219]
    write_latencies = [73, 227, 415, 603, 769, 771, 1052, 1384, 1259, 1407, 2037]

    # If additional data file is specified, implement logic to parse it
    if args.data:
        raise NotImplementedError("Custom data handling not implemented yet.")

    # Plot the trendlines
    plot_read_write_performance(size_labels, read_latencies, write_latencies)

if __name__ == "__main__":
    main()
