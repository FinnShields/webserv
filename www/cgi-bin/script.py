#!/usr/bin/python3

import cgi

print("Content-Type: text/html\r\n")    # HTML is following
print()                             # blank line, end of headers

print("<html>")
print("<head>")
print("<title>Simple CGI Script</title>")
print("</head>")
print("<body>")
print("<h1>Hello, World!</h1>")
print("<p>This is a simple CGI script written in Python.</p>")
print("</body>")
print("</html>")
