#pragma once
#include <string>
#include <vector>
#include <thread>
#include "MatWidget.h"

namespace Camera {
	void retrieveFrameRun(void* arg);
	bool OpenCapture(int cap_index);
	void CloseCapture();
	void SetRawWidget(GLUI::MatWidget* widget);
};