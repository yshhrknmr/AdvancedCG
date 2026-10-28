#pragma once

#include "Ray.h"
#include "ImageRect.h"
#include "glm/glm.hpp"
#include "GLSLProgramObject.h"
#include <random>
#include <atomic>

class PathTracer
{
public:
	typedef ImageRect<glm::vec3> ImageRGBf;

	static float s_Gamma;
	static int s_MaxRecursionDepth;
	static int s_MinRecursionDepth;
	static int s_NumSamplesPerPixel;
	static int s_NumSamplesPerUpdate;

	PathTracer() : m_FrameBufferTexID(0), m_pGammaShader(0), m_isNVIDIADriver(false) {}
	~PathTracer()
	{
		if (m_FrameBufferTexID) glDeleteTextures(1, &m_FrameBufferTexID);
		if (m_pGammaShader) delete m_pGammaShader;
	}

	void renderScene();
	void renderFrame();

private:
	ImageRGBf m_FrameBuffer;
	GLuint m_FrameBufferTexID;

	GLSLProgramObject* m_pGammaShader;
	bool m_isNVIDIADriver;

	void initShader();

	glm::vec3 traceRec(const Ray& ray, int recursionDepth);

	void updateFrameBufferTexture();
	void renderIntermediateFrame();

	// local coordinate system at a hit point: yLocal = normal, xLocal and zLocal span the tangent plane
	void calcLocalCoordinateSystem(const glm::vec3& normal, const glm::vec3& rayDir, glm::vec3& xLocal, glm::vec3& yLocal, glm::vec3& zLocal) const;

	// uniform random number in [0, 1)
	// Each thread has its own generator: std::mt19937 must not be shared by the OpenMP threads
	// in renderScene() (sharing it is a data race).
	inline float frand()
	{
		static std::atomic<unsigned int> s_SeedCounter(12345);
		thread_local std::mt19937 randSrc(s_SeedCounter++);
		thread_local std::uniform_real_distribution<float> randDist(0.f, 1.f);
		const float r = randDist(randSrc);
		return (r < 1.f) ? r : 0.99999994f;	// some implementations can return 1.0f
	}
};