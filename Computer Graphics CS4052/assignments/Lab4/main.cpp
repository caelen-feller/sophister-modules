// Windows includes (For Time, IO, etc.)
// #include <windows.h>
// #include <mmsystem.h>
// Note, I compiled and ran this on linux, so I didn't use these!

#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
#include "maths_funcs.h"

#pragma region SimpleTypes
typedef struct
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
	std::vector<unsigned int> mIndices;
} ModelData;
#pragma endregion SimpleTypes

using namespace std;
GLuint shaderProgramID;

const int n = 7;
ModelData mesh_data[n];
unsigned int mesh_vao[n];
mat4 mesh_models[n];
int width = 1280;
int height = 720;

char * mesh_names[n] = {"models/bike/body.obj", 
						"models/bike/steering.obj", 
						"models/bike/pedals.obj", 
						"models/bike/saddlebar.obj", 
						"models/bike/saddle.obj",
						"models/bike/wheelB.obj",
						"models/bike/wheelB.obj"}; 

int parents[n] = {-1,0,0,0,3,0,1};

vec3 mesh_origins[n] = {
	vec3(-0.029388, 0.025388,-0.018609),
	vec3(-1.34,0,0),
	vec3(0,0,0),
	vec3(0.466992,1.96509,0),
	vec3(0.386864-0.466992,2.30173-1.96509-.1,0),
	vec3(1.48472,0.090538,0),
	vec3(-1.4,0.090537,-0.00043)
};

GLuint loc1, loc2, loc3;
GLfloat rotate_y = 0.0f;


#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name, 
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	); 

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
		for(unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for(unsigned int j = 0; j < face.mNumIndices; j++)
				modelData.mIndices.push_back(face.mIndices[j]);
		}  
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp = fopen(shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "shaders/simpleVertexShader.vert", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "shaders/simpleFragmentShader.frag", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh() {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.
	
	for(int i = 0; i < n; i++) 
	{
		mesh_models[i] = identity_mat4();
		
		mesh_vao[i] = 0;
		glGenVertexArrays (1, &mesh_vao[i]);
		glBindVertexArray(mesh_vao[i]);


		mesh_data[i] = load_mesh(mesh_names[i]);
		unsigned int vp_vbo = 0;
		loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
		loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
		loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

		glGenBuffers(1, &vp_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
		glBufferData(GL_ARRAY_BUFFER, mesh_data[i].mPointCount * sizeof(vec3), &mesh_data[i].mVertices[0], GL_STATIC_DRAW);
		unsigned int vn_vbo = 0;
		glGenBuffers(1, &vn_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
		glBufferData(GL_ARRAY_BUFFER, mesh_data[i].mPointCount * sizeof(vec3), &mesh_data[i].mNormals[0], GL_STATIC_DRAW);
		
		unsigned int ebo = 0;
		glGenBuffers(1, &ebo);


		//	This is for texture coordinates which you don't currently need, so I have commented it out
		//	unsigned int vt_vbo = 0;
		//	glGenBuffers (1, &vt_vbo);
		//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
		//	glBufferData (GL_ARRAY_BUFFER, monkey_head_data.mTextureCoords * sizeof (vec2), &monkey_head_data.mTextureCoords[0], GL_STATIC_DRAW);


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_data[i].mIndices.size() * sizeof(unsigned int), &mesh_data[i].mIndices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(loc1);
		glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
		glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(loc2);
		glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
		glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);


		//	This is for texture coordinates which you don't currently need, so I have commented it out
		//	glEnableVertexAttribArray (loc3);
		//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
		//	glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	}
}
#pragma endregion VBO_FUNCTIONS


void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	for(int i =0; i<n; i++)
	{
		glBindVertexArray(mesh_vao[i]);

		//Declare your uniform variables that will be used in your shader
		int matrix_location = glGetUniformLocation(shaderProgramID, "model");
		int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
		int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");


		mat4 view = identity_mat4();
		mat4 persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
		mat4 model = identity_mat4();
		
		// Translate Last
		model = translate(model, mesh_origins[i]);
		
		//Apply tranformations from the parents in the hierarchy
		for(int p = parents[i]; p != -1; p = parents[p])
		{
			model = translate(model, mesh_origins[p]);
			model = mesh_models[p] * model;
		}

		// Model Tranform first
		model =  model * mesh_models[i];
			
		view = translate(view, vec3(0.0, 0.0, -10.0f));

		// update uniforms & draw
		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);
		glDrawElements(GL_TRIANGLES, mesh_data[i].mIndices.size(), GL_UNSIGNED_INT, 0);
	}

	glutSwapBuffers();
}


void updateScene() {

	// Rotate the model slowly around the y axis at 20 degrees per second
	rotate_y += 0.2f;
	rotate_y = fmodf(rotate_y, 360.0f);

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	// load mesh into a vertex buffer array
	generateObjectBufferMesh();

}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
	if (key == 'x') {
		mesh_models[0] = translate(mesh_models[0], vec3(-0.1,0,0));
	}
	if (key == 'z') {
		mesh_models[0] = translate(mesh_models[0], vec3(0.1,0,0));
	}
	if(key == 'w') { 
		mesh_models[2] = rotate_z_deg(mesh_models[2], 5);
		mesh_models[5] = rotate_z_deg(mesh_models[5], 5);
		mesh_models[6] = rotate_z_deg(mesh_models[6], 5);	
		mesh_models[0] = translate(mesh_models[0], vec3(-0.1,0,0));

	}
	if(key == 's') { 
		mesh_models[2] = rotate_z_deg(mesh_models[2], -5);
		mesh_models[5] = rotate_z_deg(mesh_models[5], -5);
		mesh_models[6] = rotate_z_deg(mesh_models[6], -5);	
		mesh_models[0] = translate(mesh_models[0], vec3(0.1,0,0));

	}
	if(key =='a'){
		mesh_models[1] = translate(mesh_models[1], vec3(-0.1,0,0));
	} 
	if(key == 'd'){
		mesh_models[1] = translate(mesh_models[1], vec3(0.1,0,0));
	} 
	if(key == 'q'){
		mesh_models[1] = rotate_y_deg(mesh_models[1],2);
	} 
	if(key == 'e'){
		mesh_models[1] = rotate_y_deg(mesh_models[1],-2);
	} 
	if(key == '['){
		mesh_models[4] = rotate_y_deg(mesh_models[4],2);
	} 
	if(key == ']'){
		mesh_models[4] = rotate_y_deg(mesh_models[4],-2);
	} 
	
}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("Hello Triangle");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
