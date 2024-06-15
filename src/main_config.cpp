
/*enum class Config::tok {
	server, location, word, 
	semicol, newline,  // comment,
	open, close, eof};
*/

#include "Config.hpp"



int main() {

    Config config("test_config.txt");
    //config.peek();
   // while ((config.getToken()) != ParceConf::tok::eof)
    //    ;
    std::cout << "word = " << config.parseWord() 
            << "\n";    
    int i =-1;
    while (++i < 30){
        config.getToken();
        /*if (config._token == Config::tok::word)
            std::cout << "word = " << config.parseWord() 
            << "\n";
        */
    }
    return 0;
}
