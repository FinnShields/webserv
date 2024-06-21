
/*enum class Config::tok {
	server, location, word, 
	semicol, newline,  // comment,
	open, close, eof};
*/

#include "Config.hpp"



//#include "WebServer.hpp"

int main(int argc, char *argv[]){
    if (argc < 2){
        std::cerr << "Usage: ./webserv [configuration file]" << std::endl;
        return 1;
    }
    try{
        Parser pars(argv[1]);
        Config conf(pars.get(),0);
        std::vector<t_server>& data = conf.get();
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
    catch (const std::runtime_error& e){
         std::cerr << e.what() << "\n";
//         std::cerr << "Unexpectedly got this:" << config.currentString() << "\n";
         return 1;
    }
/*    try
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
*/  
    return 0;
}