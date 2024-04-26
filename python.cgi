#!/usr/bin/python

import cgi

# Raw data
raw_data = "John,Doe,25\nJane,Smith,30\nBob,Johnson,35"

# Process the raw data
lines = raw_data.split("\n")
formatted_data = []

for line in lines:
    first_name, last_name, age = line.split(",")
    formatted_data.append(f"<tr><td>{first_name}</td><td>{last_name}</td><td>{age}</td></tr>")

table_rows = "\n".join(formatted_data)

# Generate the HTML output
print("Content-Type: text/html")
print()
print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>Processed Data</title>
</head>
<body>
    <h1>Processed Data</h1>
    <table>
        <tr>
            <th>First Name</th>
            <th>Last Name</th>
            <th>Age</th>
        </tr>
        {table_rows}
    </table>
</body>
</html>
""")