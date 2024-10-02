/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:39 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/05 10:46:09 by apimikov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "WebServer.hpp"
#include "Parser.hpp"
#include "Config.hpp"

#define DEFAULT_CONFIG "config/three_servers.conf"
#define DEBUG_MODE 1

int main(int argc, char *argv[]) {
    try
    {
        std::string filename = argc < 2 ? DEFAULT_CONFIG : argv[1];
        std::cout << "FAB WebServer\n[INFO] Config file: " << filename << std::endl;
        Parser data(filename);
        if (DEBUG_MODE){
            std::cout << "----- EXTRACTED DATA ------------\n"
                    << data.get()
                    << "----- END of DATA ---------------\n";
        }
        data.isValid();
        WebServer webserv(data.get());
        webserv.setup();
        webserv.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Caught exception in main" << std::endl;
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
