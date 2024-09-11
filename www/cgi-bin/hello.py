#!/usr/bin/env python3

import cgi
import cgitb
import os  # Import the os module

# Enable error handling
cgitb.enable()

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields
text_content = form.getvalue('textcontent')

# Get the current working directory
current_directory = os.getcwd()

# HTTP header
print("Content-Type: text/html\r\n")
print()

# HTML content
print(f"""
<html>
<head>
  <title>CGI Script Output</title>
</head>
<body>
  <h2>Your Submitted Text</h2>
  <p>{text_content}</p>

  <h2>Current Directory</h2>
  <p>{current_directory}</p>
</body>
</html>\r\n\r\n\r\n
""")