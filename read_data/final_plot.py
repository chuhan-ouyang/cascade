import matplotlib.pyplot as plt
import argparse

def plot_latency_trend(size_labels, latencies):
    sizes = [int(size.replace("KB", "").replace("MB", "000")) for size in size_labels]
    
    plt.figure(figsize=(10, 6))
    plt.plot(sizes, latencies, marker='o', linestyle='-', label="Average Latency")
    plt.title("Filesystem Read Latency for Each File Size")
    plt.xlabel("File Size")
    plt.ylabel("Average Latency across 10 Runs (µs)")
    plt.xticks(sizes, size_labels, rotation=45)
    plt.grid(visible=True, linestyle='--', alpha=0.7)
    plt.legend()
    plt.tight_layout()
    plt.show()
    plt.savefig("read_performance.png")

def main():
    parser = argparse.ArgumentParser(description="Plot Read Performance as a trendline.")
    parser.add_argument('--data', type=str, default=None,
                        help="Path to CSV file with Size and Average Latency data. Use default dataset if not specified.")
    args = parser.parse_args()

    # Default dataset
    size_labels = ["1KB", "100KB", "200KB", "300KB", "400KB", "500KB",
                   "600KB", "700KB", "800KB", "900KB", "1MB"]
    latencies = [87, 256, 325, 619, 614, 693, 853, 1049, 1141, 1367, 1219]

    # If additional data file is specified, implement logic to parse it
    if args.data:
        # Add parsing logic here if needed for custom datasets
        raise NotImplementedError("Custom data handling not implemented yet.")

    # Plot the trendline
    plot_latency_trend(size_labels, latencies)

if __name__ == "__main__":
    main()
