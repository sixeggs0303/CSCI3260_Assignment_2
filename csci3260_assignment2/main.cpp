/*******************
FILE : main.cpp (csci3260 2019-2020 Assignment 2)
*******************/
/*******************
Student Information
Student ID:	1155094761
Student Name: Luk Ming Ho
*******************/

#include "Dependencies/glew/glew.h"
#include "Dependencies/freeglut/freeglut.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/gtc/type_ptr.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "Dependencies/stb_image/stb_image.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
using namespace std;
using glm::vec3;
using glm::mat4;

GLint programID;
// Could define the Vao&Vbo and interaction parameter here

//a series utilities for setting shader parameters 
void setMat4(const std::string& name, glm::mat4& value)
{
	unsigned int transformLoc = glGetUniformLocation(programID, name.c_str());
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(value));
}
void setVec4(const std::string& name, glm::vec4 value)
{
	glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setVec3(const std::string& name, glm::vec3 value)
{
	glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setFloat(const std::string& name, float value)
{
	glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}
void setInt(const std::string& name, int value)
{
	glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(programID);
}

int useTexture = 1;
int mouseX;
int mouseY;

float c_time;
float diffuseBrightness = 0.2f;
float zoom = 45.0f;
float catRotateDirection;
float moveSpeed = 0.5f;
float mouseDeltaX;
float mouseDeltaY;

bool mouseLeftDown = false;
bool runningLight = true;

vec3 catMoveDirection;

void mouse_callback(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		mouseX = x;
		mouseY = y;
	}
}

vec3 cameraFocus(0.0f);
float rotateDegreeX = 0.0f;
float rotateDegreeY = 0.0f;

float cameraMoveX;
float cameraMoveY;
void motion_callback(int x, int y)
{
	mouseDeltaX = (x - mouseX) * 0.1f;
	mouseDeltaY = (y - mouseY) * 0.1f;
	mouseX = x;
	mouseY = y;
	cameraMoveX -= mouseDeltaX;
	cameraMoveY -= mouseDeltaY;
}

void keyboard_callback(unsigned char key, int x, int y)
{
	if (key == 'w') 
	{ 
		diffuseBrightness = diffuseBrightness + 0.1f;  
		if (diffuseBrightness > 1)
		{
			diffuseBrightness = 1;
		}
	};
	if (key == 's') { 
		diffuseBrightness = diffuseBrightness - 0.1f;  
		if (diffuseBrightness < 0) 
		{ 
			diffuseBrightness = 0;
		}; 
	};
	if (key == '1') { useTexture = 1; };
	if (key == '2') { useTexture = 2; };
	if (key == 'p') { runningLight = !runningLight; };
}

void special_callback(int key, int x, int y)
{
	if (key == GLUT_KEY_UP) { catMoveDirection = catMoveDirection + vec3(moveSpeed * cos(glm::radians(catRotateDirection - 90.0f)), moveSpeed * sin(glm::radians(catRotateDirection - 90.0f)), 0.0f); };
	if (key == GLUT_KEY_DOWN) { catMoveDirection = catMoveDirection - vec3(moveSpeed * cos(glm::radians(catRotateDirection - 90.0f)), moveSpeed * sin(glm::radians(catRotateDirection - 90.0f)), 0.0f); };
	if (key == GLUT_KEY_LEFT) { catRotateDirection = catRotateDirection + 2.0f; };
	if (key == GLUT_KEY_RIGHT) { catRotateDirection = catRotateDirection - 2.0f; };
}

void mouseWheel_callback(int wheel, int direction, int x, int y)
{
	if (direction > 0) {
		zoom += 1.0f;
	}
	else {
		zoom -= 1.0f;
	}
}


// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}

	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}

			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
}

GLuint loadTexture(const char* texturePath)
{
	// tell stb_image.h to flip loaded texture's on the y-axis.
	stbi_set_flip_vertically_on_load(true);
	// load the texture data into "data"
	int Width, Height, BPP;
	unsigned char* data = stbi_load(texturePath, &Width, &Height, &BPP, 0);
	// Please pay attention to the format when sending the data to GPU
	GLenum format = 3;
	switch (BPP) {
	case 1: format = GL_RED; break;
	case 3: format = GL_RGB; break;
	case 4: format = GL_RGBA; break;
	}
	if (!data) {
		std::cout << "Failed to load texture: " << texturePath << std::endl;
		exit(1);
	}

	GLuint textureID = 0;
	//TODO: Create one OpenGL texture and set the texture parameter 
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
	std::cout << "Load " << texturePath << " successfully!" << std::endl;
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

Model obj_cat, obj_floor, obj_dog, obj_sofa, obj_cabinet;
GLuint cat_vao, cat_vbo, cat_ebo;
GLuint floor_vao, floor_vbo, floor_ebo;
GLuint dog_vao, dog_vbo, dog_ebo;
GLuint sofa_vao, sofa_vbo, sofa_ebo;
GLuint cabinet_vao, cabinet_vbo, cabinet_ebo;
GLuint cat_texture[2];
GLuint floor_texture[2];
GLuint dog_texture[2];
GLuint sofa_texture[2];
GLuint cabinet_texture[2];

void bindCat() 
{
	cat_texture[0] = loadTexture("resources/cat/cat_01.jpg");
	cat_texture[1] = loadTexture("resources/cat/cat_02.jpg");
	obj_cat = loadOBJ("resources/cat/cat.obj");

	glGenVertexArrays(1, &cat_vao);
	glBindVertexArray(cat_vao);

	glGenBuffers(1, &cat_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cat_vbo);
	glBufferData(GL_ARRAY_BUFFER, obj_cat.vertices.size() * sizeof(Vertex), &obj_cat.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &cat_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cat_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj_cat.indices.size() * sizeof(unsigned int), &obj_cat.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

}

void bindFloor()
{
	floor_texture[0] = loadTexture("resources/floor/floor_diff.jpg");
	floor_texture[1] = loadTexture("resources/floor/floor_spec.jpg");

	obj_floor = loadOBJ("resources/floor/floor.obj");
	glGenVertexArrays(1, &floor_vao);
	glBindVertexArray(floor_vao);

	glGenBuffers(1, &floor_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, floor_vbo);
	glBufferData(GL_ARRAY_BUFFER, obj_floor.vertices.size() * sizeof(Vertex), &obj_floor.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &floor_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floor_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj_floor.indices.size() * sizeof(unsigned int), &obj_floor.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

}

void bindDog()
{
	dog_texture[0] = loadTexture("resources/dog/dog_1.jpg");
	dog_texture[1] = loadTexture("resources/dog/dog_2.jpg");
	obj_dog = loadOBJ("resources/dog/dog.obj");

	glGenVertexArrays(1, &dog_vao);
	glBindVertexArray(dog_vao);

	glGenBuffers(1, &dog_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, dog_vbo);
	glBufferData(GL_ARRAY_BUFFER, obj_dog.vertices.size() * sizeof(Vertex), &obj_dog.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &dog_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dog_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj_dog.indices.size() * sizeof(unsigned int), &obj_dog.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

}

void bindSofa()
{
	sofa_texture[0] = loadTexture("resources/sofa/sofa_1.jpg");
	sofa_texture[1] = loadTexture("resources/sofa/sofa_2.png");
	obj_sofa = loadOBJ("resources/sofa/sofa.obj");

	glGenVertexArrays(1, &sofa_vao);
	glBindVertexArray(sofa_vao);

	glGenBuffers(1, &sofa_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, sofa_vbo);
	glBufferData(GL_ARRAY_BUFFER, obj_sofa.vertices.size() * sizeof(Vertex), &obj_sofa.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &sofa_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sofa_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj_sofa.indices.size() * sizeof(unsigned int), &obj_sofa.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

}

void bindCabinet()
{
	cabinet_texture[0] = loadTexture("resources/cabinet/cabinet_1.jpg");
	cabinet_texture[1] = loadTexture("resources/cabinet/cabinet_2.jpg");
	obj_cabinet = loadOBJ("resources/cabinet/cabinet.obj");

	glGenVertexArrays(1, &cabinet_vao);
	glBindVertexArray(cabinet_vao);

	glGenBuffers(1, &cabinet_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cabinet_vbo);
	glBufferData(GL_ARRAY_BUFFER, obj_cabinet.vertices.size() * sizeof(Vertex), &obj_cabinet.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &cabinet_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cabinet_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj_cabinet.indices.size() * sizeof(unsigned int), &obj_cabinet.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

}

void sendDataToOpenGL()
{
	bindCat();
	bindFloor();
	bindDog();
	bindSofa();
	bindCabinet();
}

vec3 cameraPos;

void updateCamera()
{
	GLint projectionMatrixUniformLocation;
	GLint cameraMatrixUniformLocation;
	mat4 cameraMatrix;
	mat4 projectionMatrix;

	cameraPos = vec3(-15.0f, 10.0f, 0.0f);
	cameraMatrix = glm::lookAt(cameraPos, cameraFocus, vec3(0.0f, 1.0f, 0.0f));
	cameraMatrix = translate(cameraMatrix, vec3(0.0f, cameraMoveY , -cameraMoveX));

	cameraMatrixUniformLocation = glGetUniformLocation(programID, "cameraMatrix");
	glUniformMatrix4fv(cameraMatrixUniformLocation, 1, GL_FALSE, &cameraMatrix[0][0]);

	projectionMatrix = glm::perspective(glm::radians(45.0f + zoom), 1.5f, 1.0f, 100.0f);
	projectionMatrixUniformLocation = glGetUniformLocation(programID, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}

void ambientLight()
{
	GLint ambientLightingUniformLocation = glGetUniformLocation(programID, "ambientLight");
	float ambientLight = 0.5f;
	glUniform1f(ambientLightingUniformLocation, ambientLight);
}

void diffuseLight()
{
	float delta_time = 0.1f;
	if (runningLight) {
		c_time += delta_time;
	}
	float speed = 0.001f;
	vec3 lightPosition1, lightPosition2, lightPosition3;
	GLint directionalLightBrightness = glGetUniformLocation(programID, "directionalLightBrightness");
	glUniform1f(directionalLightBrightness, diffuseBrightness);

	vec3 color1(1.0f,0.0f,0.0f);
	GLint lightColor1 = glGetUniformLocation(programID, "diffuseLightColor1");
	glUniform3fv(lightColor1, 1, &color1[0]);

	GLint lightPositionUniformLocation1 = glGetUniformLocation(programID, "lightPositionWorld1");
	lightPosition1 = vec3(20.0f * sin(c_time * speed), 20.0f, 20.0f * cos(c_time * speed));
	glUniform3fv(lightPositionUniformLocation1, 1, &lightPosition1[0]);

	vec3 color2(0.0f, 1.0f, 0.0f);
	GLint lightColor2 = glGetUniformLocation(programID, "diffuseLightColor2");
	glUniform3fv(lightColor2, 1, &color2[0]);

	GLint lightPositionUniformLocation2 = glGetUniformLocation(programID, "lightPositionWorld2");
	lightPosition2 = vec3(20.0f* sin(c_time * speed + glm::radians(120.0f)), 20.0f, 20.0f*cos(c_time * speed + glm::radians(120.0f)));
	glUniform3fv(lightPositionUniformLocation2, 1, &lightPosition2[0]);

	vec3 color3(0.0f, 0.0f, 1.0f);
	GLint lightColor3 = glGetUniformLocation(programID, "diffuseLightColor3");
	glUniform3fv(lightColor3, 1, &color3[0]);

	GLint lightPositionUniformLocation3 = glGetUniformLocation(programID, "lightPositionWorld3");
	lightPosition3 = vec3(20.0f * sin(c_time * speed + glm::radians(240.0f)), 20.0f, 20.0f * cos(c_time * speed + glm::radians(240.0f)));
	glUniform3fv(lightPositionUniformLocation3, 1, &lightPosition3[0]);
}

void specularLight()
{
	GLint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePositionWorld");
	glUniform3fv(eyePositionUniformLocation, 1, &cameraPos[0]);
}

void drawCat()
{
	GLint modelTransformMatrixUniformLocation;
	mat4 modelTransformMatrix;
	GLuint slot;
	GLuint TextureID;

	slot = 0;
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0 + slot);
	if (useTexture == 1) {
		glBindTexture(GL_TEXTURE_2D, cat_texture[0]);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, cat_texture[1]);
	}
	glUniform1i(TextureID, slot);

	glBindVertexArray(cat_vao);
	modelTransformMatrix = translate(mat4(1.0), vec3(0.0f, -1.0f, 0.0f));
	modelTransformMatrix = rotate(modelTransformMatrix, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
	modelTransformMatrix = rotate(modelTransformMatrix, glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
	modelTransformMatrix = rotate(modelTransformMatrix, glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
	modelTransformMatrix = modelTransformMatrix * glm::scale(mat4(1.0f), vec3(0.2f, 0.2f, 0.2f));

	modelTransformMatrix = translate(modelTransformMatrix, catMoveDirection);
	modelTransformMatrix = rotate(modelTransformMatrix, glm::radians(catRotateDirection), vec3(0.0f, 0.0f, 1.0f));

	modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, obj_cat.indices.size(), GL_UNSIGNED_INT, 0);
}

void drawFloor()
{
	GLint modelTransformMatrixUniformLocation;
	mat4 modelTransformMatrix;
	GLuint slot;
	GLuint TextureID;

	slot = 0;
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0 + slot);
	if (useTexture == 1) {
		glBindTexture(GL_TEXTURE_2D, floor_texture[0]);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, floor_texture[1]);
	}
	glUniform1i(TextureID, slot);

	glBindVertexArray(floor_vao);
	modelTransformMatrix = rotate(mat4(1.0f), 0.0f, vec3(1.0f, 0.0f, 0.0f));
	modelTransformMatrix = scale(modelTransformMatrix, vec3(5.0f, 1.0f, 5.0f));
	modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, obj_floor.indices.size(), GL_UNSIGNED_INT, 0);
}

void drawDog()
{
	GLint modelTransformMatrixUniformLocation;
	mat4 modelTransformMatrix;
	GLuint slot;
	GLuint TextureID;

	slot = 0;
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0 + slot);
	if (useTexture == 1) {
		glBindTexture(GL_TEXTURE_2D, dog_texture[0]);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, dog_texture[1]);
	}
	glUniform1i(TextureID, slot);

	glBindVertexArray(dog_vao);
	modelTransformMatrix = translate(mat4(1.0), vec3(-5.0f, 2.8f, -10.0f));
	modelTransformMatrix = rotate(modelTransformMatrix, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
	modelTransformMatrix = rotate(modelTransformMatrix, glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
	modelTransformMatrix = rotate(modelTransformMatrix, sin(glm::radians(c_time)*0.4f)*0.5f + glm::radians(135.0f), vec3(0.0f, 0.0f, 1.0f));
	modelTransformMatrix = modelTransformMatrix * glm::scale(mat4(1.0f), vec3(0.2f, 0.2f, 0.2f));

	modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, obj_dog.indices.size(), GL_UNSIGNED_INT, 0);
}

void drawSofa()
{
	GLint modelTransformMatrixUniformLocation;
	mat4 modelTransformMatrix;
	GLuint slot;
	GLuint TextureID;

	slot = 0;
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0 + slot);
	if (useTexture == 1) {
		glBindTexture(GL_TEXTURE_2D, sofa_texture[0]);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, sofa_texture[1]);
	}
	glUniform1i(TextureID, slot);

	glBindVertexArray(sofa_vao);
	modelTransformMatrix = translate(mat4(1.0), vec3(15.0f, 6.5f, 15.0f));
	//modelTransformMatrix = rotate(modelTransformMatrix, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
	modelTransformMatrix = rotate(modelTransformMatrix, glm::radians(-50.0f), vec3(0.0f, 1.0f, 0.0f));
	//modelTransformMatrix = rotate(modelTransformMatrix, glm::radians(135.0f), vec3(0.0f, 0.0f, 1.0f));
	modelTransformMatrix = modelTransformMatrix * glm::scale(mat4(1.0f), vec3(0.3f));

	modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, obj_sofa.indices.size(), GL_UNSIGNED_INT, 0);
}

void drawCabinet()
{
	GLint modelTransformMatrixUniformLocation;
	mat4 modelTransformMatrix;
	GLuint slot;
	GLuint TextureID;

	slot = 0;
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0 + slot);
	if (useTexture == 1) {
		glBindTexture(GL_TEXTURE_2D, cabinet_texture[0]);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, cabinet_texture[1]);
	}
	glUniform1i(TextureID, slot);

	glBindVertexArray(cabinet_vao);
	modelTransformMatrix = translate(mat4(1.0), vec3(15.0f, -1.0f, -30.0f));
	//modelTransformMatrix = rotate(modelTransformMatrix, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
	modelTransformMatrix = rotate(modelTransformMatrix, glm::radians(-50.0f), vec3(0.0f, 1.0f, 0.0f));
	//modelTransformMatrix = rotate(modelTransformMatrix, glm::radians(135.0f), vec3(0.0f, 0.0f, 1.0f));
	modelTransformMatrix =  glm::scale(modelTransformMatrix, vec3(1.0f));

	modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, obj_cabinet.indices.size(), GL_UNSIGNED_INT, 0);
}

void paintGL(void)
{
	glClearColor(0.3f, 0.2f, 0.2f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLineWidth(2.0f);

	updateCamera();
	ambientLight();
	diffuseLight();
	specularLight();
	
	drawCat();
	drawFloor();
	drawDog();
	drawSofa();
	drawCabinet();

	glFlush();
	glutPostRedisplay();
}

void initializedGL(void) //run only once
{
	glewInit();
	installShaders();
	sendDataToOpenGL();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("Assignment 2");

	initializedGL();
	glutDisplayFunc(paintGL);
	glutMouseFunc(mouse_callback);
	glutMotionFunc(motion_callback);
	glutKeyboardFunc(keyboard_callback);
	glutSpecialFunc(special_callback);
	//glutMouseWheelFunc(mouseWheel_callback);

	glutMainLoop();

	return 0;
}
