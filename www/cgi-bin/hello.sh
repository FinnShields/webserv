#!/bin/bash

# Function to parse query string
parse_query_string() {
    local query_string="$1"
    local -n params=$2
    IFS='&' read -r -a pairs <<< "$query_string"
    for pair in "${pairs[@]}"; do
        IFS='=' read -r key value <<< "$pair"
        params["$key"]="$value"
    done
}

# Parse query string
declare -A QUERY_PARAMS
parse_query_string "$QUERY_STRING" QUERY_PARAMS

# Output the CGI headers
echo "Content-type: text/html"
echo ""

# Output HTML content
echo "<html>"
echo "<head><title>CGI Environment Variables</title></head>"
echo "<body>"
echo "<h1>CGI Environment Variables</h1>"

# Print environment variables
echo "<h2>Environment Variables</h2>"
echo "<pre>"
env
echo "</pre>"

# Print body (input from POST request)
echo "<h2>Request Body</h2>"
if [[ "$REQUEST_METHOD" == "POST" ]]; then
    echo "<pre>"
    read POST_DATA
    echo "$POST_DATA"
    echo "</pre>"
else
    echo "<p>No POST data received.</p>"
fi

echo "</body>"
echo "</html>"
