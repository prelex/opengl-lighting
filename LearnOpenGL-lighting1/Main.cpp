#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.h"
#include <stb-master/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
#include "Model.h"

void processInput(GLFWwindow* window);

// Callback functions. Called whenever the user changes the window size, uses the mouse, or uses the scroll wheel
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

unsigned int &loadTexture(const char* name, bool gammaCorrection);
unsigned int loadCubemap(std::vector<std::string> faces);

const unsigned int screenWidth = 800, screenHeight = 600;

// Used to make camera speed independent of framerate
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Used to smooth initial mouse input
float lastX = screenWidth / 2.0f, lastY = screenHeight / 2.0f;
bool firstMouse = true;

// set up the camera
Camera camera(glm::vec3(0.0f, 2.0f, 20.0f));

// Position of lamp
glm::vec3 lampPos(0.0f, 5.0f, 10.0f);

// Position of the nanosuit model
glm::vec3 modelPos(0.0f, -0.5f, 4.0f);

int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// glfw: create window
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// load appropriate vertex and fragment shaders, and create shader programs
	Shader lightingShader("shaders/lightingVertexShader.txt", "shaders/lightingFragmentShader.txt");
	Shader lampShader("shaders/lampVertexShader.txt", "shaders/lampFragmentShader.txt");
	Shader modelShader("shaders/modelVertexShader.txt", "shaders/modelFragmentShader.txt");
	Shader skyboxShader("shaders/skybox_vs.txt", "shaders/skybox_fs.txt");
	Shader depthShader("shaders/depthMap_vs.txt", "shaders/depthMap_fs.txt", "shaders/depthMap_gs.txt");

	// cube vertices
	// defined in the appropriate order for face culling
	float cubeVertices[] = {
	// position           // normals		   // texture coords
	// back face
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // top-left
	 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // bottom-left
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // top-left
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // bottom-right
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, // top-right
	 // front face					  		 			   
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // bottom-left
	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // top-right
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // top-right
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f, // top-left
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // bottom-left
	 // left face					  					   
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-right
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top-left
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-left
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-left
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom-right
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-right
	// right face									   
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-left
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top-right
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-right
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-left
	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom-left
	 // bottom face										   
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // top-right
 	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, // top-left
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // bottom-left
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // bottom-left
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f, // bottom-right
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // top-right
	// top face					   
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // top-left
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f, // top-right
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, // bottom-right
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // top-left
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f, // bottom-left
	};

	float quadVertices[] = {
		// positions		// normals		  // texture coords
		-0.5f,  0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 0.0f, 1.0f
	};

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// positions of the 15 cubes to spell out "C++"
	glm::vec3 cubePositions[] = {
	 glm::vec3(-6.0f,  2.0f, 0.0f),  // C
	 glm::vec3(-5.5f, 3.25f, 0.0f),  // 
	 glm::vec3(-5.5f, 0.75f, 0.0f),  //  
	 glm::vec3(-4.0f,  3.5f, 0.0f),  // 
	 glm::vec3(-4.0f,  0.5f, 0.0f),  // _______
	 glm::vec3(-2.0f,  2.0f, 0.0f),  // +
	 glm::vec3(-0.5f,  2.0f, 0.0f),  // 
	 glm::vec3( 1.0f,  2.0f, 0.0f),  //
	 glm::vec3(-0.5f,  3.5f, 0.0f),  //
	 glm::vec3(-0.5f,  0.5f, 0.0f),  // _______
	 glm::vec3( 3.0f,  2.0f, 0.0f),  // +
	 glm::vec3( 4.5f,  2.0f, 0.0f),	 // 
	 glm::vec3( 6.0f,  2.0f, 0.0f),	 // 
	 glm::vec3( 4.5f,  3.5f, 0.0f),	 //
	 glm::vec3( 4.5f,  0.5f, 0.0f)	 // 
	};

	// configure the cube's VAO
	unsigned int cubeVBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glBindVertexArray(cubeVAO);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// configure the light's VAO
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBindVertexArray(lightVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// configure the quad's VAO
	unsigned int quadVBO, quadVAO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glBindVertexArray(quadVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// configure the skybox VAO
	unsigned int skyboxVBO, skyboxVAO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glBindVertexArray(skyboxVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	// load textures
	unsigned int cubeDiffuseMap = loadTexture("images/container2.png", true);
	unsigned int cubeSpecularMap = loadTexture("images/container2_specular.png", false);
	unsigned int floorDiffuseMap = loadTexture("images/wood.jpg", true);
	unsigned int floorSpecularMap = loadTexture("images/wood_specular.jpg", false);
	unsigned int wallTexture = loadTexture("images/wall_tex.jpg", true);

	std::vector<std::string> faces
	{ "skybox/right.jpg", "skybox/left.jpg", "skybox/top.jpg", "skybox/bottom.jpg", "skybox/front.jpg", "skybox/back.jpg" };
	unsigned int skyboxTexture = loadCubemap(faces);
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	Model nanosuitModel("objects/nanosuit/nanosuit.obj", true);

	// bind uniform block to binding point 0
	unsigned int uniformBlockIndexLamp = glGetUniformBlockIndex(lampShader.ID, "Matrices");
	unsigned int uniformBlockIndexLighting = glGetUniformBlockIndex(lightingShader.ID, "Matrices");
	unsigned int uniformBlockIndexModel = glGetUniformBlockIndex(modelShader.ID, "Matrices");
	glUniformBlockBinding(lampShader.ID, uniformBlockIndexLamp, 0);
	glUniformBlockBinding(lightingShader.ID, uniformBlockIndexLighting, 0);
	glUniformBlockBinding(modelShader.ID, uniformBlockIndexModel, 0);

	// create uniform buffer object
	unsigned int uboMatrices;
	glGenBuffers(1, &uboMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// create depth map framebuffer
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth cubemap
	unsigned int depthCubemap;
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	for (unsigned int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_CULL_FACE);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		lampPos.x = 5 * cos(glfwGetTime());

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// create light space transformation matrices
		float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
		float near = 1.0f;
		float far = 35.0f;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj * glm::lookAt(lampPos, lampPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lampPos, lampPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lampPos, lampPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lampPos, lampPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lampPos, lampPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lampPos, lampPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

		// render to depth cubemap
		// ----------------------
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		depthShader.use();
		for (unsigned int i = 0; i < 6; i++)
			depthShader.setMat4("shadowTransforms[" + std::to_string(i) + "]", shadowTransforms[i]);
		depthShader.setFloat("far_plane", far);
		depthShader.setVec3("lightPos", lampPos);
		// render cube objects
		glBindVertexArray(cubeVAO);
		for (unsigned int i = 0; i < 15; ++i)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 50.0f * glfwGetTime();
			model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
			depthShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		//// render floor
		//glBindVertexArray(quadVAO);
		//model = glm::translate(model, glm::vec3(0.0f, -0.5f, 10.0f));
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(20.0f, 30.0f, 1.0f));
		//depthShader.setMat4("model", model);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		//// left wall
		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(-10.0f, 2.5f, 10.0f));
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(30.0f, 10.0f, 1.0f));
		//depthShader.setMat4("model", model);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		//// right wall
		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(10.0f, 2.5f, 10.0f));
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(30.0f, 10.0f, 1.0f));
		//depthShader.setMat4("model", model);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		//// front wall
		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(0.0f, 2.5f, -5.0f));
		//model = glm::scale(model, glm::vec3(30.0f, 10.0f, 1.0f));
		//depthShader.setMat4("model", model);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		//// rear wall
		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(0.0f, 2.5f, 25.0f));
		//model = glm::scale(model, glm::vec3(30.0f, 10.0f, 1.0f));
		//model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//depthShader.setMat4("model", model);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		// render nanosuit model
		model = glm::mat4(1.0f);
		model = glm::translate(model, modelPos);
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		depthShader.setMat4("model", model);
		nanosuitModel.Draw(depthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// clear the color buffer and depth buffer
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// activate the lighting shader program
		lightingShader.use();
		lightingShader.setBool("gamma", true);
		lightingShader.setFloat("scaleS", 1.0f);
		lightingShader.setFloat("scaleT", 1.0f);
		lightingShader.setInt("depthMap", 2);
		lightingShader.setVec3("viewPos", camera.Position);
		lightingShader.setFloat("material.shininess", 64.0f);
		lightingShader.setInt("material.diffuse", 0);
		lightingShader.setInt("material.specular", 1);
		// point light
		lightingShader.setVec3("pointLight.position", lampPos);
		lightingShader.setVec3("pointLight.ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLight.diffuse", 0.8f, 0.66f, 0.41f);
		lightingShader.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLight.constant", 1.0f);
		lightingShader.setFloat("pointLight.linear", 0.14f);
		lightingShader.setFloat("pointLight.quadratic", 0.07f);
		lightingShader.setFloat("far_plane", far);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		
		//// spotlight
		//lightingShader.setVec3("spotLight.position", camera.Position);
		//lightingShader.setVec3("spotLight.direction", camera.Front);
		//lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		//lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		//lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		//lightingShader.setFloat("spotLight.constant", 1.0f);
		//lightingShader.setFloat("spotLight.linear", 0.09);
		//lightingShader.setFloat("spotLight.quadratic", 0.032);
		//lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		//lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
		
		// world transformation (set to identity matrix initially)
		model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);

		// render cube objects
		//----------------
		glBindVertexArray(cubeVAO);
		// bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeDiffuseMap);
		// bind specular map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, cubeSpecularMap);
		for (unsigned int i = 0; i < 15; ++i)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 50.0f * glfwGetTime();
			model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
			lightingShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// render floor
		//--------------
		glBindVertexArray(quadVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorDiffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, floorSpecularMap);
		model = glm::translate(model, glm::vec3(0.0f, -0.5f, 10.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(20.0f, 30.0f, 1.0f));
		lightingShader.setFloat("scaleS", 5.0f);
		lightingShader.setFloat("scaleT", 7.5f);
		lightingShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// render walls
		//----------
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wallTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0); // no specular map for walls
		// left wall
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-10.0f, 2.5f, 10.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 10.0f, 1.0f));
		lightingShader.setFloat("scaleS", 7.5f);
		lightingShader.setFloat("scaleT", 5.0f);
		lightingShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// right wall
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(10.0f, 2.5f, 10.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 10.0f, 1.0f));
		lightingShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// front wall
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 2.5f, -5.0f));
		model = glm::scale(model, glm::vec3(30.0f, 10.0f, 1.0f));
		lightingShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// rear wall
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 2.5f, 25.0f));
		model = glm::scale(model, glm::vec3(30.0f, 10.0f, 1.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lightingShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// render nanosuit model
		lightingShader.setFloat("scaleS", 1.0f);
		lightingShader.setFloat("scaleT", 1.0f);
		model = glm::mat4(1.0f);
		model = glm::translate(model, modelPos);
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		lightingShader.setMat4("model", model);
		nanosuitModel.Draw(lightingShader);

		// render light source
		lampShader.use();
		model = glm::mat4(1.0f);
		model = glm::translate(model, lampPos);
		model = glm::scale(model, glm::vec3(0.2f)); 
		lampShader.setMat4("model", model);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// render skybox
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		skyboxShader.setMat4("projection", projection);
		view = glm::mat4(glm::mat3(camera.getViewMatrix()));
		skyboxShader.setMat4("view", view);
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		// swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	float cameraSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos;
	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}

unsigned int &loadTexture(const char* name, bool gammaCorrection)
{
	unsigned int id;
	glGenTextures(1, &id);
	// load and generate texture
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(name, &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum dataFormat;
		GLenum internalFormat;
		if (nrChannels == 3)
		{
			dataFormat = GL_RGB;
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
		}
		if (nrChannels == 4)
		{
			dataFormat = GL_RGBA;
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set the texture filtering options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	return id;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	stbi_set_flip_vertically_on_load(false);
	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}