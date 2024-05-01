#!/usr/bin/python
import os
import sys
import matplotlib.pyplot as plt

def count_file_types(directory):
    file_types = {
        "regular": 0,
        "directory": 0,
        "link": 0,
        "fifo": 0,
        "socket": 0,
        "block": 0,
        "character": 0
    }

    for root, dirs, files in os.walk(directory):
        for entry in dirs + files:
            entry_path = os.path.join(root, entry)
            if os.path.isfile(entry_path):
                file_types["regular"] += 1
            elif os.path.isdir(entry_path):
                file_types["directory"] += 1
            elif os.path.islink(entry_path):
                file_types["link"] += 1
            elif os.path.isfifo(entry_path):
                file_types["fifo"] += 1
            elif os.path.issocket(entry_path):
                file_types["socket"] += 1
            elif os.path.isblk(entry_path):
                file_types["block"] += 1
            elif os.path.ischr(entry_path):
                file_types["character"] += 1

    return file_types

def generate_histogram(file_types):
    labels = list(file_types.keys())
    counts = list(file_types.values())

    fig, ax = plt.subplots()
    ax.bar(labels, counts)
    ax.set_xlabel("File Types")
    ax.set_ylabel("Frequency")
    ax.set_title("File Type Histogram")
    plt.xticks(rotation=45)
    plt.tight_layout()

    plt.savefig("histogram.jpg")
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: my-histogram.py <directory>")
        sys.exit(1)
    directory = sys.argv[1]

    file_types = count_file_types(directory)
    generate_histogram(file_types)