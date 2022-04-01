#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

/* require opengl dependencies from operation system */
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

/* require GLEW_STATIC */
#ifdef _DEBUG
#pragma comment(lib, "./Debug/libglew32d.lib")
#else
#pragma comment(lib, "./Release/libglew32.lib")
#endif

#ifdef _DEBUG
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "glfw3.lib")
#else
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3_mt.lib")
#endif

#include <mutex>

namespace GLUI {
	class Window {
	private:
		/* inline static keywords combination require c++17 */
		inline static Window* s_Window = nullptr; // static this instance
		inline static std::mutex s_MtxGetInstance;
	public:
		void Destroy();

		static Window* Create(const char* title, int width, int height);
		static Window* Get();

		/*
		* poll window events
		* \return bool isWindowShouldClose?
		*/
		bool PollEvents();
		void SwapWindow();

		void SetWindowOpacity(float value);

		GLFWwindow* GetGlfwWindow();
		void GetWindowSize(int* width, int* height);
		double GetDeltaTime() const;

	private:
		Window(const char* title, int width, int height);

	public:
		// callback without GLFWwindow
		void(*OnFrameResizedCallback)(int width, int height) = nullptr;
		void(*OnScrollCallback)(double xoffset, double yoffset) = nullptr;
		void(*OnCursorPosCallback)(bool pressed, double xpos, double ypos) = nullptr;

		float MaxFPS = 60.0f;

	private:
		GLFWwindow* m_GlfwWindow = nullptr;

		double m_CurrentFrame = 0.0;
		double m_LastFrame = 0.0;
		double m_DeltaTime = 0.02;	//seconds
	};
}
