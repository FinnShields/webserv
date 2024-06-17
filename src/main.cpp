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
#include "Config.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./webserv [configuration file]" << std::endl;
        return 1;
    }
/*/
    try{
        Config config(argv[1]);
        std::vector<t_server>& data = config.parseFile();
        std::cout
            << "----- DATA ------------\n"
            << data
            << "----- END of DATA -----\n";
        std::cout << "value=value [server=0, name2, key] = ("
            << data[0]["name2"]["key"]<< ")\n";
        std::cout << "value=none, [0, name2, keynoval] =("
            << data[0]["name2"]["keynoval"] << ")\n";
        std::cout << "value for noexistkey, [0, name2, noexistkey] =("
            << data[0]["name2"]["noexistkey"] << ")\n";
    }
    catch (const std::ios_base::failure& e) {
        std::cerr << e.what() << std::endl;
    }
    catch (const std::runtime_error& e){
         std::cerr << e.what() << "\n";
         return 1;
    }
*/
    try
    {
        WebServer webserv;
        webserv.setup(argv[1]);
        webserv.run();
    }
    catch (const std::ios_base::failure& e) {
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