/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:39 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/07/07 12:09:05 by apimikov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "WebServer.hpp"
#include "Parser.hpp"
#include "Config.hpp"

int main(int argc, char *argv[]) {
    std::cout << argv[0] << "\n";
    if (argc < 2) {
        std::cerr << "Usage: ./webserv [configuration file]" << std::endl;
        return 1;
    }
    try
    {
        Parser data(argv[1]);
        std::cout
		<< "----- EXTRACTED DATA ------------\n"
		<< data.get()
		<< "----- END of DATA ---------------\n";
        data.isValid();
        WebServer webserv(data.get());
        webserv.setup();
        webserv.run();
    }
    catch (const std::ios_base::failure& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (const std::runtime_error& e){
         std::cerr << e.what() << "\n";
         return 1;
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