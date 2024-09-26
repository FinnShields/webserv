
import cgi
import os
import sys

sys.stdout.flush()

print("Content-Type: text/html\r\n")    # HTML is following
print()                             # blank line, end of headers

os.environ['My var'] = "my variable"

# HTML content
html_content = """
<html>
<head>
  <title>Simple CGI Script</title>
</head>
<body>
  <h1>Hello, World!</h1>
  <p>This is a simple CGI script written in Python.</p>
</body>
</html>
"""

# Print the HTML content
print(html_content)

# print("<html><head><title>Simple CGI Script</title></head><body><h1>Hello, World!</h1><p>This is a simple CGI script written in Python.</p></body></html>")
# print("<html>")
# print("<head>")
# print("<title>Simple CGI Script</title>")
# print("</head>")
# print("<body>")
# print("<h1>Hello, World!</h1>")
# print("<p>This is a simple CGI script written in Python.</p>")
# print("</body>")
# print("</html>")

# env_vars = "\n".join([f"{key}: {value}" for key, value in os.environ.items()])
# print(f"<pre>{env_vars}</pre>")