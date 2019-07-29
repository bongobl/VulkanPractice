#include <iostream>
#include <RenderApplication.h>

using namespace std;

//On Particles branch
int main() {

    cout << "Running Render Application" << endl;
    
	/*
	float aspect = 3;
	glm::vec4 result = glm::perspective(glm::radians(90.0f), aspect, 0.2f, 300.0f) * glm::vec4(5,5,-5,1);

	result /= result.w;

	result.x *= aspect;
	cout << "Result: (" << result.x << ", " << result.y << ")" << endl;
	*/
    try {
		RenderApplication::run();
    }
    catch (const std::runtime_error& e) {
        cout << "Exception Thrown: " << e.what() << endl;
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}

