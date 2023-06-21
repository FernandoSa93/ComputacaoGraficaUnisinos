/* Trabalho GB Computação Gráfica - Unisinos 2023/1
 *
 * Criado por Fernando Sá
 *
 * Comandos do teclado e mouse:
 * # W, A, S, D e movimento do mouse: Movimentação da câmera
 * # Scroll do mouse: Zoom in e zoom out
 * # X, Y, e Z: Rotação da malha selecionada nos eixos correspondentes
 * # 0, 1 e 2(Teclado alfanumérico): Translação da malha selecionada nos eixos correspondentes
 * # TAB: Alterna entre a malha selecionada
 * # + e -(Teclado numérico): Aumentar ou diminuir a escala da malha selecionada
 * # SPACE: Reseta a cena
 *
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#include "Object.h"

//Curvas
#include <random>
#include <algorithm>
#include "Bezier.h"


struct Vertex
{
	glm::vec3 position;
	glm::vec3 v_color;
};

// Funções de callback de teclado, mouse e scroll
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Funções para gerar a curva
vector <glm::vec3> generateControlPointsSet(int nPoints);
vector <glm::vec3> generateControlPointsSet();
std::vector<glm::vec3> generateUnisinosPointsSet();
GLuint generateControlPointsBuffer(vector <glm::vec3> controlPoints);

//Função para montar a cena
void montarCena(Shader* shader);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Variáveis para armazenar a posição e orientação da camera
glm::vec3 cameraPos = glm::vec3(13.8, 3.8, 6.4);
glm::vec3 cameraFront = glm::vec3(-0.8, -0.3, -0.4);
glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);
float cameraSpeed = 0.05;

//Array de posições dos objetos
glm::vec3 posicoesObj[11];

// Variáveis de controle de rotação em x, y, z
bool rotateX = false, rotateY = false, rotateZ = false;

// Variáveis de controle de translação em x, y, z
bool translateX = false, translateY = false, translateZ = false;

// Variáveis de controle de escala das malhas
bool escalaAumentar = false, escalaDiminuir = false;

// Variáveis de seleção de objetos da cena
bool trocarObjetoSelecionado = false;
GLuint ObjID = 0;

// Variáveis de controle de movimento da câmera
bool movimento = false;
bool scroll = false;

// Variáveis de controle de movimentação de câmera com o mouse
bool firstMouse = true;
float lastX = 0.0, lastY = 0.0;
float yaw = -153.05, pitch = -17.75;
float fov = 45.0f;

Object objetos[11];

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Trabalho GB", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	//Desabilita o desenho do cursor do mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Registrar o scroll do mouse
	glfwSetScrollCallback(window, scroll_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	Shader shader("TrabalhoGB.vs", "TrabalhoGB.fs");

	glUseProgram(shader.ID);

	//Definindo a matriz de view (posição e orientação da câmera)
	glm::mat4 view;
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	GLint viewLoc = glGetUniformLocation(shader.ID, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	//Definindo a matriz de projeção perpectiva
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
	GLint projLoc = glGetUniformLocation(shader.ID, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//Definindo as propriedades da fonte de luz
	shader.setVec3("lightPos", -2.0f, 10.0f, 3.0f);

	//Carregar e posicionar todos os objetos
	montarCena(&shader);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shader.ID, "colorBuffer"), 0);

	glEnable(GL_DEPTH_TEST);

	//Definições para criação da curva paramétrica
	//Conjunto de pontos de controle
	std::vector<glm::vec3> controlPoints = generateControlPointsSet();
	GLuint VAO = generateControlPointsBuffer(controlPoints);

	std::vector<glm::vec3> uniPoints = generateUnisinosPointsSet();
	GLuint VAOUni = generateControlPointsBuffer(uniPoints);

	Bezier bezier;
	bezier.setControlPoints(uniPoints);
	bezier.setShader(&shader);
	bezier.generateCurve(10);

	int nbCurvePoints = bezier.getNbCurvePoints();
	int i = 0;

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.6f, 0.6f, 0.6f, 0.6f);  //cor de fundo(Cinza)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		glm::vec3 scale = glm::vec3(1);

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		projection = glm::perspective(glm::radians(fov), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

		if (trocarObjetoSelecionado)
		{
			if (ObjID < (sizeof objetos / sizeof objetos[0]) - 1)
			{
				ObjID++;
			}
			else
			{
				ObjID = 0;
			}

			trocarObjetoSelecionado = false;
		}

		// Rotação em x, y, z
		else if (rotateX)
		{
			objetos[ObjID].setAngle(angle);
			objetos[ObjID].setAxis(glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else if (rotateY)
		{
			objetos[ObjID].setAngle(angle);
			objetos[ObjID].setAxis(glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (rotateZ)
		{
			objetos[ObjID].setAngle(angle);
			objetos[ObjID].setAxis(glm::vec3(0.0f, 0.0f, 1.0f));
		}

		// Translação em x, y, z
		else if (translateX)
		{
			objetos[ObjID].setPosition(glm::vec3(cos(angle) * 10.0, objetos[ObjID].getPosition().y, objetos[ObjID].getPosition().z));
		}
		else if (translateY)
		{
			objetos[ObjID].setPosition(glm::vec3(objetos[ObjID].getPosition().x, cos(angle) * 10.0, objetos[ObjID].getPosition().z));
		}
		else if (translateZ)
		{
			objetos[ObjID].setPosition(glm::vec3(objetos[ObjID].getPosition().x, objetos[ObjID].getPosition().y, cos(angle) * 10.0));
		}

		// Escala
		else if (escalaAumentar)
		{
			scale = objetos[ObjID].getScale();
			scale += glm::vec3(0.05);
			objetos[ObjID].setScale(scale);
			escalaAumentar = false;
		}
		else if (escalaDiminuir)
		{
			scale = objetos[ObjID].getScale();
			scale -= glm::vec3(0.05);
			objetos[ObjID].setScale(scale);
			escalaDiminuir = false;
		}

		//Movimentando a câmera com A, W, S e D e usando o mouse
		else if (movimento) {
			view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		}

		//Scroll do mouse para zoom in e zoom out
		else if (scroll) {
			projection = glm::perspective(glm::radians(fov), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
		}

		// Passar o conteúdo para o shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		//Criando a model para a curva do U da Unisinos e definindo a posição dela na cena
		glm::mat4 model = glm::mat4(1); //matriz identidade
		model = glm::translate(model, glm::vec3(1.0, -1.0, -0.5));
		shader.setMat4("model", glm::value_ptr(model));

		//Desenhando o U
		glBindVertexArray(VAOUni);
		glDrawArrays(GL_LINE_STRIP, 0, uniPoints.size());
		glBindVertexArray(0);

		bezier.drawCurve(glm::vec4(0, 1, 0, 1));

		glm::vec3 pointOnCurve = bezier.getPointOnCurve(i);
		vector <glm::vec3> aux;
		aux.push_back(pointOnCurve);
		GLuint VAOPoint = generateControlPointsBuffer(aux);

		glBindVertexArray(VAOPoint);

		// Desenhando o ponto que percorre o U
		glDrawArrays(GL_POINTS, 0, aux.size());
		glBindVertexArray(0);

		//Variável de controle da posição do ponto
		i = (i + 1) % nbCurvePoints;

		// Chamada de desenho dos objetos da cena
		for (int idObjeto = 0; idObjeto < (sizeof objetos / sizeof objetos[0]); idObjeto++) {

			// Destacar objeto selecionado
			if (idObjeto == ObjID) {
				shader.setVec3("lightColor", 0.8f, 0.8f, 0.0f);
			}
			else {
				shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
			}

			//Definindo as propriedades do material de cada objeto de acordo com o mtl
			shader.setFloat("ka", objetos[idObjeto].getKa());  //Luz ambiente
			shader.setFloat("kd", objetos[idObjeto].getKd());  //Luz difusa
			shader.setFloat("ks", objetos[idObjeto].getKs());  //Luz especular
			shader.setFloat("q", 10);                          //Potência especular

			objetos[idObjeto].update();
			objetos[idObjeto].draw();
		}

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}

	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado 
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// Resetar cena
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		trocarObjetoSelecionado = false;
		escalaAumentar = false;
		escalaDiminuir = false;
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		translateX = false;
		translateY = false;
		translateZ = false;
		movimento = false;
		scroll = false;

		ObjID = 0;

		for (int idObjeto = 0; idObjeto < (sizeof objetos / sizeof objetos[0]); idObjeto++) {
			objetos[idObjeto].setPosition(posicoesObj[idObjeto]);
			objetos[idObjeto].setScale(glm::vec3(1));
			objetos[idObjeto].setAngle(glm::radians(0.0f));
			objetos[idObjeto].setAxis(glm::vec3(0.0, 0.0, 1.0));
		}
	}

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		trocarObjetoSelecionado = false;
		escalaAumentar = false;
		escalaDiminuir = false;
		rotateX = true;
		rotateY = false;
		rotateZ = false;
		translateX = false;
		translateY = false;
		translateZ = false;
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		trocarObjetoSelecionado = false;
		escalaAumentar = false;
		escalaDiminuir = false;
		rotateX = false;
		rotateY = true;
		rotateZ = false;
		translateX = false;
		translateY = false;
		translateZ = false;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		trocarObjetoSelecionado = false;
		escalaAumentar = false;
		escalaDiminuir = false;
		rotateX = false;
		rotateY = false;
		rotateZ = true;
		translateX = false;
		translateY = false;
		translateZ = false;
	}

	if (key == GLFW_KEY_0 && action == GLFW_PRESS)
	{
		trocarObjetoSelecionado = false;
		escalaAumentar = false;
		escalaDiminuir = false;
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		translateX = true;
		translateY = false;
		translateZ = false;
	}

	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		trocarObjetoSelecionado = false;
		escalaAumentar = false;
		escalaDiminuir = false;
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		translateX = false;
		translateY = true;
		translateZ = false;
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		trocarObjetoSelecionado = false;
		escalaAumentar = false;
		escalaDiminuir = false;
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		translateX = false;
		translateY = false;
		translateZ = true;
	}

	if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS)
	{
		trocarObjetoSelecionado = false;
		escalaAumentar = true;
		escalaDiminuir = false;
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		translateX = false;
		translateY = false;
		translateZ = false;
	}

	if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS)
	{
		trocarObjetoSelecionado = false;
		escalaAumentar = false;
		escalaDiminuir = true;
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		translateX = false;
		translateY = false;
		translateZ = false;
	}

	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
	{
		trocarObjetoSelecionado = true;
		escalaAumentar = false;
		escalaDiminuir = false;
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		translateX = false;
		translateY = false;
		translateZ = false;
	}

	if ((key == GLFW_KEY_W) || (key == GLFW_KEY_S) || (key == GLFW_KEY_A) || (key == GLFW_KEY_D)) {
		movimento = true;
	}

	// Alterar posição da câmera quando pressionado W, S, A ou D
	if (key == GLFW_KEY_W)
		cameraPos += cameraSpeed * cameraFront;
	if (key == GLFW_KEY_S)
		cameraPos -= cameraSpeed * cameraFront;
	if (key == GLFW_KEY_A)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (key == GLFW_KEY_D)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// Função de callback do mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	movimento = true;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	// Calcular o deslocamento do mouse de um frame para o outro
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	// Deixando o movimento mais suave
	float sensitivity = 0.05;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	// Adicionar algumas restrições no yaw e no pitch
	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	// Recalcular o vetor direção
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

// Função de callback do scroll
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	scroll = true;

	// Modificar fov
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}

void montarCena(Shader* shader) {
	//Armazenando a posição dos objetos na cena
	posicoesObj[0] = glm::vec3(0.0, 0.0, 0.0);
	posicoesObj[1] = glm::vec3(0.7, 1.0, -3.2);
	posicoesObj[2] = glm::vec3(1.2, -1.8, 0.0);
	posicoesObj[3] = glm::vec3(0.7, -1.8, -2.8);
	posicoesObj[4] = glm::vec3(-3.25, 0.8, 0.0);
	posicoesObj[5] = glm::vec3(-3.0, -0.8, -2.9);
	posicoesObj[6] = glm::vec3(0.5, -1.4, -3.0);
	posicoesObj[7] = glm::vec3(0.27, -1.37, -2.68);
	posicoesObj[8] = glm::vec3(1.3, -1.39, -2.9);
	posicoesObj[9] = glm::vec3(0.5, -1.25, -0.25);
	posicoesObj[10] = glm::vec3(0.0, -1.5, 3.0);


	string texParedes[] = { "../../Cena/Minha cena/Paredes/Texturas/Wood051_2K_Displacement.png" };
	objetos[0].initialize("../../Cena/Minha cena/Paredes/paredes.obj",  //Caminho do objeto
		texParedes,									                    //Caminho da textura
		"../../Cena/Minha cena/Paredes/paredes.mtl",                    //Caminho do mtl
		shader,                                                         //Shader
		posicoesObj[0]);                                                //Posição inicial do obj na cena  

	string texTv[] = { "../../Cena/Minha cena/Tv/Texturas/cyberpunk77.jpeg" };
	objetos[1].initialize("../../Cena/Minha cena/Tv/tv.obj",
		texTv,
		"../../Cena/Minha cena/Tv/tv.mtl",
		shader,
		posicoesObj[1]);

	string texMesa[] = { "../../Cena/Minha cena/Mesa/Texturas/Wood051_2K_Color.png" };
	objetos[2].initialize("../../Cena/Minha cena/Mesa/mesa.obj",
		texMesa,
		"../../Cena/Minha cena/Mesa/mesa.mtl",
		shader,
		posicoesObj[2]);

	string texGabinete[] = { "../../Cena/Minha cena/Gabinete Tv/Texturas/Wood050_2K_Color.jpg" };
	objetos[3].initialize("../../Cena/Minha cena/Gabinete Tv/gabinete.obj",
		texGabinete,
		"../../Cena/Minha cena/Gabinete Tv/gabinete.mtl",
		shader,
		posicoesObj[3]);

	string texQuadro[] = { "../../Cena/Minha cena/Quadro/Texturas/among us.jpg" };
	objetos[4].initialize("../../Cena/Minha cena/Quadro/quadro.obj",
		texQuadro,
		"../../Cena/Minha cena/Quadro/quadro.mtl",
		shader,
		posicoesObj[4]);

	string texGuitarra[] = { "../../Cena/Minha cena/Guitarra/Texturas/corpodif.jpg" };
	objetos[5].initialize("../../Cena/Minha cena/Guitarra/guitarra.obj",
		texGuitarra,
		"../../Cena/Minha cena/Guitarra/guitarra.mtl",
		shader,
		posicoesObj[5]);

	string texPS4[] = { "../../Cena/Minha cena/PS4/Texturas/black.jpg" };
	objetos[6].initialize("../../Cena/Minha cena/PS4/ps4Base.obj",
		texPS4,
		"../../Cena/Minha cena/PS4/ps4Base.mtl",
		shader,
		posicoesObj[6]);
	objetos[7].initialize("../../Cena/Minha cena/PS4/ps4Topo.obj",
		texPS4,
		"../../Cena/Minha cena/PS4/ps4Topo.mtl",
		shader,
		posicoesObj[7]);

	string texControleRemoto[] = { "../../Cena/Minha cena/Controle Remoto/Texturas/dif.jpg" };
	objetos[8].initialize("../../Cena/Minha cena/Controle Remoto/controleRemoto.obj",
		texControleRemoto,
		"../../Cena/Minha cena/Controle Remoto/controleRemoto.mtl",
		shader,
		posicoesObj[8]);

	string texControlePs4[] = { "../../Cena/Minha cena/Controle PS4/Texturas/black.jpg" };
	objetos[9].initialize("../../Cena/Minha cena/Controle PS4/controlePS4.obj",
		texControlePs4,
		"../../Cena/Minha cena/Controle PS4/controlePS4.mtl",
		shader,
		posicoesObj[9]);

	string texSofa[] = { "../../Cena/Minha cena/Sofa/Texturas/black.jpg" };
	objetos[10].initialize("../../Cena/Minha cena/Sofa/sofa.obj",
		texSofa,
		"../../Cena/Minha cena/Sofa/sofa.mtl",
		shader,
		posicoesObj[10]);
}

//Funções das curvas
std::vector<glm::vec3> generateControlPointsSet(int nPoints) {
	std::vector<glm::vec3> controlPoints;

	// Gerar números aleatórios
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distribution(-0.9f, 0.9f);  // Intervalo aberto (-0.9, 0.9)

	for (int i = 0; i < nPoints; i++) {
		glm::vec3 point;
		do {
			// Gerar coordenadas x e y aleatórias
			point.x = distribution(gen);
			point.y = distribution(gen);
		} while (std::find(controlPoints.begin(), controlPoints.end(), point) != controlPoints.end());

		// Definir coordenada z como 0.0
		point.z = 0.0f;

		controlPoints.push_back(point);
	}

	return controlPoints;
}

vector<glm::vec3> generateControlPointsSet()
{
	vector <glm::vec3> controlPoints;

	controlPoints.push_back(glm::vec3(-0.6, -0.4, 0.0));
	controlPoints.push_back(glm::vec3(-0.4, -0.6, 0.0));
	controlPoints.push_back(glm::vec3(-0.2, -0.2, 0.0));
	controlPoints.push_back(glm::vec3(0.0, 0.0, 0.0));
	controlPoints.push_back(glm::vec3(0.2, 0.2, 0.0));
	controlPoints.push_back(glm::vec3(0.4, 0.6, 0.0));
	controlPoints.push_back(glm::vec3(0.6, 0.4, 0.0));

	return controlPoints;
}

GLuint generateControlPointsBuffer(vector <glm::vec3> controlPoints)
{
	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(GLfloat) * 3, controlPoints.data(), GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);

	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

std::vector<glm::vec3> generateUnisinosPointsSet()
{
	float vertices[] = { -0.262530, 0.376992, 0.000000,
						 -0.262530, 0.377406, 0.000000,
						 -0.262530, 0.334639, 0.000000,
						 -0.262530, 0.223162, 0.000000,
						 -0.262530, 0.091495, 0.000000,
						 -0.262371, -0.006710, 0.000000,
						 -0.261258, -0.071544, -0.000000,
						 -0.258238, -0.115777, -0.000000,
						 -0.252355, -0.149133, -0.000000,
						 -0.242529, -0.179247, -0.000000,
						 -0.227170, -0.208406, -0.000000,
						 -0.205134, -0.237216, -0.000000,
						 -0.177564, -0.264881, -0.000000,
						 -0.146433, -0.289891, -0.000000,
						 -0.114730, -0.309272, -0.000000,
						 -0.084934, -0.320990, -0.000000,
						 -0.056475, -0.328224, -0.000000,
						 -0.028237, -0.334170, -0.000000,
						  0.000000, -0.336873, -0.000000,
						  0.028237, -0.334170, -0.000000,
						  0.056475, -0.328224, -0.000000,
						  0.084934, -0.320990, -0.000000,
						  0.114730, -0.309272, -0.000000,
						  0.146433, -0.289891, -0.000000,
						  0.177564, -0.264881, -0.000000,
						  0.205134, -0.237216, -0.000000,
						  0.227170, -0.208406, -0.000000,
						  0.242529, -0.179247, -0.000000,
						  0.252355, -0.149133, -0.000000,
						  0.258238, -0.115777, -0.000000,
						  0.261258, -0.071544, -0.000000,
						  0.262371, -0.009704, 0.000000,
						  0.262530, 0.067542, 0.000000,
						  0.262769, 0.153238, 0.000000,
						  0.264438, 0.230348, 0.000000,
						  0.268678, 0.284286, 0.000000,
						  0.275462, 0.320338, 0.000000,
						  0.284631, 0.347804, 0.000000,
						  0.296661, 0.372170, 0.000000,
						  0.311832, 0.396628, 0.000000,
						  0.328990, 0.419020, 0.000000,
						  0.347274, 0.436734, 0.000000,
						  0.368420, 0.450713, 0.000000,
						  0.393395, 0.462743, 0.000000,
						  0.417496, 0.474456, 0.000000,
						  0.436138, 0.487056, 0.000000,
						  0.450885, 0.500213, 0.000000,
						  0.464572, 0.513277, 0.000000,
						  0.478974, 0.525864, 0.000000,
						  0.494860, 0.538133, 0.000000,
					      0.510031, 0.552151, 0.000000,
						  0.522127, 0.570143, 0.000000,
						  0.531124, 0.593065, 0.000000,
						  0.537629, 0.620809, 0.000000,
						  0.542465, 0.650303, 0.000000,
						  0.546798, 0.678259, 0.000000,
					      0.552959, 0.703513, 0.000000,
						  0.563121, 0.725745, 0.000000,
						  0.577656, 0.745911, 0.000000,
						  0.596563, 0.764858, 0.000000,
						  0.620160, 0.781738, 0.000000,
						  0.648302, 0.795385, 0.000000,
						  0.678670, 0.805057, 0.000000,
						  0.710336, 0.810741, 0.000000,
						  0.750111, 0.814914, 0.000000,
						  0.802994, 0.819945, 0.000000,
						  0.860771, 0.825435, 0.000000 };

	vector <glm::vec3> uniPoints;

	for (int i = 0; i < 67 * 3; i += 3)
	{
		glm::vec3 point;
		point.x = vertices[i];
		point.y = vertices[i + 1];
		point.z = 0.0;

		uniPoints.push_back(point);
	}

	return uniPoints;
}