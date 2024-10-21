# FAB server
HTTP server in C++ (Project at School 42)

Server created by [@FinnShields](https://github.com/FinnShields), [@dnapi](https://github.com/dnapi) and [@Moridar](https://github.com/Moridar)

FAB server is a lightweight web server designed as part of the curriculum at Hive Helsinki (42 School). The project focuses on developing a deep understanding of HTTP protocols, web architecture. It aims to provide students with hands-on experience in building a fully functional web server from scratch.

## Features

- **HTTP/1.1 Support**: Almost full compliance with the HTTP/1.1 specification, including methods like GET, POST, DELETE, and more.
- **Non blocking**: Handles multiple requests simultaneously.
- **Static and Dynamic Content**: Serve static files and execute CGI scripts for dynamic content generation.
- **Configuration**: Customizable server settings through a user-friendly configuration file.

## Installation

### Prerequisites

- C++17 or later
- CMake
- Make

### Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/FinnShields/webserv.git
   cd webserv
   ```
2. Build:
    ``` bash 
    make
    ```
2. Run the server:
    ``` bash 
    ./webserve your_config_file.conf
    ```

## Configuration file

The server is configured via a JSON-like file. Here’s an example of a basic configuration:
```
server  # server 0
{
	group main
	{
		listen					4200;     #comment
		host					127.0.0.1;
		limit_except			GET;
		client_max_body_size	30000000;
		index					index_cgi.html;
		root					www;
		error_page				404 www/error_pages/404.html;
	    #return                  777 http://www.google.com;
	}

    group /uploads
    {
        limit_except            GET POST DELETE PUT HEAD;
    }
	group /cgi-bin
	{
		root					www/cgifolder;
		limit_except			POST GET DELETE;
		autoindex				off;
		cgi_ext					.cgi .py .sh;
		cgi_path				.cgi /usr/bin/python3 /bin/bash;  # correct
	}
    group /project
    {
         return 301 https://github.com/FinnShields/webserv;
    }
}

server    #  name based virtual server
{
	group main
	{
		server_name 			localhost;
		listen 4200;
        host    127.0.0.1;
		index					index.html;
		root					www;
	}
}
```

## Results

✅ Passed with 125/100 ✅

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgments

- Hive School for providing the opportunity to work on this project.
- The  Hive School community for invaluable recommendations and inspiration.
 
