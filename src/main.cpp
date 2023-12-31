/*
musicvisualizer base code
by Christian Eckhardt 2018
with some code snippets from Ian Thomas Dunn and Zoe Wood, based on their CPE CSC 471 base code
On Windows, it whould capture "what you here" automatically, as long as you have the Stereo Mix turned on!! (Recording devices -> activate)
*/

#include <iostream>
#include <fstream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "WindowManager.h"
#include "Shape.h"
#include "line.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <thread>
using namespace std;
using namespace glm;
shared_ptr<Shape> shape;
shared_ptr<Shape> plane;
#define MESHSIZE 1000

ofstream file;
int renderstate = 2;
int realspeed = 0;
//********************
#include <math.h>
#include <algorithm>    


double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d, q, e, z, c;
	camera()
	{
		w = a = s = d = q = e = z = c = 0;
		pos = glm::vec3(0, 0, 0);
        rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;

		float fwdspeed = 90;
		if (realspeed)
			fwdspeed = 10;

		if (w == 1)
		{
			speed = fwdspeed*ftime;
		}
		else if (s == 1)
		{
			speed = -fwdspeed*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = 3. * ftime;
		else if(d==1)
			yangle = -3. * ftime;
		rot.y += yangle;
		float zangle = 0;
		if (q == 1)
			zangle = -ftime;
		else if (e == 1)
			zangle = ftime;
		rot.z += zangle;
		float xangle = 0;
		if (z == 1)
			xangle = -0.2 * ftime;
		else if (c == 1)
			xangle = 0.2 * ftime;
		rot.x += xangle;

		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::mat4 Rz = glm::rotate(glm::mat4(1), rot.z, glm::vec3(0, 0, 1));
		glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		R = Rz *Rx * R;
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
	void get_dirpos(vec3 &up,vec3 &dir,vec3 &position)
		{		
		position = pos;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::mat4 Rz = glm::rotate(glm::mat4(1), rot.z, glm::vec3(0, 0, 1));
		glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
		glm::vec4 dir4 = glm::vec4(0, 0, 1, 0);
		R = Rz *Rx * R;
		dir4 = dir4*R;
		dir = vec3(dir4);
		glm::vec4 up4 = glm::vec4(0, 1, 0, 0);
		up4 = R*vec4(0, 1, 0, 0);
		up4 = vec4(0, 1, 0, 0)*R;
		up = vec3(up4);
		}
	
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, heightshader, skyprog, linesshader, pplane;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint MeshPosID, MeshTexID, IndexBufferIDBox;

    // Contains vertex information for OpenGL
    GLuint VertexArrayID2;

    // Data necessary to give our box to OpenGL
    GLuint VertexBufferID2, VertexNormDBox2, VertexTexBox2, IndexBufferIDBox2;
    
	//texture data
	GLuint Texture,AudioTex, AudioTexBuf;
	GLuint Texture2,HeightTex;
    
    Line linerender_1;
    Line smoothrender_1;
    vector<vec3> line_1;
    
    Line linerender_2;
    Line smoothrender_2;
    vector<vec3> line_2;
    
    vector<float> y_rot_1;
    vector<float> y_rot_2;
    
    

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		{
			mycam.q = 1;
		}
		if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
		{
			mycam.q = 0;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS)
		{
			mycam.e = 1;
		}
		if (key == GLFW_KEY_E && action == GLFW_RELEASE)
		{
			mycam.e = 0;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		{
			mycam.z = 1;
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
		{
			mycam.z = 0;
		}
		if (key == GLFW_KEY_C && action == GLFW_PRESS)
		{
			mycam.c = 1;
		}
		if (key == GLFW_KEY_C && action == GLFW_RELEASE)
		{
			mycam.c = 0;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		{
			if (renderstate == 1)
				renderstate = 2;
			else
				renderstate = 1;
		}
        if (key == GLFW_KEY_P && action == GLFW_RELEASE)
        {
            if (smoothrender_1.is_active())
                smoothrender_1.reset();
            else
                {
                vector<vec3> splinepoints_1;
                spline(splinepoints_1, line_1, 10, 2.0);
                smoothrender_1.re_init_line(splinepoints_1);
                    
                vector<vec3> splinepoints_2;
                spline(splinepoints_2, line_2, 10, 2.0);
                smoothrender_2.re_init_line(splinepoints_2);

                }
        }
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
			{
			vec3 dir,pos,up;
			mycam.get_dirpos(up, dir, pos);
			cout << "point position:" << pos.x << "," << pos.y<< "," << pos.z << endl;
			cout << "Zbase:" << dir.x << "," << dir.y << "," << dir.z << endl;
			cout << "Ybase:" << up.x << "," << up.y << "," << up.z << endl;
			}
		if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
			{
			vec3 dir, pos, up;
			mycam.get_dirpos(up, dir, pos);
			cout << endl;
			cout << "point position:" << pos.x << "," << pos.y << "," << pos.z << endl;
			cout << "Zbase:" << dir.x << "," << dir.y << "," << dir.z << endl;
			cout << "Ybase:" << up.x << "," << up.y << "," << up.z << endl;
			cout << "point saved into file!" << endl << endl;
			file << "point position:" << pos.x << "," << pos.y << "," << pos.z << endl;
			file << "Zbase:" << dir.x << "," << dir.y << "," << dir.z << endl;
			file << "Ybase:" << up.x << "," << up.y << "," << up.z << endl;
			}
		if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE)
			{
			realspeed = !realspeed;
			}
		
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
//		double posX, posY;
//		float newPt[2];
//		if (action == GLFW_PRESS)
//		{
//			glfwGetCursorPos(window, &posX, &posY);
//			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;
//
//			//change this to be the points converted to WORLD
//			//THIS IS BROKEN< YOU GET TO FIX IT - yay!
//			newPt[0] = 0;
//			newPt[1] = 0;
//
//			std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
//			glBindBuffer(GL_ARRAY_BUFFER, MeshPosID);
//			//update the vertex array with the updated points
//			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*6, sizeof(float)*2, newPt);
//			glBindBuffer(GL_ARRAY_BUFFER, 0);
//		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	void init_mesh()
	{

		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &MeshPosID);
		glBindBuffer(GL_ARRAY_BUFFER, MeshPosID);
		glm::vec3 *vertices = new glm::vec3[MESHSIZE * MESHSIZE * 6];
		for (int x = 0; x < MESHSIZE; x++)
		{
			for (int z = 0; z < MESHSIZE; z++)
			{
				vertices[x * 6 + z*MESHSIZE * 6 + 0] = vec3(0.0, 0.0, 0.0) + vec3(x, 0, z);//LD
				vertices[x * 6 + z*MESHSIZE * 6 + 1] = vec3(1.0, 0.0, 0.0) + vec3(x, 0, z);//RD
				vertices[x * 6 + z*MESHSIZE * 6 + 2] = vec3(1.0, 0.0, 1.0) + vec3(x, 0, z);//RU
				vertices[x * 6 + z*MESHSIZE * 6 + 3] = vec3(0.0, 0.0, 0.0) + vec3(x, 0, z);//LD
				vertices[x * 6 + z*MESHSIZE * 6 + 4] = vec3(1.0, 0.0, 1.0) + vec3(x, 0, z);//RU
				vertices[x * 6 + z*MESHSIZE * 6 + 5] = vec3(0.0, 0.0, 1.0) + vec3(x, 0, z);//LU

			}
	
		}
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * MESHSIZE * MESHSIZE * 6, vertices, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		delete[] vertices;
		//tex coords
		float t = 1. / MESHSIZE;
		vec2 *tex = new vec2[MESHSIZE * MESHSIZE * 6];
		for (int x = 0; x<MESHSIZE; x++)
			for (int y = 0; y < MESHSIZE; y++)
			{
				tex[x * 6 + y*MESHSIZE * 6 + 0] = vec2(0.0, 0.0)+ vec2(x, y)*t;	//LD
				tex[x * 6 + y*MESHSIZE * 6 + 1] = vec2(t, 0.0)+ vec2(x, y)*t;	//RD
				tex[x * 6 + y*MESHSIZE * 6 + 2] = vec2(t, t)+ vec2(x, y)*t;		//RU
				tex[x * 6 + y*MESHSIZE * 6 + 3] = vec2(0.0, 0.0) + vec2(x, y)*t;	//LD
				tex[x * 6 + y*MESHSIZE * 6 + 4] = vec2(t, t) + vec2(x, y)*t;		//RU
				tex[x * 6 + y*MESHSIZE * 6 + 5] = vec2(0.0, t)+ vec2(x, y)*t;	//LU
			}
		glGenBuffers(1, &MeshTexID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, MeshTexID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * MESHSIZE * MESHSIZE * 6, tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		delete[] tex;
		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLuint *elements = new GLuint[MESHSIZE * MESHSIZE * 8];
	//	GLuint ele[10000];
		int ind = 0,i=0;
		for (i = 0; i<(MESHSIZE * MESHSIZE * 8); i+=8, ind+=6)
			{
			elements[i + 0] = ind + 0;
			elements[i + 1] = ind + 1;
			elements[i + 2] = ind + 1;
			elements[i + 3] = ind + 2;
			elements[i + 4] = ind + 2;
			elements[i + 5] = ind + 5;
			elements[i + 6] = ind + 5;
			elements[i + 7] = ind + 0;
			}			
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*MESHSIZE * MESHSIZE * 8, elements, GL_STATIC_DRAW);
		delete[] elements;
		glBindVertexArray(0);
	}
	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		//initialize the net mesh
		init_mesh();

		string resourceDirectory = "../../resources" ;
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();

        plane = make_shared<Shape>();
        string mtldir = resourceDirectory + "/FA-18E_SuperHornet/";
        plane->loadMesh(resourceDirectory + "/FA-18E_SuperHornet/FA-18E_SuperHornet.obj", &mtldir, stbi_load);
        plane->resize();
        plane->init();
        
        //generate the VAO
        glGenVertexArrays(1, &VertexArrayID2);
        glBindVertexArray(VertexArrayID2);

        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &VertexBufferID2);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID2);

        GLfloat cube_vertices[] = {
            // front
            -1.0, -1.0,  1.0,//LD
            1.0, -1.0,  1.0,//RD
            1.0,  1.0,  1.0,//RU
            -1.0,  1.0,  1.0,//LU
        };
        //make it a bit smaller
        for (int i = 0; i < 12; i++)
            cube_vertices[i] *= 0.5;
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);

        //we need to set up the vertex array
        glEnableVertexAttribArray(1);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        //color
        GLfloat cube_norm[] = {
            // front colors
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,

        };
        glGenBuffers(1, &VertexNormDBox2);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexNormDBox2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norm), cube_norm, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        //color
        glm::vec2 cube_tex[] = {
            // front colors
            glm::vec2(0.0, 1.0),
            glm::vec2(1.0, 1.0),
            glm::vec2(1.0, 0.0),
            glm::vec2(0.0, 0.0),

        };
        glGenBuffers(1, &VertexTexBox2);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexTexBox2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex), cube_tex, GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glGenBuffers(1, &IndexBufferIDBox2);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox2);
        GLushort cube_elements[] = {

            // front
            0, 1, 2,
            2, 3, 0,
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

        
        glBindVertexArray(0);
        
		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/sky.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//texture 2
		str = resourceDirectory + "/sky1.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture 3
		str = resourceDirectory + "/height.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &HeightTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, HeightTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);


		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		Tex1Location = glGetUniformLocation(heightshader->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(heightshader->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(heightshader->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		Tex1Location = glGetUniformLocation(skyprog->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(skyprog->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(skyprog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);
		
		Tex1Location = glGetUniformLocation(linesshader->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(linesshader->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(linesshader->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);
        


		// dynamic audio texture
		
/*
		int datasize = TEXSIZE *TEXSIZE * 4 * sizeof(GLfloat);
		glGenBuffers(1, &AudioTexBuf);
		glBindBuffer(GL_TEXTURE_BUFFER, AudioTexBuf);
		glBufferData(GL_TEXTURE_BUFFER, datasize, NULL, GL_DYNAMIC_COPY);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		glGenTextures(1, &AudioTex);
		glBindTexture(GL_TEXTURE_BUFFER, AudioTex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, AudioTexBuf);
		glBindTexture(GL_TEXTURE_BUFFER, 0);
		//glBindImageTexture(2, AudioTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);
		*/
        
        // plane 1
        smoothrender_1.init();
        linerender_1.init();
        
        line_1.push_back(vec3(3.0f, 26.0f, 10.0f));
        line_1.push_back(vec3(3.0f, 24.0f, 0.0f));
        line_1.push_back(vec3(3.0f, 20.0f, -20.0f));
        line_1.push_back(vec3(7.0f, 24.0f, -35.0f));
        line_1.push_back(vec3(3.0f, 28.0f, -50.0f));
        line_1.push_back(vec3(-1.0f, 24.0f, -65.0f));
        line_1.push_back(vec3(3.0f, 20.0f, -80.0f));
        line_1.push_back(vec3(3.0f, 20.0f, -95.0f));
        line_1.push_back(vec3(3.0f, 20.0f, -110.0f));
        line_1.push_back(vec3(3.0f, 18.0f, -125.0f));
        line_1.push_back(vec3(3.0f, 20.0f, -140.0f));
        line_1.push_back(vec3(3.0f, 22.0f, -155.0f));
        line_1.push_back(vec3(3.0f, 20.0f, -170.0f));
        line_1.push_back(vec3(3.0f, 18.0f, -185.0f));
        line_1.push_back(vec3(1.0f, 20.0f, -200.0f));
        line_1.push_back(vec3(1.0f, 20.0f, -215.0f));
        line_1.push_back(vec3(1.0f, 24.0f, -230.0f));
        line_1.push_back(vec3(1.0f, 28.0f, -245.0f));
        line_1.push_back(vec3(1.0f, 24.0f, -260.0f));
        line_1.push_back(vec3(0.0f, 20.0f, -275.0f));
        line_1.push_back(vec3(-0.5f, 22.0f, -290.0f));
        line_1.push_back(vec3(-1.0f, 24.0f, -305.0f));
        line_1.push_back(vec3(-1.0f, 60.0f, -320.0f));
        line_1.push_back(vec3(-1.0f, 100.0f, -335.0f));

        linerender_1.re_init_line(line_1);
        
        y_rot_1.push_back(0.0f);
        y_rot_1.push_back(20.0f);
        y_rot_1.push_back(40.0f);
        y_rot_1.push_back(20.0f);
        y_rot_1.push_back(-150.0f);
        y_rot_1.push_back(-180.0f);
        y_rot_1.push_back(-270.0f);
        y_rot_1.push_back(-315.0f);
        y_rot_1.push_back(-360.0f);
        y_rot_1.push_back(-270.0f);
        y_rot_1.push_back(-180.0f);
        y_rot_1.push_back(-90.0f);
        y_rot_1.push_back(0.0f);
        y_rot_1.push_back(0.0f);
        y_rot_1.push_back(0.0f);
        y_rot_1.push_back(0.0f);
        y_rot_1.push_back(0.0f);
        y_rot_1.push_back(0.0f);
        y_rot_1.push_back(0.0f);
        y_rot_1.push_back(9.0f);
        y_rot_1.push_back(0.0f);
        y_rot_1.push_back(0.0f);
        y_rot_1.push_back(0.0f);
        y_rot_1.push_back(0.0f);
        y_rot_1.push_back(0.0f);
        y_rot_1.push_back(0.0f);
        
        // plane 2
        smoothrender_2.init();
        linerender_2.init();
        
        line_2.push_back(vec3(-3.0f, 26.0f, 10.0f));
        line_2.push_back(vec3(-3.0f, 24.0f, -5.0f));
        line_2.push_back(vec3(-3.0f, 20.0f, -15.0f));
        line_2.push_back(vec3(-7.0f, 24.0f, -30.0f));
        line_2.push_back(vec3(-3.0f, 28.0f, -45.0f));
        line_2.push_back(vec3(1.0f, 24.0f, -60.0f));
        line_2.push_back(vec3(-3.0f, 20.0f, -80.0f));
        line_2.push_back(vec3(-3.0f, 20.0f, -95.0f));
        line_2.push_back(vec3(-3.0f, 20.0f, -110.0f));
        line_2.push_back(vec3(-3.0f, 22.0f, -125.0f));
        line_2.push_back(vec3(-3.0f, 20.0f, -140.0f));
        line_2.push_back(vec3(-3.0f, 18.0f, -155.0f));
        line_2.push_back(vec3(-3.0f, 20.0f, -170.0f));
        line_2.push_back(vec3(-3.0f, 22.0f, -185.0f));
        line_2.push_back(vec3(-1.0f, 24.0f, -200.0f));
        line_2.push_back(vec3(-1.0f, 28.0f, -215.0f));
        line_2.push_back(vec3(-1.0f, 24.0f, -230.0f));
        line_2.push_back(vec3(-1.0f, 20.0f, -245.0f));
        line_2.push_back(vec3(-1.0f, 24.0f, -260.0f));
        line_2.push_back(vec3(0.0f, 28.0f, -275.0f));
        line_2.push_back(vec3(0.5f, 26.0f, -290.0f));
        line_2.push_back(vec3(1.0f, 24.0f, -305.0f));
        line_2.push_back(vec3(1.0f, 60.0f, -320.0f));
        line_2.push_back(vec3(1.0f, 100.0f, -335.0f));
        
        y_rot_2.push_back(0.0f);
        y_rot_2.push_back(-20.0f);
        y_rot_2.push_back(-40.0f);
        y_rot_2.push_back(-20.0f);
        y_rot_2.push_back(150.0f);
        y_rot_2.push_back(180.0f);
        y_rot_2.push_back(270.0f);
        y_rot_2.push_back(315.0f);
        y_rot_2.push_back(360.0f);
        y_rot_2.push_back(270.0f);
        y_rot_2.push_back(180.0f);
        y_rot_2.push_back(90.0f);
        y_rot_2.push_back(0.0f);
        y_rot_2.push_back(0.0f);
        y_rot_2.push_back(0.0f);
        y_rot_2.push_back(0.0f);
        y_rot_2.push_back(-90.0f);
        y_rot_2.push_back(-180.0f);
        y_rot_2.push_back(-180.0f);
        y_rot_2.push_back(-90.0f);
        y_rot_2.push_back(-180.0f);
        y_rot_2.push_back(-180.0f);
        y_rot_2.push_back(-180.0f);
        y_rot_2.push_back(-180.0f);
        y_rot_2.push_back(-90.0f);
        y_rot_2.push_back(0.0f);
        

        linerender_2.re_init_line(line_2);
	

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}
	
	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		

		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		// Initialize the GLSL program.
		skyprog = std::make_shared<Program>();
		skyprog->setVerbose(true);
		skyprog->setShaderNames(resourceDirectory + "/sky_vertex.glsl", resourceDirectory + "/sky_fragment.glsl");
		if (!skyprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		skyprog->addUniform("P");
		skyprog->addUniform("V");
		skyprog->addUniform("M");
		skyprog->addAttribute("vertPos");
		skyprog->addAttribute("vertTex");

		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");

		// Initialize the GLSL program.
		heightshader = std::make_shared<Program>();
		heightshader->setVerbose(true);
		heightshader->setShaderNames(resourceDirectory + "/height_vertex.glsl", resourceDirectory + "/height_frag.glsl", resourceDirectory + "/geometry.glsl");
		if (!heightshader->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		heightshader->addUniform("P");
		heightshader->addUniform("V");
		heightshader->addUniform("M");
		heightshader->addUniform("camoff");
		heightshader->addUniform("campos");
		heightshader->addAttribute("vertPos");
		heightshader->addAttribute("vertTex");
		heightshader->addUniform("bgcolor");
		heightshader->addUniform("renderstate");
        heightshader->addUniform("heightScale");

		// Initialize the GLSL program.
		linesshader = std::make_shared<Program>();
		linesshader->setVerbose(true);
		linesshader->setShaderNames(resourceDirectory + "/lines_height_vertex.glsl", resourceDirectory + "/lines_height_frag.glsl", resourceDirectory + "/lines_geometry.glsl");
		if (!linesshader->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		linesshader->addUniform("P");
		linesshader->addUniform("V");
		linesshader->addUniform("M");
		linesshader->addUniform("camoff");
		linesshader->addUniform("campos");
		linesshader->addAttribute("vertPos");
		linesshader->addAttribute("vertTex");
		linesshader->addUniform("bgcolor");
        
        pplane = std::make_shared<Program>();
        pplane->setVerbose(true);
        pplane->setShaderNames(resourceDirectory + "/plane_vertex.glsl", resourceDirectory + "/plane_frag.glsl");
        if (!pplane->init())
            {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
            }
        pplane->addUniform("P");
        pplane->addUniform("V");
        pplane->addUniform("M");
        pplane->addUniform("campos");
        pplane->addAttribute("vertPos");
        pplane->addAttribute("vertNor");
        pplane->addAttribute("vertTex");
		
	}
    
    vec3 mid_point(vec3 a, vec3 b){
        vec3 res = (a + b) * 0.5f;
        res.z += 10.0f;
        return res;
    }
    
    vec3 mid_point_2(vec3 a, vec3 b, float ratio){
        vec3 res = (a + b) * 0.5f;
        res.z -= 30.0f * ratio;
        return res;
    }
    
    template <class T>
    T cos_interp(float t, T a, T b){
        float ct = 1.0f - (cos(t * 3.1415926)+ 1.0f) / 2.0f;
        return a * (1-ct) + b*ct;
    }
    
    float cos_interp_float(float t){
        return 1.0f - (cos(t * 3.1415926)+ 1.0f) / 2.0f;
    }
    
    vec3 linear_interp_points(vector<vec3> & points, float total_time, float curTime, vector<float> ey, mat4 &rot, int &cur_point){
        vector<float>distances(points.size(), 0.0f);
        float total_distance = 0;
        for (int i = 0; i < points.size() - 1; i++) {
            distances[i] = glm::distance(points[i], points[i+1]);
            total_distance += distances[i];
        }
        
        float speed = total_distance/total_time;
        
        float distance_from_start = curTime * speed;
        int cur_index = 0;
        
        while (cur_index < points.size() - 2 && distance_from_start > distances[cur_index]){
            distance_from_start -= distances[cur_index];
            cur_index ++;
        }
        
        cur_point = cur_index;
        
        float ratio = curTime/total_time;
        
        float interp_factor = distance_from_start / distances[cur_index];
        
        // ey
        int i1 = (int) (ratio * (ey.size() - 1));
        int i2 = i1 + 1;
        if (i2 >= ey.size()) i2 = static_cast<int>(ey.size()) - 1;
        float ey1 = ey[i1];
        float ey2 = ey[i2];
        
        float unit_time = total_time/(ey.size()-1);
        float cur_ratio = curTime - (unit_time * i1);
        float rot_factor = cos_interp_float(cur_ratio / unit_time);
        
        vec3 p0 = points[cur_index-1];
        vec3 p1 = points[cur_index];
        vec3 p2 = points[cur_index+1];
        
        vec3 up = vec3(0,1,0);
        
        vec3 direction0 = normalize(p1-p0);
        vec3 direction = normalize(p2-p1);
        vec3 forward = cos_interp(interp_factor, direction0, direction);
        
        vec3 ex = cross(forward, up);
        vec3 new_up = cross(forward, ex);
        
        mat3 rotation_matrix(ex, new_up, forward);
        
        glm::mat4 matrix;
        matrix[0] = glm::vec4(rotation_matrix[0], 0.0f);
        matrix[1] = glm::vec4(rotation_matrix[1], 0.0f);
        matrix[2] = glm::vec4(rotation_matrix[2], 0.0f);
        matrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        
        quat q = quat_cast(matrix);
        q = angleAxis(radians(mix(ey1,ey2,rot_factor)), forward) * q;
        rot = mat4_cast(q);
    
        return mix(p1,p2,interp_factor);
    }

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		static double count = 0;
		double frametime = get_last_elapsed_time();
		count += frametime;
	
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClearColor(0.8f, 0.8f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V,Vi, M, P; //View, Model and Perspective matrix
		
		V = mycam.process(frametime);
		Vi = glm::inverse(V);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 10000.0f);		
		if (width < height)
			{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect,  1.0f / aspect, -2.0f, 10000.0f);
			}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.01f, 100000.0f); //so much type casting... GLM metods are quite funny ones
		
		// Draw the skybox --------------------------------------------------------------
		skyprog->bind();		
		static float w = 0.0;
		w += 1.0 * frametime;//rotation angle
		float trans = 0;// sin(t) * 2;
		w = 0.6;
		glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = 3.1415926 / 2.0;
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), -mycam.pos);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));
		M = TransZ *RotateY * RotateX * S;
		glUniformMatrix4fv(skyprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(skyprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(skyprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glActiveTexture(GL_TEXTURE0);
		if(renderstate==1)
			glBindTexture(GL_TEXTURE_2D, Texture);
		else if (renderstate == 2)
			glBindTexture(GL_TEXTURE_2D, Texture2);		
		glDisable(GL_DEPTH_TEST);
		shape->draw(skyprog);
		glEnable(GL_DEPTH_TEST);
        
        //-----------------------------------------------
        
        mat4 rot_1;
        mat4 rot_2;
        mat4 rot_3;
        int cur_point_1 = 0;
        int cur_point_2 = 0;
        float sangle = 3.1415926 / 2.;
        static float curTime = 0;
        float total_time = 70.0f;
        glm::mat4 SPlane = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
        glm::mat4 RotateXPlane = glm::rotate(glm::mat4(1.0f), sangle, vec3(1,0,0));
        
        // plane 1
        pplane->bind();
        vector<vec3> splinepoints_1;
        spline(splinepoints_1, line_1, 10, 2.0);
        
        vec3 curPos_1 = linear_interp_points(splinepoints_1, total_time, curTime, y_rot_1, rot_1, cur_point_1);
        curTime += frametime;

        // Draw the plane using GLSL.
        glm::mat4 TransPlane_1 = glm::translate(glm::mat4(1.0f), curPos_1);
        
        M = TransPlane_1 * rot_1 * RotateXPlane;

        glUniformMatrix4fv(pplane->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(pplane->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(pplane->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(pplane->getUniform("campos"), 1, &mycam.pos[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        plane->draw(pplane);            //render!!!!!!!
        
        // plane 2
        vector<vec3> splinepoints_2;
        spline(splinepoints_2, line_2, 10, 2.0);
        
        vec3 curPos_2 = linear_interp_points(splinepoints_2, total_time, curTime, y_rot_1, rot_2, cur_point_2);
        curTime += frametime;
        
        printf("%f\n",curTime);

        // Draw the plane using GLSL.
        glm::mat4 TransPlane_2 = glm::translate(glm::mat4(1.0f), curPos_2);
        
        M = TransPlane_2 * rot_2 * RotateXPlane;
        glUniformMatrix4fv(pplane->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(pplane->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(pplane->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(pplane->getUniform("campos"), 1, &mycam.pos[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        plane->draw(pplane);            //render!!!!!!!

        // Interpolate camera position towards the target position
        mycam.pos = -mid_point(curPos_1, curPos_2) + sin(curTime);
        
        if (curTime > 18.0f && curTime < 35.0f){
            mycam.pos = -mid_point_2(curPos_1, curPos_2, curTime/total_time) + sin(curTime);
            if (curTime > 18.0f && curTime < 20.1f)
                mycam.a = 1;
            else if (curTime > 20.0f && curTime < 32.9f){
                mycam.a = 0;
            }
            else{
                mycam.a = 1;
            }
        }else{
            mycam.a = 0;
        }


        
        
        if (curTime > 40.0f && curTime < 52.7f){
            mycam.q = 1;
        }
        else{
            mycam.q = 0;
        }
        
        if (curTime > 53.0f){
            mycam.pos = -mid_point(curPos_1, curPos_2);
            mycam.pos.y = -24.0f;
        }
        
        
        //mycam.pos = -linear_interp_points(splinepoints_3, total_time, curTime, y_rot_3, rot_3, cur_point_2);

        //mycam.rot = eulerAngles(quat_cast(rot_3));
        
        pplane->unbind();
        
        //draw the dot
        prog->bind();
        int num;
        mat4 Trans;
        for (num = 0; num < 8; num++){
            Trans = glm::translate(glm::mat4(1.0f), splinepoints_1[cur_point_1 - num - 1]);
            if (cur_point_1 >= 9){
                S = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
                M = Trans * S;
                glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
                glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
                shape->draw(prog);
                Trans = glm::translate(glm::mat4(1.0f), (splinepoints_1[cur_point_1-num-1] + splinepoints_1[cur_point_1 - num]) * 0.5f);
                M = Trans * S;
                glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
                glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
                shape->draw(prog);
            }
            Trans = glm::translate(glm::mat4(1.0f), splinepoints_2[cur_point_2 - num - 1]);
            if (cur_point_2 >= 9){
                S = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
                M = Trans * S;
                glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
                glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
                shape->draw(prog);
                Trans = glm::translate(glm::mat4(1.0f), (splinepoints_2[cur_point_2-num-1] + splinepoints_2[cur_point_2 - num]) * 0.5f);
                M = Trans * S;
                glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
                glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
                shape->draw(prog);
            }
        }

		// Draw the terrain --------------------------------------------------------------
		heightshader->bind();
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glm::mat4 TransY = glm::translate(glm::mat4(1.0f), glm::vec3(-500.0f, -9.0f, -500));
		M = TransY;
		glUniformMatrix4fv(heightshader->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniformMatrix4fv(heightshader->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(heightshader->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		vec3 offset = mycam.pos;
		offset.y = 0;
		offset.x = (int)offset.x;
		offset.z = (int)offset.z;
		//	offset = vec3(0, 0, 0);
		vec3 bg = vec3(254. / 255., 225. / 255., 168. / 255.);
		if (renderstate == 2)
			bg = vec3(49. / 255., 88. / 255., 114. / 255.);
		glUniform3fv(heightshader->getUniform("camoff"), 1, &offset[0]);
		glUniform3fv(heightshader->getUniform("campos"), 1, &mycam.pos[0]);
		glUniform3fv(heightshader->getUniform("bgcolor"), 1, &bg[0]);
		glUniform1i(heightshader->getUniform("renderstate"), renderstate);
        glUniform1f(heightshader->getUniform("heightScale"), curTime-floor(curTime));
		glBindVertexArray(VertexArrayID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, HeightTex);
		glDrawArrays(GL_TRIANGLES, 0, MESHSIZE*MESHSIZE * 6);
		heightshader->unbind();
        
        //draw the lines

        glm::vec3 linecolor = glm::vec3(0, 1, 1);
//        linerender_1.draw(P, V, linecolor);
//        linerender_2.draw(P, V, linecolor);
        smoothrender_1.draw(P, V, linecolor);
        smoothrender_2.draw(P, V, linecolor);

	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}
	file.open("pathinfo.txt");
	if (!file.is_open())
		{
		cout << "warning! could not open pathinfo.txt file!" << endl;
		}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	file.close();

	// Quit program.
	windowManager->shutdown();
	return 0;
}
