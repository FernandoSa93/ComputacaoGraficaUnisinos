#pragma once

#include <stb_image.h>

#include <vector>

#include "Mesh.h"

using namespace std;

class Object
{
public:
	Object() {}
	void initialize(string filePath, string texNames[], string MtlPath, Shader* shader, glm::vec3 position = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1), float angle = 0.0, glm::vec3 axis = glm::vec3(0.0, 0.0, 1.0));
	void update();
	void draw();

	glm::vec3 getPosition();
	float getAngle();
	float getKa();
	float getKs();
	float getKd();
	glm::vec3 getAxis();
	glm::vec3 getScale();
	Shader* getShader();
	vector <Mesh> getGrupos();

	void setPosition(glm::vec3 p);
	void setAngle(float a);
	void setKa(float Ka);
	void setKs(float Ks);
	void setKd(float Kd);
	void setAxis(glm::vec3 a);
	void setScale(glm::vec3 s);

protected:
	void loadObj(string filePath, string texNames[]);
	void loadMtl(string filePath);
	int generateTexture(string filePath);
	GLuint generateVAO(vector <GLfloat> vertbuffer, int& nVertices);
	vector <Mesh> grupos;
	//Atributos que armazenam informações para aplicar as transforms no objeto
	glm::vec3 position;
	float angle, ka, ks, kd;
	glm::vec3 axis;
	glm::vec3 scale;
	Shader* shader;

};