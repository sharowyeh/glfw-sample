#include <stdio.h>

//#include "Window.hpp"
#include "MatWidget.h"
#include "Camera.h"

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

void errorCallback(int code, const char* msg) {
	printf("error:%d msg:%s\n", code, msg);
}

GLUI::MatWidget *cap_widget;

/* cap ctrl UI properties */
bool is_cap_opened = false;
const char* cap_open_text = "Open";
const char* cap_show_text = "Hide";
const char* cap_draw_text = "Undraw";
/* cap ctrl draw */
void cap_ctrl_widget_Render() {
	ImGui::Begin("Camera Control", NULL, ImGuiWindowFlags_NoCollapse);

	if (ImGui::Button(cap_open_text)) {
		if (is_cap_opened) {
			Camera::CloseCapture();
			is_cap_opened = false;
			cap_open_text = "Open";
		}
		else {
			is_cap_opened = Camera::OpenCapture(0);
			cap_open_text = is_cap_opened ? "Close Camera" : "Failed";
		}
	}

	if (ImGui::Button(cap_show_text)) {
		cap_widget->IsShow = !cap_widget->IsShow;
		cap_show_text = cap_widget->IsShow ? "Hide" : "Show";
	}
	ImGui::SameLine();
	if (ImGui::Button(cap_draw_text)) {
		cap_widget->IsDraw = !cap_widget->IsDraw;
		cap_draw_text = cap_widget->IsDraw ? "Undraw" : "Draw";
	}
}

int main() {
	GLFWwindow* window;
	
	glfwSetErrorCallback(errorCallback);

	/* init framework */
	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(1280, 720, "GLFW", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	/* make context current */
	glfwMakeContextCurrent(window);
	/* for vsync */
	glfwSwapInterval(1);
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");


	cap_widget = new GLUI::MatWidget("Capture", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	Camera::SetRawWidget(cap_widget);

	/* loop until user close window */
	while (glfwWindowShouldClose(window) == 0) {

		/* poll for and process event */
		glfwPollEvents();
		/* render after clear */
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (cap_widget->IsShow) {
			cap_widget->Render();
		}

		ImGui::SetNextWindowPos(ImVec2(670, 0));
		cap_ctrl_widget_Render();
		
		ImGui::Text("average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		/* swap buffer */
		glfwSwapBuffers(window);
	}

	Camera::CloseCapture();

	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	return 0;
}