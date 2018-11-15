#version 330                                                                  
                                                                              
in vec3 vPosition;															  
in vec4 vColor;																  
out vec4 color;																 
uniform mat4 mM;
uniform mat4 mV;
uniform mat4 mP;																  
uniform	vec3 vTriColour;

void main()                                                                     
{                                                                                
	gl_Position =  mP * mV * mM * vec4(vPosition, 1.0);  
	color = vec4(vTriColour,1.0);					
}