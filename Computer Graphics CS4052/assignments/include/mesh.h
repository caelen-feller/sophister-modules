/***
 *  Mesh Handling Class 
 */

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Mesh
{
public:
    unsigned int bufferID;
    
    // constructor generates the mesh
    // ------------------------------------------------------------------------
    
    // General Vertex array constructor
    Mesh(const float* vertices)
    {

    }

    // Default Triangle Constructor
    Mesh()
    {

    }

    Mesh(const char* vertexFile)
    {
        // TODO
    }
    
private:

};
