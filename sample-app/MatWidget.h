#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32
/* for 2D texture */
#include <GL/glew.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <opencv2/opencv.hpp>
#ifdef _DEBUG
#pragma comment(lib, "opencv_world455d.lib")
#else
#pragma comment(lib, "opencv_world455.lib")
#endif

#include <mutex>

namespace GLUI {

	struct Roi {
		float x = 0;
		float y = 0;
		float width = 0;
		float height = 0;
		std::string id;
	};

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
		std::vector<Roi> rois;
	public:
		bool IsShow = true;
		bool IsDraw = true;
	public:
		MatWidget(const char* name, ImGuiWindowFlags flags);

		/* code: cv::ColorConversionCodes, transform to rgba for opengl texture */
		void Update(cv::Mat mat, int code);

		ImVec2 handle_a_pos = ImVec2(100, 30);
		ImVec2 handle_b_pos = ImVec2(30, 100);

		void Render();
	};
}