#include <stdio.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#ifdef _DEBUG
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "glfw3.lib")
#else
#pragma comment(lib, "glfw3_mt.lib")
#endif
#pragma comment(lib, "opengl32.lib")

#include <opencv2/opencv.hpp>
#ifdef _DEBUG
#pragma comment(lib, "opencv_world455d.lib")
#else
#pragma comment(lib, "opencv_world455.lib")
#endif

#include <thread>
#include <mutex>
#include <condition_variable>

cv::VideoCapture cap;
cv::Mat raw;

static bool frame_loop = true;
static bool frame_ready = false;
cv::Mat frame;
std::thread frame_thread;
std::mutex mtx;

void errorCallback(int code, const char* msg) {
	printf("error:%d msg:%s\n", code, msg);
}

void retrieveFrameRun() {
	if (!cap.open(0))
		return;

	while (frame_loop) {
		if (cap.grab() && cap.retrieve(raw)) {
			mtx.lock();
			//raw.copyTo(frame);
			cv::cvtColor(raw, frame, cv::COLOR_BGR2RGBA);
			mtx.unlock();
			frame_ready = true;
		}

		auto key = cv::waitKey(1);
		if (key == 'c')
			break;
	}
}

int main() {
	frame_thread = std::thread(retrieveFrameRun);

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
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	bool is_show = true;

	/* loop until user close window */
	while (glfwWindowShouldClose(window) == 0) {

		/* poll for and process event */
		glfwPollEvents();
		/* render after clear */
		glClear(GL_COLOR_BUFFER_BIT);

		if (frame_ready) {
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("imgui image", &is_show);
			mtx.lock();
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.cols, frame.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame.data);
			ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(texture)), ImVec2(frame.cols, frame.rows));
			mtx.unlock();
			ImGui::End();
			//TODO: black screen
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		/* swap buffer */
		glfwSwapBuffers(window);
	}

	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	frame_loop = false;
	if (frame_thread.joinable())
		frame_thread.join();

	return 0;
}