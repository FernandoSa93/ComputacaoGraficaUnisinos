#include "Mesh.h"

void Mesh::initialize(GLuint VAO, int nVertices, Shader* shader, glm::vec3 position, glm::vec3 scale, float angle, glm::vec3 axis)
{
	this->VAO = VAO;
	this->nVertices = nVertices;
	this->shader = shader;
	this->position = position;
	this->scale = scale;
	this->angle = angle;
	this->axis = axis;
}

void Mesh::update()
{
	glm::mat4 model = glm::mat4(1); //matriz identidade
	model = glm::translate(model, position);
	model = glm::rotate(model, angle, axis);
	model = glm::scale(model, scale);

	// Passar o conteúdo para o shader
	shader->setMat4("model", glm::value_ptr(model));
}

void Mesh::draw()
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, nVertices);
	glBindVertexArray(0);
}

// Getters
glm::vec3 Mesh::getPosition()
{
	return position;
}

float Mesh::getAngle()
{
	return angle;
}

glm::vec3 Mesh::getAxis()
{
	return axis;
}

glm::vec3 Mesh::getScale()
{
	return scale;
}

void Mesh::setPosition(glm::vec3 p)
{
	position = p;
}

void Mesh::setAngle(float a)
{
	angle = a;
}

void Mesh::setAxis(glm::vec3 a)
{
	axis = a;
}

void Mesh::setScale(glm::vec3 s)
{
	scale = s;
}
