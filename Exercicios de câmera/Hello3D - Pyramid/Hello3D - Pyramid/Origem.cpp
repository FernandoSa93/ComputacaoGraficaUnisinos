/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 01/03/2023
 *
 */

#include <iostream>
#include <string>
#include <assert.h>

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


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Protótipos das funções
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

bool rotateX=false, rotateY=false, rotateZ=false,
  posicao_1 = false, posicao_2 = false, posicao_3 = false, posicao_4 = false, posicao_5 = false, posicao_6 = false;

//Exercicio 2
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool movimento = false;
bool scroll = false;
//Fim Exercicio 2

bool firstMouse = true;
float lastX = 0.0, lastY = 0.0;
float yaw = -90.0, pitch = 0.0;
float fov = 45.0f;

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	//Muita atenção aqui: alguns ambientes não aceitam essas configurações
	//Você deve adaptar para a versão do OpenGL suportada por sua placa
	//Sugestão: comente essas linhas de código para desobrir a versão e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola Cubo!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPos(window, WIDTH/2, HEIGHT/2);
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
	Shader shader("Hello3D.vs", "Hello3D.fs");

	// Gerando um buffer simples, com a geometria de um triângulo
	GLuint VAO = setupGeometry();


	glUseProgram(shader.ID);

	//Exercicio
	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shader.ID, "model");
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glm::mat4 view;
	//view = glm::lookAt(glm::vec3(0.0f, 0.0f, -3.0f),  // Posição (ponto)
	//				   glm::vec3(0.0f, 0.0f, 0.0f),  // Target (ponto, não vetor) -> dir = target - pos
	//				   glm::vec3(0.0f, 1.0f, 0.0f)); // Up (vetor)
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	GLint viewLoc = glGetUniformLocation(shader.ID, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
	GLint projLoc = glGetUniformLocation(shader.ID, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	//Fim Exercicio

	glEnable(GL_DEPTH_TEST);

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		model = glm::mat4(1); 
		//Exercicio
		view = glm::lookAt(glm::vec3(0.0f, 0.0f, -3.0f),  // Posição (ponto)
			               glm::vec3(0.0f, 0.0f, 0.0f),  // Target (ponto, não vetor) -> dir = target - pos
			               glm::vec3(0.0f, 1.0f, 0.0f)); // Up (vetor)
		projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
		//Fim Exercicio

		if (rotateX)
		{
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
			
		}
		else if (rotateY)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

		}
		else if (rotateZ)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

		}
		//Exercicio
		else if (posicao_1) //Frente
		{
			view = glm::lookAt(glm::vec3(0.0f, 0.0f, -3.0f),  // Posição (ponto)
				               glm::vec3(0.0f, 0.0f, 0.0f),  // Target (ponto, não vetor) -> dir = target - pos
				               glm::vec3(0.0f, 1.0f, 0.0f)); // Up (vetor)
		}
		else if (posicao_2) //Direita
		{
			view = glm::lookAt(glm::vec3(3.0f, 0.0f, 0.0f),  // Posição (ponto)
				               glm::vec3(0.0f, 0.0f, 0.0f),  // Target (ponto, não vetor) -> dir = target - pos
				               glm::vec3(0.0f, 1.0f, 0.0f)); // Up (vetor)
		}
		else if (posicao_3) //Trás
		{
			view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),  // Posição (ponto)
				               glm::vec3(0.0f, 0.0f, 0.0f),  // Target (ponto, não vetor) -> dir = target - pos
			 	               glm::vec3(0.0f, 1.0f, 0.0f)); // Up (vetor)
		}
		else if (posicao_4) //Esquerda
		{
			view = glm::lookAt(glm::vec3(-3.0f, 0.0f, 0.0f),  // Posição (ponto)
				               glm::vec3(0.0f, 0.0f, 0.0f),  // Target (ponto, não vetor) -> dir = target - pos
				               glm::vec3(0.0f, 1.0f, 0.0f)); // Up (vetor)
		}
		else if (posicao_5) //Cima
		{
			view = glm::lookAt(glm::vec3(0.0f, 3.0f, 0.1f),  // Posição (ponto)
				               glm::vec3(0.0f, 0.0f, 0.0f),  // Target (ponto, não vetor) -> dir = target - pos
				               glm::vec3(0.0f, 1.0f, 0.0f)); // Up (vetor)
		}
		else if (posicao_6) //Baixo
		{
			view = glm::lookAt(glm::vec3(0.0f, -3.0f, 0.1f),  // Posição (ponto)
				               glm::vec3(0.0f, 0.0f, 0.0f),  // Target (ponto, não vetor) -> dir = target - pos
				               glm::vec3(0.0f, 1.0f, 0.0f)); // Up (vetor)
		}
		//Movimentando a câmera com A, W, S e D e usando o mouse
		else if (movimento) {
			view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		}

		//Scroll do mouse para zoom in e zoom out
		else if (scroll) {
			projection = glm::perspective(glm::radians(fov), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
		}

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		//Fim Exercicio

		// Chamada de desenho - drawcall
		// Poligono Preenchido - GL_TRIANGLES
		
		glBindVertexArray(VAO);
		//QTD triângulos * 3
		glDrawArrays(GL_TRIANGLES, 0, 42);

		// Chamada de desenho - drawcall
		// CONTORNO - GL_LINE_LOOP
		
		//QTD triângulos * 3
		glDrawArrays(GL_POINTS, 0, 42);
		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
	}

	//Exercicio
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		posicao_1 = true;
		posicao_2 = false;
		posicao_3 = false;
		posicao_4 = false;
		posicao_5 = false;
		posicao_6 = false;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		posicao_1 = false;
		posicao_2 = true;
		posicao_3 = false;
		posicao_4 = false;
		posicao_5 = false;
		posicao_6 = false;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		posicao_1 = false;
		posicao_2 = false;
		posicao_3 = true;
		posicao_4 = false;
		posicao_5 = false;
		posicao_6 = false;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		posicao_1 = false;
		posicao_2 = false;
		posicao_3 = false;
		posicao_4 = true;
		posicao_5 = false;
		posicao_6 = false;
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		posicao_1 = false;
		posicao_2 = false;
		posicao_3 = false;
		posicao_4 = false;
		posicao_5 = true;
		posicao_6 = false;
	}
	if (key == GLFW_KEY_6 && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		posicao_1 = false;
		posicao_2 = false;
		posicao_3 = false;
		posicao_4 = false;
		posicao_5 = false;
		posicao_6 = true;
	}
	//Fim Exercicio

	//Exercicio 2
	if ((key == GLFW_KEY_W) || (key == GLFW_KEY_S) || (key == GLFW_KEY_A) || (key == GLFW_KEY_D)) {
		rotateX = false;
		rotateY = false;
		rotateZ = false;
		posicao_1 = false;
		posicao_2 = false;
		posicao_3 = false;
		posicao_4 = false;
		posicao_5 = false;
		posicao_6 = false;
		scroll = false;
		movimento = true;
	}

	float cameraSpeed = 0.05f;
	if (key == GLFW_KEY_W)
		cameraPos += cameraSpeed * cameraFront;
	if (key == GLFW_KEY_S)
		cameraPos -= cameraSpeed * cameraFront;
	if (key == GLFW_KEY_A)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (key == GLFW_KEY_D)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	//Fim Exercicio 2
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	rotateX = false;
	rotateY = false;
	rotateZ = false;
	posicao_1 = false;
	posicao_2 = false;
	posicao_3 = false;
	posicao_4 = false;
	posicao_5 = false;
	posicao_6 = false;
	scroll = false;
	movimento = true;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window,double xoffset, double yoffset)
{
	rotateX = false;
	rotateY = false;
	rotateZ = false;
	posicao_1 = false;
	posicao_2 = false;
	posicao_3 = false;
	posicao_4 = false;
	posicao_5 = false;
	posicao_6 = false;
	movimento = false;
	scroll = true;

	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a 
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupGeometry()
{
	// Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
	// sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
	// Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO único ou em VBOs separados
	GLfloat vertices[] = {

		//Base do quadrado: 2 triângulos
		//x    y    z    r    g    b
		-0.5, -0.5, -0.5, 1.0, 0.0, 0.0,
		-0.5, -0.5,  0.5, 1.0, 0.0, 0.0,
		 0.5, -0.5, -0.5, 1.0, 0.0, 0.0,

		-0.5, -0.5,  0.5, 1.0, 0.0, 0.0,
		 0.5, -0.5,  0.5, 1.0, 0.0, 0.0,
		 0.5, -0.5, -0.5, 1.0, 0.0, 0.0,

		 //Topo do quadrado: 2 triângulos
	     //x    y    z    r    g    b
		-0.5, 0.5, -0.5, 0.0, 1.0, 0.0,
		-0.5, 0.5,  0.5, 0.0, 1.0, 0.0,
		 0.5, 0.5, -0.5, 0.0, 1.0, 0.0,

		-0.5, 0.5,  0.5, 0.0, 1.0, 0.0,
		 0.5, 0.5,  0.5, 0.0, 1.0, 0.0,
		 0.5, 0.5, -0.5, 0.0, 1.0, 0.0,

		 //Frente do quadrado: 2 triângulos
		 //x    y    z    r    g    b
		-0.5, -0.5, -0.5, 0.0, 0.0, 1.0,
		 0.5,  0.5, -0.5, 0.0, 0.0, 1.0,
		 0.5, -0.5, -0.5, 0.0, 0.0, 1.0,

		-0.5, -0.5, -0.5, 0.0, 0.0, 1.0,
		 0.5,  0.5, -0.5, 0.0, 0.0, 1.0,
		-0.5,  0.5, -0.5, 0.0, 0.0, 1.0,

		 //Direita do quadrado: 2 triângulos
		 //x    y    z    r    g    b
		 0.5, -0.5,  0.5, 1.0, 1.0, 0.0,
		 0.5,  0.5, -0.5, 1.0, 1.0, 0.0,
		 0.5, -0.5, -0.5, 1.0, 1.0, 0.0,

		 0.5, -0.5,  0.5, 1.0, 1.0, 0.0,
		 0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
		 0.5,  0.5, -0.5, 1.0, 1.0, 0.0,

		 //Esquerda do quadrado: 2 triângulos
		 //x    y    z    r    g    b
		-0.5,  -0.5,  0.5, 1.0, 0.0, 1.0,
		-0.5,  0.5, -0.5, 1.0, 0.0, 1.0,
		-0.5, -0.5, -0.5, 1.0, 0.0, 1.0,

		-0.5, -0.5,  0.5, 1.0, 0.0, 1.0,
		-0.5,  0.5,  0.5, 1.0, 0.0, 1.0,
		-0.5,  0.5, -0.5, 1.0, 0.0, 1.0,

		 //Trás do quadrado: 2 triângulos
		 //x    y    z    r    g    b
		-0.5, -0.5, 0.5, 0.0, 1.0, 1.0,
		 0.5,  0.5, 0.5, 0.0, 1.0, 1.0,
		 0.5, -0.5, 0.5, 0.0, 1.0, 1.0,

		-0.5, -0.5, 0.5, 0.0, 1.0, 1.0,
		 0.5,  0.5, 0.5, 0.0, 1.0, 1.0,
		-0.5,  0.5, 0.5, 0.0, 1.0, 1.0,

		//Chão: 2 triângulos
		//x    y    z    r    g    b
		-5.0, -0.5, -5.0, 0.5, 0.5, 0.5,
		-5.0, -0.5,  5.0, 0.5, 0.5, 0.5,
		 5.0, -0.5, -5.0, 0.5, 0.5, 0.5,

		-5.0, -0.5,  5.0, 0.5, 0.5, 0.5,
		 5.0, -0.5,  5.0, 0.5, 0.5, 0.5,
		 5.0, -0.5, -5.0, 0.5, 0.5, 0.5,

	};

	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);



	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

