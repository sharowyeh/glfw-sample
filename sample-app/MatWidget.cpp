#include "MatWidget.h"
#include "HandleWidget.h"

GLUI::MatWidget::MatWidget(const char* name, ImGuiWindowFlags flags)
{
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
}

void GLUI::MatWidget::Update(cv::Mat mat, int code)
{
	std::lock_guard<std::mutex> lock(mtx);
	cv::cvtColor(mat, image, code);
}

void GLUI::MatWidget::Render()
{
	/* update texture if image is changed */
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (image.empty() == false && IsDraw) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
			if (image_width != image.cols || image_height != image.rows) {
				image_width = image.cols;
				image_height = image.rows;
			}
		}
	}

	/* UI part with opengl texture */
	if (widget_init == false) {
		// reset widget position when first time shows up
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		widget_init = true;
	}
	ImGui::Begin(widget_name, &IsShow, widget_flags);
	ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(texture)), ImVec2(image_width, image_height));

	// drawing ROIs as draggable handle widgets
	for (auto it = rois.begin(); it != rois.end(); it++) {
		if (HandleWidget(it->x, it->y, it->width, it->height, it->id, false)) {

		}
	}

	ImGui::End();
}
