#!/usr/bin/python

import sys
import subprocess
import os 

def generate_histogram(directory):
    # Execute my-histogram.py and capture its output
    my_histogram_cmd = f"python3 my-histogram.py {directory}"
    subprocess.check_output(my_histogram_cmd, shell=True)
def generate_html(histogram_image):
    print("<!DOCTYPE html>")
    print("<html>")
    print("<head>")
    print("<style>")
    print("h1 {text-align: center;}")
    print("img {display: block;margin-left: auto;margin-right: auto;}")
    print("</style>")
    print("</head>")
    print("<body>")
    print("<h1 style='color: red; font-size: 16pt;'>CS410 Webserver</h1>")
    print("</br>")
    print(f"<img src={histogram_image} class='center'> ")
    print("</body>")
    print("</html>")
# Retrieve the command-line argument
if len(sys.argv) > 1:
    arg_value = sys.argv[1]
    generate_histogram(arg_value)
    generate_html("histogram.jpg")
else:
    arg_value = None
    print("<!DOCTYPE html>")
    print("<html>")
    print("<head>")
    print("</head>")
    print("<body>")
    print("<h1>No data found</h1>")
    print("</body>")
    print("</html>")






