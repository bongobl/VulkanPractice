#include <iostream>
#include <RenderApplication.h>


using namespace std;

//On BasicRender branch
int main() {

    cout << "Running Render Application" << endl;

    try {
		RenderApplication::run();
    }
    catch (const std::runtime_error& e) {
        cout << "Exception Thrown: " << e.what() << endl;
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}

