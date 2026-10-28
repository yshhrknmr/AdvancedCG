#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "ScreenshotSaver.h"
#include <iostream>
#include <vector>
#undef _UNICODE
#include <IL/il.h>

using namespace std;

bool SaveScreenshot(const char* filename)
{
	if (!filename) return false;

	// read the whole framebuffer of the current window
	int w, h;
	glfwGetFramebufferSize(glfwGetCurrentContext(), &w, &h);
	if (w <= 0 || h <= 0) return false;

	vector<unsigned char> pixels(w * h * 3);

	glReadBuffer(GL_BACK);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

	ILuint imgName;
	ilGenImages(1, &imgName);
	ilBindImage(imgName);

	// glReadPixels() returns the bottom row first
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	ilEnable(IL_FILE_OVERWRITE);

	ILboolean ok = ilTexImage(w, h, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, pixels.data());
	if (ok) ok = ilSaveImage(filename);

	ilDeleteImages(1, &imgName);

	if (ok)
		cerr << __FUNCTION__ << ": saved " << filename << " (" << w << "x" << h << ")" << endl;
	else
		cerr << __FUNCTION__ << ": Error: cannot save " << filename << endl;

	return ok == IL_TRUE;
}
