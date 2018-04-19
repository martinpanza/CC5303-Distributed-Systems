#include <iostream>
#include <vector>
#include "./Nodes/Node.h"
#include "./Nodes/C.h"
#include "./Nodes/T.h"

Node* node;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " T/C" << " ip" << " port" << " name" << std::endl;
        return 1;
    }

    if (std::string(argv[1]) == "C") {
        try{
            node = new C(std::string(argv[2]), int(argv[3]), std::string(argv[4]));
        } catch (int e){
            std::cout << "Couldn't create C node";
        }
    } else if(std::string(argv[1]) == "T"){
        try{
            node = new T(std::string(argv[2]), int(argv[3]), std::string(argv[4]));
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
