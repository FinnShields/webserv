#!/usr/bin/env python3

import cgi
import cgitb

# Enable error handling
cgitb.enable()

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields
text_content = form.getvalue('textcontent')

# HTTP header
print("Content-Type: text/html\n")

# HTML content
print(f"""
<html>
<head>
  <title>CGI Script Output</title>
</head>
<body>
  <h2>Your Submitted Text</h2>
  <p>{text_content}</p>
</body>
</html>
""")
