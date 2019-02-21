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
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    
	
    //open image
	system("\"Rendered Image.png\"");
	

    return EXIT_SUCCESS;
}

