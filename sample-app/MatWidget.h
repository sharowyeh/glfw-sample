#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GL/glew.h>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
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

#include <mutex>

namespace GLUI {
	class MatWidget {
	private:
		GLuint texture;
		cv::Mat image;
		int image_width;
		int image_height;
		bool widget_init; // passing default or using ini?
		const char* widget_name;
		ImGuiWindowFlags widget_flags;
		std::mutex mtx;
	public:
		bool IsShow = true;
		bool IsDraw = true;
	public:
		MatWidget(const char* name, ImGuiWindowFlags flags) {
			widget_init = true;
			widget_name = name;
			widget_flags = flags;

			/* must be after ImGui_ImplOpenGL3_Init */
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

			image_width = 0;
			image_height = 0;
		};

		/* code: cv::ColorConversionCodes, transform to rgba for opengl texture */
		void Update(cv::Mat mat, int code) {
			mtx.lock();
			cv::cvtColor(mat, image, code);
			mtx.unlock();
		}

		void Render() {
			/* update texture if image is changed */
			mtx.lock();
			if (image.empty() == false && IsDraw) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
				if (image_width != image.cols || image_height != image.rows) {
					image_width = image.cols;
					image_height = image.rows;
				}
			}
			mtx.unlock();

			/* UI part with opengl texture */
			if (widget_init) {
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				widget_init = false;
			}
			ImGui::Begin(widget_name, &IsShow, widget_flags);
			ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(texture)), ImVec2(image_width, image_height));
			ImGui::End();
		}
	};
}