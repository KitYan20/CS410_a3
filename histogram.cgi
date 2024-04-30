#!/usr/bin/python
import cgi
import os
import sys
import subprocess
import base64

def generate_histogram(directory):
    # Execute my-histogram.py and capture its output
    my_histogram_cmd = f"python3 my-histogram.py {directory}"
    subprocess.check_output(my_histogram_cmd, shell=True)

def generate_html(histogram_image):
    with open(histogram_image, "rb") as file:
        histogram_base64 = base64.b64encode(file.read()).decode('utf-8')
        html = """
        <!DOCTYPE html>
        <html>
        <head>
            <title>CS410 Webserver</title>
        </head>
        <body>
            <h1>CS410 Webserver</h1>
            <br>
            <img src="data:image/jpeg;base64,{}">
        </body>
        </html>
        """.format(histogram_base64)
        return html

# Generate the histogram image
generate_histogram(".")

# Generate the HTML content
html_content = generate_html("histogram.jpg")

# Send the HTTP response
print("Content-Type: text/html")
print("Content-Length: {}".format(len(html_content)))
print()
print(html_content)
os.remove("histogram.jpg")