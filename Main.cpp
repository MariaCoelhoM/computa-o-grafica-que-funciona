#include <stdio.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

// VAO é o ponteiro do programa que estou rodando, espaço de memória que representa aquele objeto
// VBO é o ponteiro para os meus dados brutos, são os dados do objeto a ser representado pelo VAO 
// shaderProgram é qual programa estou rodando 
// todo programa pode ser chamado de shader

// variaveis globais
const GLint WIDTH = 800, HEIGHT = 600; // O GLint vai ser o componente criado direto no glew
GLuint VAO, VBO, IBO, shaderProgram; //um inteiro não sinalizado onde só vai ter valores 0,512 não tem o lado negativo

float toRadians = 3.1415f / 180.0f;
//float directionY = false;
bool direction = false, directionSize = false;// quando for falso vamos adicionando se for true vamos subtraindo
float triOffset = 0.0f, triOffsetMax = 0.7f, triOffsetMin = -0.7f, triIncrement = 0.03f; //por onde vamos caminhar vamos começar no 0 no meio da tela - triOffsetMax maximo que vamos chegar a direita na tela - triOffsetMin valor minimo que chegaremos a esquerda
//float triOffsetY = 0.0f, triOffsetMaxY = 0.7f, triOffsetMinY = -0.7f, triIncrementY = 0.01f;//qualquer coisa apaga isso
float triOffsetSize = 0.2f, triOffsetSizeMax = 1.2f, triOffsetSizeMin = 0.2f, triOffsetSizeIncrement = 0.1f;
float triCurrentAngle = 0.0f, triAngleIncrement = 1.0f;

//código em GLSL - vc2 vai ter o valor de (x e y) 
// o programa vai pegar a posição de x e y
// passando um argumento para o inicio do programa (args do C++                        
// estou passando um argumento de entrada na primeira posiçâo                             
// esse argumento deve ser um vetor de duas posições
// layout pode mudar mais depois da execução
// uniform float posX; - nova posição de dados 
// o clamp faz direito pegar o min e max dos valores 
static const char* vertexShader = "									\n\
#version 330														\n\
																	\n\
layout(location=0) in vec3 pos;										\n\
uniform mat4 model;													\n\
uniform mat4 projection;											\n\
out vec4 vCol;														\n\
																	\n\
void main() {														\n\
	gl_Position = projection * model * vec4(pos, 1.0);				\n\
	vCol = vec4(clamp(pos, 0.0 , 1.0f), 1.0f);						\n\
}																	\n\
";

//fragmento o que podemos mudar do desenho que podemos fazer o vec3 e por causa do RGB
// diferente da entrada por layout, uniform é uma entrada em tempo de execução   
// fragment pode ser lido como "a partir desse ponto o que faço com ele?"
// shader para atribuir cores aos pontos
// diferente da entrada por layout, uniform é uma entrada em tempo de execução 
// uniform pode mudar adurante a execução da cor
static const char* fragmentShader = "				\n\
#version 330										\n\
													\n\
in vec4 vCol;										\n\
uniform vec3 triColor;								\n\
out vec4 color;										\n\
													\n\
void main() {										\n\
	color = vCol;									\n\
}													\n\
";

void criarTriangulo() {
	unsigned int indices[] = {
		0,1,2, //Base
		0,1,3, //face 1 da esquerda
		0,2,3, //face 2 da direita
		1,2,3, //frente
	};


	GLfloat vertices[] = { //nosso buffer de vertices
		0.0f, 1.0f, 0.0f,//vertices 1
		-1.0f, -1.0f, 0.0f, //vertices 2
		1.0f, -1.0f, 0.0f, //vertices 3
		0.0f, 0.0f, 1.0f, //vertice 4 
	};

	// abrir um espaço na placa de video a onde vai ser guardado algo
	glGenVertexArrays(1, &VAO); // alocando - inteiro não sinalizado
	glBindVertexArray(VAO); // me da o espaço de memoria pra eu alterar ele!
	
	glGenBuffers(1, &IBO); // IBO - indice buffer objects
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO); // o VAO esta apontando pro VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //começa a contar pra escrever 

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0); //location

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

//adiciona um programa dentro da tela
void adicionaTriangulo(GLuint program, const char* shaderCode, GLenum type) {
	GLuint _shader = glCreateShader(type);

	// converte char para GL_CHAR
	const GLchar* code[1];
	code[0] = shaderCode;

	glShaderSource(_shader, 1, code, NULL);
	glCompileShader(_shader);

	// tratar os erros (aula que vêm)



	glAttachShader(program, _shader);
}

void adicionaPrograma() {
	shaderProgram = glCreateProgram();
	if (!shaderProgram) {
		printf("Erro ao criar o programa");
		return;
	}
	adicionaTriangulo(shaderProgram, vertexShader, GL_VERTEX_SHADER);
	adicionaTriangulo(shaderProgram, fragmentShader, GL_FRAGMENT_SHADER);

	glLinkProgram(shaderProgram);
}

// quando o aplicativo rodar vai rodar o main
int main() {
	// iniciar o glfw 
	if (!glfwInit()) {
		printf("Glfw não foi iniciado");
		glfwTerminate(); // isso e caso tenha um erro e gaste a memoria da memoria do computador então vai parar o erro que pode ser causado
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // isso e so pra usar a base
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // tudo que tiver gl na frente e do glew - o glew vai fazer todo o gerenciamento de agora em diante
	// o glew sempre gera o o tamanho exemplo float ou int - a gente manda o glew fazer isso

	//fazendo uma janela
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Computação gráfica", NULL, NULL); // o window vai ser um ponteiro ja que o glfwCreateWindow devolve um endereço

	if (!window) {
		printf("Erro ao criar a janela");
		glfwTerminate();
		return 1;
	}

	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
	glfwMakeContextCurrent(window); // principal janela que vai ser usada

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		printf("Erro ao inicar o glew");
		glfwDestroyWindow(window); // destroi a janela caso de erro!
		glfwTerminate(); // isso e caso tenha um erro e gaste a memoria da memoria do computador então vai parar o erro que pode ser causado
		return 1;
	}

	glViewport(0, 0, bufferWidth, bufferHeight); // os 0´s vão começar na altura e largura  0/0  - o que vai ser no meio
	criarTriangulo();
	adicionaPrograma();

	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)bufferWidth / (GLfloat)bufferHeight, 0.1f, 100.0f); //camera - distancia - o qual perto e o qual longe da camera

	// de fundo da janela
	while (!glfwWindowShouldClose(window)) { // enquanto a janela não for fechada o loop vai continuar rodando
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // RGBA A - de transparencia
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT); // altera a cor de fundo
	
	//altera a cor do triangulo
	GLint uniformColor = glGetUniformLocation(shaderProgram, "triColor");
	glUniform3f(uniformColor, 1.0f, 1.0f, 0.0f); // e pra sair a cor amarela

	//movimenta o triangulo X
	if (!direction) {
		triOffset += triIncrement;
	}
	else {
		triOffset -= triIncrement;
	}
	if (triOffset > triOffsetMax || triOffset < triOffsetMin) 
		direction = !direction;
	triCurrentAngle += triAngleIncrement;
	
	if (triCurrentAngle >= 360)
		triCurrentAngle = 0;
	if (!directionSize)
		triOffsetSize += triOffsetSizeIncrement;
	else
		triOffsetSize -= triOffsetSizeIncrement;
	if (triOffsetSize > triOffsetSizeMax || triOffsetSize < triOffsetSizeMin)
		directionSize = !directionSize;

	GLint uniformModel = glGetUniformLocation(shaderProgram, "model"); // estamos pegando as posições
	GLint uniformProjection = glGetUniformLocation(shaderProgram, "projection");
	glm::mat4 model(1.0f); //criamos uma matriz 4x4 onde tem 1´s dentro dela
	
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
	model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
	model = glm::rotate(model, triCurrentAngle * toRadians, glm::vec3(0.7f, 0.5f, 1.0f)); // isso aqui faz rotação no x, y,z
	
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));



	//movimenta o triangulo Y
	//if (!directionY) {
	//	triOffsetY += triIncrementY;
	//}
	//else {
	//	triOffsetY -= triIncrementY;
	//}
	//if (triOffsetY > triOffsetMaxY || triOffsetY < triOffsetMinY)
	//	directionY = !directionY;

	//GLint uniformPosY = glGetUniformLocation(shaderProgram, "posY");
	//glUniform1f(uniformPosY, triOffsetY);



	// testando com numeros aleatorios 

	GLint R = glGetUniformLocation(shaderProgram, "triColor");
	glUniform3f(uniformColor, 1.0f, 1.0f, 0.0f);

	//desenhando o triangulo
	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0); // os 12 lados dos triangulos e seus pontos de ligação entre eles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // pra sinalizar que acabou o IBO
	glBindVertexArray(0);

	//glBindVertexArray(VAO);
	//	glDrawArrays(GL_TRIANGLES, 0, 3); //triangulo começando na posição 0, numeros de pontos 3
	//glBindVertexArray(0);


	glfwSwapBuffers(window);

}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}