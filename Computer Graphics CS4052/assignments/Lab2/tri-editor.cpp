#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <shader.h>
#include <glfw-utilities.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
GLuint generateObjectBuffer(GLuint numVertices, GLfloat vertices[], GLfloat colors[]) {
	// Genderate 1 generic buffer object, called VBO
	GLuint VBO;
 	glGenBuffers(1, &VBO);
	// In OpenGL, we bind (make active) the handle to a target name and then execute commands on that target
	// Buffer will contain an array of vertices 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// After binding, we now fill our object with data, everything in "Vertices" goes to the GPU
	glBufferData(GL_ARRAY_BUFFER, numVertices*7*sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	// if you have more data besides vertices (e.g., vertex colours or normals), use glBufferSubData to tell the buffer when the vertices array ends and when the colors start
	glBufferSubData (GL_ARRAY_BUFFER, 0, numVertices*3*sizeof(GLfloat), vertices);
	glBufferSubData (GL_ARRAY_BUFFER, numVertices*3*sizeof(GLfloat), numVertices*4*sizeof(GLfloat), colors);
return VBO;
}

void linkCurrentBuffertoShader(GLuint numVertices, GLuint shaderProgramID){
	// find the location of the variables that we will be using in the shader program
	GLuint positionID = glGetAttribLocation(shaderProgramID, "vPosition");
	GLuint colorID = glGetAttribLocation(shaderProgramID, "vColor");
	// Have to enable this
	glEnableVertexAttribArray(positionID);
	// Tell it where to find the position data in the currently active buffer (at index positionID)
    glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Similarly, for the color data.
	glEnableVertexAttribArray(colorID);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(numVertices*3*sizeof(GLfloat)));
}
#pragma endregion VBO_FUNCTIONS



// Create 3 vertices that make up a triangle that fits on the viewport 
GLfloat vertices[] = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	0.5f,  0.5f, 0.0f
};
// Create a color array that identfies the colors of each vertex (format R, G, B, A)
GLfloat colors[] = {
	0.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f
};

glm::vec3 triPositions[] = {
	glm::vec3( 0.0f,  0.0f,  0.0f), 
	glm::vec3( 2.0f,  5.0f, -15.0f), 
	glm::vec3(-1.5f, -2.2f, -2.5f),  
	glm::vec3(-3.8f, -2.0f, -12.3f),  
	glm::vec3( 2.4f, -0.4f, -3.5f),  
	glm::vec3(-1.7f,  3.0f, -7.5f),  
	glm::vec3( 1.3f, -2.0f, -2.5f),  
	glm::vec3( 1.5f,  2.0f, -2.5f), 
	glm::vec3( 1.5f,  0.2f, -1.5f), 
	glm::vec3(-1.3f,  1.0f, -1.5f) 
};

const GLuint numVertices = 3;
const GLuint numTriangles = 10;

glm::mat4 triTrans[numTriangles];
glm::mat4 triRot[numTriangles];
glm::mat4 triScale[numTriangles];
glm::vec3 triColor[numTriangles];

enum EditState {
	TRANS,
	ROT,
	SCALE
};

EditState EDITMODE = TRANS;
glm::vec3 EDITAXIS(1,0,0);
bool SELECTION[numTriangles];

void translateSel(glm::vec3 trans) {
	for(int i=0;i<numTriangles;i++) {
		if(SELECTION[i])
			triTrans[i] = glm::translate(triTrans[i], trans);
	}
}

void rotateSel(float deg, glm::vec3 axis) {
	if(axis == glm::vec3(0)) return;
	for(int i=0;i<numTriangles;i++) {
		if(SELECTION[i])
			triRot[i] = glm::rotate(triRot[i], glm::radians(deg), axis);
	}
}

void scaleSel(glm::vec3 scale) {
	for(int i=0;i<numTriangles;i++) {
		if(SELECTION[i])
			triScale[i] = glm::scale(triScale[i], scale);
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
		EDITMODE = TRANS;	
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		EDITMODE = ROT;	
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		EDITMODE = SCALE;	
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
		EDITAXIS = glm::vec3(1,0,0);
		for(int i=0;i<numTriangles;i++) {
			triTrans[i] = glm::mat4(1);
			triRot[i] = glm::mat4(1);
			triScale[i] = glm::mat4(1);
			SELECTION[i] = false;
		}
		SELECTION[0] = true;
	}
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
		EDITAXIS.x = EDITAXIS.x ? 0 : 1;
	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
		EDITAXIS.y = EDITAXIS.y ? 0 : 1;
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		EDITAXIS.z = EDITAXIS.z ? 0 : 1;
	if (EDITMODE == TRANS){
		if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
			translateSel(-EDITAXIS/10.0);
		if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
			translateSel(EDITAXIS/10.0);
	}
	if (EDITMODE == ROT){
		if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
			rotateSel(5.0f, EDITAXIS);
		if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
			rotateSel(-5.0f, EDITAXIS);
	}
	if (EDITMODE == SCALE){
		if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
			scaleSel(glm::vec3(1) - (EDITAXIS/2.0));
		if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
			scaleSel(glm::vec3(1) + (EDITAXIS/2.0f));
	}
	
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		SELECTION[0] = SELECTION[0] ? false : true;
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		SELECTION[1] = SELECTION[1] ? false : true;
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		SELECTION[2] = SELECTION[2] ? false : true;
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
		SELECTION[3] = SELECTION[3] ? false : true;
	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
		SELECTION[4] = SELECTION[4] ? false : true;
	if (key == GLFW_KEY_6 && action == GLFW_PRESS)
		SELECTION[5] = SELECTION[5] ? false : true;
	if (key == GLFW_KEY_7 && action == GLFW_PRESS)
		SELECTION[6] = SELECTION[6] ? false : true;
	if (key == GLFW_KEY_8 && action == GLFW_PRESS)
		SELECTION[7] = SELECTION[7] ? false : true;
	if (key == GLFW_KEY_9 && action == GLFW_PRESS)
		SELECTION[8] = SELECTION[8] ? false : true;
	if (key == GLFW_KEY_0 && action == GLFW_PRESS)
		SELECTION[9] = SELECTION[9] ? false : true;
}

int main(int argc, char** argv){
	// GLFW Error handling.
	glfwSetErrorCallback(error_callback);

	// Initialise GLFW library.
	if (!glfwInit())
        exit(EXIT_FAILURE);

	// Minimum GL 4.2
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Request Fullscreen window.
	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primary);
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Lab 2", primary, NULL);
	if (!window)
	{
		cerr << "GLFW Could not create window." << endl;
		glfwDestroyWindow(window);
	}
	
	// Associate with created GL context.
    glfwMakeContextCurrent(window);

	// Set window resize function
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Set Keyboard handling function.
    glfwSetKeyCallback(window, key_callback);

	// A call to glewInit() must be done after glfw is initialized!
    GLenum res = glewInit();
	// Check for any errors
    if (res != GLEW_OK) {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
    }

	glEnable(GL_DEPTH_TEST);  
	glfwSwapInterval(1);

	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	Shader shaderProgram("shaders/mvp-tri-col.vert", "shaders/basic.frag");
	shaderProgram.use();

	vbo = generateObjectBuffer(numVertices, vertices, colors);	
	linkCurrentBuffertoShader(numVertices, shaderProgram.ID);	

	// Init Transforms
	for(int i=0;i<numTriangles;i++) {
		triTrans[i] = glm::mat4(1);
		triRot[i] = glm::mat4(1);
		triScale[i] = glm::mat4(1);
		triColor[i] = glm::vec3((double)rand() / (double)RAND_MAX , (double)rand() / (double)RAND_MAX , (double)rand() / (double)RAND_MAX );
		SELECTION[i] = false;
	}
	SELECTION[0] = true;

	while (!glfwWindowShouldClose(window))
	{
		// Rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activate Shader 
		shaderProgram.use();

		// Set Viewport
		int width, height;     
		glViewport(0, 0, width, height);
		glfwGetFramebufferSize(window, &width, &height);

		// Configure Transformations
		glm::mat4 view(1), projection(1);
		projection = glm::perspective(glm::radians(45.0f), (float) width / (float)height, 0.1f, 100.0f);
		view = glm::translate(glm::vec3(0.0f, 0.0f, -3.0f));
		shaderProgram.setMat4f("mV", view);	
		shaderProgram.setMat4f("mP", projection);	
		
		// NB: Make the call to draw the geometry in the currently activated vertex buffer. This is where the GPU starts to work!
		glBindVertexArray(vao);
        for (unsigned int i = 0; i < numTriangles; i++)
        {
			if(!SELECTION[i])
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else 
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::translate(triPositions[i]);
			model = model * triTrans[i] * triScale[i] * triRot[i];

            shaderProgram.setMat4f("mM", model);
			shaderProgram.setVec3f("vTriColour", triColor[i]);

            glDrawArrays(GL_TRIANGLES, 0, numVertices);
        }

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}