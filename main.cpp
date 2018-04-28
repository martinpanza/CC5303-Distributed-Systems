#include <iostream>
#include "./src/Node.h"
#include "./src/C.h"
#include "./src/T.h"
#include "utils.h"

Node* node;

int main(int argc, char* argv[]) {

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " T/C" << " ip" << " port" << std::endl;
        return 1;
    }

    if (std::string(argv[1]) == "C") {
        try{
            node = new C(std::string(argv[2]), (uint16_t) atoi(argv[3]));
        } catch (int e){
            std::cout << "Couldn't create C node";
        }
    } else if(std::string(argv[1]) == "T"){
        try{
            node = new T(std::string(argv[2]), (uint16_t) atoi(argv[3]));
        } catch (int e){
            std::cout << "Couldn't create T node";
        }
    } else{
        std::cerr << "Usage: " << argv[0] << " T/C" << std::endl;
        return 1;
    }
    node->run();
    return 0;
}
