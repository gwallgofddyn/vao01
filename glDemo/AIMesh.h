#pragma once

#include "core.h"

class AIMesh {

	GLuint				numFaces = 0;

	GLuint				meshVertexPosBuffer = 0;
	GLuint				meshTexCoordBuffer = 0;
	GLuint				meshFaceIndexBuffer = 0;

	GLuint				textureID = 0;

public:

	AIMesh(std::string filename, GLuint meshIndex = 0);

	void addTexture(GLuint textureID);
	void addTexture(std::string filename, FREE_IMAGE_FORMAT format);

	void preRender();
	void render();
	void postRender();
};