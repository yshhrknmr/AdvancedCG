#pragma once

// Saves the current contents of the back buffer (the rendered scene) as an image file.
// Call this after drawing the scene and before rendering ImGui so that the GUI is not captured.
// The format is determined by the file extension (e.g., .png, .jpg, .bmp).
bool SaveScreenshot(const char* filename);
