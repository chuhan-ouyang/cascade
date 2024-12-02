import matplotlib.pyplot as plt
import argparse

def plot_write_performance(size_labels, latencies):
    # Convert size labels to numerical values for plotting
    sizes = [int(size.replace("KB", "").replace("MB", "000")) for size in size_labels]
    
    plt.figure(figsize=(10, 6))
    plt.plot(sizes, latencies, marker='o', linestyle='-', color='blue', label="Average Write Latency")
    plt.title("Filesystem Write Latency for Each File Size")
    plt.xlabel("File Size")
    plt.ylabel("Average Latency across 10 Runs (µs)")
    plt.xticks(sizes, size_labels, rotation=45)
    plt.grid(visible=True, linestyle='--', alpha=0.7)
    plt.legend()
    plt.tight_layout()
    plt.show()
    plt.savefig("write_performance.png")

def main():
    parser = argparse.ArgumentParser(description="Plot Write Performance as a trendline.")
    parser.add_argument('--data', type=str, default=None,
                        help="Path to CSV file with Size and Average Write Latency data. Use default dataset if not specified.")
    args = parser.parse_args()

    # Default dataset
    size_labels = ["1KB", "100KB", "200KB", "300KB", "400KB", "500KB",
                   "600KB", "700KB", "800KB", "900KB", "1MB"]
    latencies = [73, 227, 415, 603, 769, 771, 1052, 1384, 1259, 1407, 2037]

    # If additional data file is specified, implement logic to parse it
    if args.data:
        raise NotImplementedError("Custom data handling not implemented yet.")

    # Plot the trendline
    plot_write_performance(size_labels, latencies)

if __name__ == "__main__":
    main()
