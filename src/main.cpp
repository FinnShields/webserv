/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:39 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/14 12:21:41 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "WebServer.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./webserv [configuration file]" << std::endl;
        return 1;
    }

    try
    {
        WebServer webserv;
        webserv.setup(argv[1]);
        webserv.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Caught exception in main" << std::endl;
        std::cerr << e.what() << '\n';
    }
    catch(std::string e)
    {
        std::cerr << "Caught string in main" << std::endl;
        std::cerr << e << '\n';
    }
    
    return 0;
}