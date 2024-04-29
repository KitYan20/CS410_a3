#!/usr/bin/python

import os
import sys

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

def generate_gnuplot_commands(file_types):
    gnuplot_commands = """
    set term png
    set output "/dev/stdout"
    set style data histogram
    set style fill solid
    set boxwidth 0.8
    set xtic rotate by -45
    set xlabel "File Types"
    set ylabel "Frequency"
    plot "-" using 2:xtic(1) title "" with boxes
    """

    for file_type, count in file_types.items():
        gnuplot_commands += f'"{file_type}" {count}\n'

    gnuplot_commands += "e"

    return gnuplot_commands

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: my-histogram <directory>")
        sys.exit(1)

    directory = sys.argv[1]
    file_types = count_file_types(directory)
    gnuplot_commands = generate_gnuplot_commands(file_types)

    print(gnuplot_commands)