#!/usr/bin/python

import cgi
import os
import subprocess
import base64

def generate_histogram(directory):
    # Execute my-histogram.py and capture its output
    my_histogram_cmd = f"python3 my-histogram.py {directory}"
    my_histogram_output = subprocess.check_output(my_histogram_cmd, shell=True)

    # Encode the histogram image as base64
    histogram_image = base64.b64encode(my_histogram_output).decode()

    return histogram_image

def generate_html(histogram_image):
    html = """
    <!DOCTYPE html>
    <html>
    <head>
        <title>CS410 Webserver</title>

    </head>
    <body>
        <h1>CS410 Webserver</h1>
        <br>
        <img src="data:image/png;base64,{}" alt="Histogram">
    </body>
    </html>
    """.format(histogram_image)

    return html

# Parse the query parameters
form = cgi.FieldStorage()
directory = form.getvalue("directory", ".")

# Generate the histogram image
histogram_image = generate_histogram(directory)

# Generate the HTML content
html_content = generate_html(histogram_image)

# Send the HTTP response
print("Content-Type: text/html")
print("Content-Length: {}".format(len(html_content)))
print()
print(html_content)