#include <stdio.h>
#include <GLFW/glfw3.h>

#ifdef _DEBUG
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "glfw3.lib")
#else
#pragma comment(lib, "glfw3_mt.lib")
#endif
#pragma comment(lib, "opengl32.lib")

void errorCallback(int code, const char* msg) {
	printf("error:%d msg:%s\n", code, msg);
}

int main() {
	GLFWwindow* window;
	
	glfwSetErrorCallback(errorCallback);

	/* init framework */
	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(640, 480, "GLFW", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	/* make context current */
	glfwMakeContextCurrent(window);

	/* loop until user close window */
	while (glfwWindowShouldClose(window) == 0) {
		/* render here */
		glClear(GL_COLOR_BUFFER_BIT);
		/* swap buffer */
		glfwSwapBuffers(window);
		/* poll for and process event */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}