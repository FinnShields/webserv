
#include "ParceConf.hpp"

int main() {

    ParceConf config("test_config.txt");
    //config.peek();
   // while ((config.getToken()) != ParceConf::tok::eof)
    //    ;
    int i =-1;
    while (++i < 30)
        config.getToken();
    return 0;
}
