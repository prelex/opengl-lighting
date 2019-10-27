#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

class Shader;

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

class Mesh {
public:
	// Mesh data
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	// Functions
	Mesh(std::vector<Vertex> aVertices, std::vector<unsigned int> aIndices, std::vector<Texture> aTextures);
	void Draw(Shader shader);

private:
	// Render data
	unsigned int VAO, VBO, EBO;

	// Functions
	void setup();
};