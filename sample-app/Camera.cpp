#include "Camera.h"

#include <opencv2/opencv.hpp>
#ifdef _DEBUG
#pragma comment(lib, "opencv_world455d.lib")
#else
#pragma comment(lib, "opencv_world455.lib")
#endif

#include <mutex>

cv::VideoCapture cap;
cv::Mat cap_raw;
static bool cap_loop = false;
std::thread cap_thread;
GLUI::MatWidget* raw_widget = NULL;

namespace Camera {

void retrieveFrameRun(void* arg) {

	while (cap_loop) {
		if (cap.grab() && cap.retrieve(cap_raw)) {
			if (raw_widget)
				raw_widget->Update(cap_raw, cv::COLOR_BGR2RGBA);
			//widget->Update(cap_raw, cv::COLOR_BGR2RGBA);
		}

		auto key = cv::waitKey(10);
		/*
		if (key == 'c')
			break;*/
	}
}

bool OpenCapture(int cap_index)
{
	if (cap.isOpened())
		return false;

	auto result = cap.open(cap_index);
	if (result) {
		cap_loop = true;
		cap_thread = std::thread(retrieveFrameRun, raw_widget);
	}
	return result;
}

void CloseCapture()
{
	cap_loop = false;
	if (cap_thread.joinable())
		cap_thread.join();
	cap.release();
}

void SetRawWidget(GLUI::MatWidget* widget) {
	raw_widget = widget;
}

}