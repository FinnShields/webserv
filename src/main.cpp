/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:39 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/29 14:29:40 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "WebServer.hpp"
#include "Parser.hpp"
#include "Config.hpp"

#define DEFAULT_CONFIG "config/three_servers.conf"

int main(int argc, char *argv[]) {
    try
    {
        std::string filename = argc < 2 ? DEFAULT_CONFIG : argv[1];
        std::cout << "FAB WebServer\nConfig: " << filename << std::endl;
        Parser data(filename);
        std::cout << "----- EXTRACTED DATA ------------\n"
		        << data.get()
    	    	<< "----- END of DATA ---------------\n";
        data.isValid();
        WebServer webserv(data.get());
        webserv.setup();
        webserv.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Caught exception in main" << std::endl;
        std::cerr << e.what() << '\n';
        std::exit(EXIT_FAILURE);
    }
    return 0;
}
