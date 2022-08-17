#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32
/* for 2D texture */
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
		Roi drag_roi;
	public:
		bool IsShow = true;
		bool IsDraw = true;
	public:
		MatWidget(const char* name, ImGuiWindowFlags flags) {
			widget_init = false;
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

			for (int i = 0; i < 3; i++) {
				Roi roi;
				roi.x = i * 50;
				roi.width = 30 + (i * 5);
				roi.height = 30 + (i * 5);
				roi.id = std::string("roi") + std::to_string(i);
				rois.push_back(roi);
			}
		};

		/* code: cv::ColorConversionCodes, transform to rgba for opengl texture */
		void Update(cv::Mat mat, int code) {
			mtx.lock();
			cv::cvtColor(mat, image, code);
			mtx.unlock();
		}

		// Gets back to previous line and continue with horizontal layout
		//      offset_from_start_x == 0 : follow right after previous item
		//      offset_from_start_x != 0 : align to specified x position (relative to window/group left)
		//      spacing_w < 0            : use default spacing if pos_x == 0, no spacing if pos_x != 0
		//      spacing_w >= 0           : enforce spacing amount
		void SameLoc(float offset_from_start_x, float offset_from_start_y)
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			if (window->SkipItems)
				return;

			ImGuiContext& g = *GImGui;
			if (offset_from_start_x != 0.0f)
			{
				window->DC.CursorPos.x = window->DC.CursorStartPos.x + offset_from_start_x;
				window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
			}
			else
			{
				window->DC.CursorPos.x = window->DC.CursorStartPos.x;
				window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
			}
			window->DC.CurrLineSize = window->DC.PrevLineSize;
			window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset;
		}

		void RenderROI() {
			
			ImGuiIO& io = ImGui::GetIO();

			for (int i = 0; i < rois.size(); i++) {
				ImGui::PushID(i);
				
				SameLoc(rois[i].x, rois[i].y);
				
				ImGui::BeginChildFrame(ImGui::GetID(rois[i].id.c_str()), ImVec2(rois[i].width, rois[i].height), true);

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None | ImGuiDragDropFlags_SourceAllowNullID))
				{
					// Set payload to carry the index of our item (could be anything)
					ImGui::SetDragDropPayload("ROI_INDEX", &i, sizeof(int));
					drag_roi = rois[i];
					//TODO: save origin x/y to payload?

					// Display preview (could be anything, e.g. when dragging an image we could decide to display
					// the filename and a small preview of the image, etc.)
					ImGui::Text("Move %s pos %g %g", rois[i].id.c_str(), io.MousePos.x, io.MousePos.y);
					ImGui::EndDragDropSource();
				}

				ImGui::EndChildFrame();

				ImGui::PopID();
			}
			if (ImGui::BeginDragDropTarget())
			{
				//TODO: read origin x/y calc offset apply to roi list
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ROI_INDEX"))
				{
					IM_ASSERT(payload->DataSize == sizeof(int));
					int payload_n = *(const int*)payload->Data;
					//std::swap(rois[i], rois[payload_n]);
				}
				ImGui::EndDragDropTarget();
			}
			//SameLoc(0, 0);
			//ImGui::BeginChildFrame(ImGui::GetID("mychild"), ImVec2(40, 40), true);
			//if (ImGui::BeginDragDropSource(/*ImGuiDragDropFlags_SourceAllowNullID*/)) {
			//	auto myid = ImGui::GetID("mychild");
			//	ImGui::SetDragDropPayload("what-is-type-for", &myid, sizeof(ImGuiID));
			//	ImGui::Text("This is a drag and drop source");
			//	ImGui::EndDragDropSource();
			//}
			//ImGui::EndChildFrame();
			//SameLoc(40, 40);
			//ImGui::BeginChildFrame(ImGui::GetID("mychild2"), ImVec2(50, 50), true);
			//ImGui::EndChildFrame();
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
			if (widget_init == false) {
				// reset widget position when first time shows up
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				widget_init = true;
			}
			ImGui::Begin(widget_name, &IsShow, widget_flags);
			ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(texture)), ImVec2(image_width, image_height));
			
			RenderROI();
			ImGui::End();
		}
	};
}