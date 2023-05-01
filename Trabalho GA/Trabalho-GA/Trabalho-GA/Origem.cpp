/* Trabalho GA Processamento Gráfico - Unisinos 2023/1 
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

#include "Mesh.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 v_color;
};

// Funções de callback de teclado, mouse e scroll
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int loadSimpleObj(string filePath, int& nVertices, vector <Vertex>& vertices, vector <glm::vec3>& normals, vector <glm::vec2>& texCoord, vector <int>& indices, glm::vec3 color);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Variáveis para armazenar a posição e orientação da camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Variáveis de controle de rotação em x, y, z
bool rotateX = false, rotateY = false, rotateZ = false;

// Variáveis de controle de translação em x, y, z
bool translateX = false, translateY = false, translateZ = false;

// Variáveis de controle de escala das malhas
bool escalaAumentar = false, escalaDiminuir = false;

// Variáveis de seleção de objetos da cena
bool trocarMalhaSelecionada = false;
GLuint MalhaID = 0;

// Variáveis de controle de movimento da câmera
bool movimento = false;
bool scroll = false;

// Variáveis de controle de movimentação de câmera com o mouse
bool firstMouse = true;
float lastX = 0.0, lastY = 0.0;
float yaw = -90.0, pitch = 0.0;
float fov = 45.0f;

GLuint VAOs[2];
Mesh Malhas[2];

// Vetores para armazenar vertices e indices do obj
int nVertices;
vector <Vertex> vertices_obj1;
vector <int> indices_obj1;
vector <glm::vec3> normals_obj1;
vector <glm::vec2> texCoord_obj1;

int nVertices2;
vector <Vertex> vertices_obj2;
vector <int> indices_obj2;
vector <glm::vec3> normals_obj2;
vector <glm::vec2> texCoord_obj2;

int main()
{
	// Inicialização da GLFW
	glfwInit();

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Trabalho GA", nullptr, nullptr);

	//fullscreen
	//GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Trabalho GA", glfwGetPrimaryMonitor(), NULL);

	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	//Desabilita o cursor do mouse
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
	Shader shader("TrabalhoGA.vs", "TrabalhoGA.fs");

	//Carregando os objetos
	VAOs[0] = loadSimpleObj("../../3D_Models/Classic-NoTexture/bunny.obj", nVertices, vertices_obj1, normals_obj1, texCoord_obj1, indices_obj1, glm::vec3(0.8, 0.8, 0.8));
	VAOs[1] = loadSimpleObj("../../3D_Models/Classic-NoTexture/apple.obj", nVertices2, vertices_obj2, normals_obj2, texCoord_obj2, indices_obj2, glm::vec3(1.0, 0.15, 0.0));

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

	//Definindo as propriedades do material 
	shader.setFloat("ka", 0.4);  //Luz ambiente
	shader.setFloat("kd", 0.5);  //Luz difusa
	shader.setFloat("ks", 0.5);  //Luz especular
	shader.setFloat("q", 10);    //Potência especular

	//Definindo as propriedades da fonte de luz
	shader.setVec3("lightPos", -2.0f, 10.0f, 3.0f);
	shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

	Malhas[0].initialize(VAOs[0], nVertices, &shader, glm::vec3(-2.0, 0.0, 0.0));
	Malhas[1].initialize(VAOs[1], nVertices2, &shader, glm::vec3(2.0, 0.0, 0.0));

	glEnable(GL_DEPTH_TEST);

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo(Branco)
		glClearColor(0.6f, 0.6f, 0.6f, 0.6f);  //cor de fundo(Cinza)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		glm::vec3 scale = glm::vec3(1);

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		projection = glm::perspective(glm::radians(fov), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

		if (trocarMalhaSelecionada)
		{
			if (MalhaID == 0) 
			{
				MalhaID++;
			}
			else 
			{
				MalhaID--;
			}

			trocarMalhaSelecionada = false;
		}

		// Rotação em x, y, z
		else if (rotateX)
		{
			Malhas[MalhaID].setAngle(angle);
			Malhas[MalhaID].setAxis(glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else if (rotateY)
		{
			Malhas[MalhaID].setAngle(angle);
			Malhas[MalhaID].setAxis(glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (rotateZ)
		{
			Malhas[MalhaID].setAngle(angle);
			Malhas[MalhaID].setAxis(glm::vec3(0.0f, 0.0f, 1.0f));
		}

		// Translação em x, y, z
		else if (translateX)
		{
			Malhas[MalhaID].setPosition(glm::vec3(cos(angle) * 10.0, Malhas[MalhaID].getPosition().y, Malhas[MalhaID].getPosition().z));
		}
		else if (translateY)
		{
			Malhas[MalhaID].setPosition(glm::vec3(Malhas[MalhaID].getPosition().x, cos(angle) * 10.0, Malhas[MalhaID].getPosition().z));
		}
		else if (translateZ)
		{
			Malhas[MalhaID].setPosition(glm::vec3(Malhas[MalhaID].getPosition().x, Malhas[MalhaID].getPosition().y, cos(angle) * 10.0));
		}

		// Escala
		else if (escalaAumentar)
		{
			scale = Malhas[MalhaID].getScale();
			scale += glm::vec3(0.05);
			Malhas[MalhaID].setScale(scale);
			escalaAumentar = false;
		}
		else if (escalaDiminuir)
		{
			scale = Malhas[MalhaID].getScale();
			scale -= glm::vec3(0.05);
			Malhas[MalhaID].setScale(scale);
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

		// Chamada de desenho - drawcall
		Malhas[0].update();
		Malhas[0].draw();

		Malhas[1].update();
		Malhas[1].draw();

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}

	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAOs[0]);
	glDeleteVertexArrays(1, &VAOs[1]);

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
		trocarMalhaSelecionada = false;
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

		Malhas[0].setPosition(glm::vec3(-2.0, 0.0, 0.0));
		Malhas[0].setScale(glm::vec3(1));
		Malhas[0].setAngle(glm::radians(0.0f));
		Malhas[0].setAxis(glm::vec3(0.0, 0.0, 1.0));

		Malhas[1].setPosition(glm::vec3(2.0, 0.0, 0.0));
		Malhas[1].setScale(glm::vec3(1));
		Malhas[1].setAngle(glm::radians(0.0f));
		Malhas[1].setAxis(glm::vec3(0.0, 0.0, 1.0));
	}

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		trocarMalhaSelecionada = false;
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
		trocarMalhaSelecionada = false;
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
		trocarMalhaSelecionada = false;
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
		trocarMalhaSelecionada = false;
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
		trocarMalhaSelecionada = false;
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
		trocarMalhaSelecionada = false;
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
		trocarMalhaSelecionada = false;
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
		trocarMalhaSelecionada = false;
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
		trocarMalhaSelecionada = true;
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
	float cameraSpeed = 0.05f;
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

int loadSimpleObj(string filePath, int& nVertices, vector <Vertex>& vertices, vector <glm::vec3>& normals, vector <glm::vec2>& texCoord, vector <int>& indices, glm::vec3 color)
{
	ifstream inputFile;
	inputFile.open(filePath);
	vector <GLfloat> vertbuffer;

	if (inputFile.is_open())
	{
		char line[100];
		string sline;

		while (!inputFile.eof())
		{
			inputFile.getline(line, 100);
			sline = line;

			string word;
			istringstream ssline(sline);

			ssline >> word;

			if (word == "v")
			{
				Vertex v;
				ssline >> v.position.x >> v.position.y >> v.position.z;
				v.v_color.r = color.r; v.v_color.g = color.g; v.v_color.b = color.b;
				vertices.push_back(v);
			}
			if (word == "vt")
			{
				glm::vec2 vt;
				ssline >> vt.s >> vt.t;
				texCoord.push_back(vt);
			}
			if (word == "vn")
			{
				glm::vec3 vn;
				ssline >> vn.x >> vn.y >> vn.z;
				normals.push_back(vn);
			}
			else if (word == "f")
			{
				string tokens[3];
				for (int i = 0; i < 3; i++)
				{
					ssline >> tokens[i];
					int pos = tokens[i].find("/");
					string token = tokens[i].substr(0, pos);
					int index = atoi(token.c_str()) - 1;
					indices.push_back(index);
					vertbuffer.push_back(vertices[index].position.x);
					vertbuffer.push_back(vertices[index].position.y);
					vertbuffer.push_back(vertices[index].position.z);
					vertbuffer.push_back(vertices[index].v_color.r);
					vertbuffer.push_back(vertices[index].v_color.g);
					vertbuffer.push_back(vertices[index].v_color.b);

					tokens[i] = tokens[i].substr(pos + 1);
					if (!texCoord.empty()) {
						pos = tokens[i].find("/");
						token = tokens[i].substr(0, pos);
						int indexT = atoi(token.c_str()) - 1;
						vertbuffer.push_back(texCoord[indexT].s);
						vertbuffer.push_back(texCoord[indexT].t);
					}
					else {
						//Se não tiver coordenadas de textura, definir como zero 
						//Para não acabar colocando as normais no lugar das coordenadas de textura
						vertbuffer.push_back(0);
						vertbuffer.push_back(0);
					}

					if (!texCoord.empty()) {
						tokens[i] = tokens[i].substr(pos + 1);
					}
					else {
						//Se não tiver coordenadas de textura, apenas remover a barra
						//E definir pos como o tamanho da palavra restante
						tokens[i] = tokens[i].substr(1);
						pos = tokens[i].length();
					}
					token = tokens[i].substr(0, pos);
					int indexN = atoi(token.c_str()) - 1;
					vertbuffer.push_back(normals[indexN].x);
					vertbuffer.push_back(normals[indexN].y);
					vertbuffer.push_back(normals[indexN].z);
				}
			}
		}

		inputFile.close();
	}
	else
	{
		cout << "Não foi possivel abrir o arquivo " << filePath << endl;
	}

	nVertices = vertbuffer.size() / 11;

	GLuint VBO, VAO;

	//Geração do identificador dos VBOs
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, vertbuffer.size() * sizeof(GLfloat), vertbuffer.data(), GL_STATIC_DRAW);

	//Geração do identificador dos VAOs (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);

	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 

	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	//Atributo coordenadas de textura (s, t)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	//Atributo vetor normal (x, y e z)
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}