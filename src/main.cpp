#include <iostream>
#include <ComputeApplication.h>
using namespace std;

//On BasicRender branch
int main() {

    cout << "Running Compute Application" << endl;
    try {
		ComputeApplication::run();
    }
    catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    
    //open image
    system("\"Simple Image.png\"");
    return EXIT_SUCCESS;
}
