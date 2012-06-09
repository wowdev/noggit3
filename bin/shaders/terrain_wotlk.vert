#version 120

varying vec3 vertex_light_position;
varying vec3 vertex_normal;

void main() 
{
    	vertex_normal = normalize(gl_NormalMatrix * gl_Normal);

        vertex_light_position = normalize(gl_LightSource[0].position.xyz); 
	
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
