#include <iostream>
#include "../include/ComputeApplication.h"
using namespace std;

//On Branch UseVkImage
int main() {
    ComputeApplication app;

    cout << "Running Compute Application" << endl;
    try {
        app.run();
    }
    catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    
    //open image
    system("\"Simple Image.png\"");
    return EXIT_SUCCESS;
}
