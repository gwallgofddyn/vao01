
#include "AIMesh.h"
#include "TextureLoader.h"

using namespace std;
using namespace glm;


AIMesh::AIMesh(std::string filename, GLuint meshIndex) {

	const struct aiScene* scene = aiImportFile(filename.c_str(),
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (!scene) {

		return;
	}

	aiMesh* mesh = scene->mMeshes[meshIndex];

	// Setup VBO for vertex position data
	glGenBuffers(1, &meshVertexPosBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshVertexPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(aiVector3D), mesh->mVertices, GL_STATIC_DRAW);

	// Setup VBO for vertex normal data
	glGenBuffers(1, &meshNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(aiVector3D), mesh->mNormals, GL_STATIC_DRAW);

	if (mesh->mTextureCoords && mesh->mTextureCoords[0]) {

		// Setup VBO for texture coordinate data (for now use uvw channel 0 only when accessing mesh->mTextureCoords)
		glGenBuffers(1, &meshTexCoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, meshTexCoordBuffer);
		glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(aiVector3D), mesh->mTextureCoords[0], GL_STATIC_DRAW);
	}
	
	// Setup VBO for mesh index buffer (face index array)

	numFaces = scene->mMeshes[meshIndex]->mNumFaces;

	// Setup contiguous array
	const GLuint numBytes = scene->mMeshes[meshIndex]->mNumFaces * 3 * sizeof(GLuint);
	GLuint* faceIndexArray = (GLuint*)malloc(numBytes);

	GLuint* dstPtr = faceIndexArray;
	for (unsigned int f = 0; f < scene->mMeshes[meshIndex]->mNumFaces; ++f, dstPtr += 3) {

		memcpy_s(dstPtr, 3 * sizeof(GLuint), scene->mMeshes[meshIndex]->mFaces[f].mIndices, 3 * sizeof(GLuint));
	}

	glGenBuffers(1, &meshFaceIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshFaceIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numBytes, faceIndexArray, GL_STATIC_DRAW);

	// Once done, release all resources associated with this import
	aiReleaseImport(scene);
}


// Texture setup methods

void AIMesh::addTexture(GLuint textureID) {

	this->textureID = textureID;
}

void AIMesh::addTexture(std::string filename, FREE_IMAGE_FORMAT format) {

	textureID = loadTexture(filename, format);
}


void AIMesh::preRender() {

	// Setup VBOs ready for rendering
	glBindBuffer(GL_ARRAY_BUFFER, meshVertexPosBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, meshNormalBuffer);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
	glEnableVertexAttribArray(3);

	if (meshTexCoordBuffer != 0) {

		glBindBuffer(GL_ARRAY_BUFFER, meshTexCoordBuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
		glEnableVertexAttribArray(2);

		if (textureID != 0) {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textureID);
		}
	}

	// Bind (and leave bound) the index array for drawing
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshFaceIndexBuffer);
}


void AIMesh::render() {

	glDrawElements(GL_TRIANGLES, numFaces * 3, GL_UNSIGNED_INT, (const GLvoid*)0);
}


void AIMesh::postRender() {

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(3);

	if (meshTexCoordBuffer != 0) {

		glDisableVertexAttribArray(2);

		if (textureID != 0) {
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}