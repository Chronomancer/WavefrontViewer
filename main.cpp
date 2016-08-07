/*
Simple Obj viewer for Windows using OpenGL and SDL
Can only read triangular faces, not polygonal, and does not draw textures

Modified code based on frank253's OpenGl Glut OBJ Loader sample
openglsamples.sourceforge.net/projects/index.pho/blog/index/
 
Wavefront loader is built on Ricardo Rendon Cepeda's OpenGL ES tutorial
raywenderlich.com/48293/

Wavefront loader also borrows elements from Lazaros Karydas's objview
github.com/lKarydas/objview

THE PROGRAM IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/

#include <windows.h> 
#include <commdlg.h>
#include <string>
#include <SDL.h>
#include <SDL_opengl.h>

#include <gl/GLU.h>

#include <iostream>

#include "wavefrontloader.h"

 
#define KEY_ESCAPE 27
 
#define SCREENWIDTH 	640
#define SCREENHEIGHT 	480
#define MAJOR_GL 		2		//using OpenGL 2 functions, fixed pipeline
#define MINOR_GL 		1

struct MyWindow
{
    int width;
	int height;
	char* title;
 
	float fovAngle;
	float zNear;
	float zFar;	
	
	SDL_Window* viewWindow;
	
	MyWindow();
}; 

void MoveCamera (int &rotX, int &rotY);
void Display(OBJClass &objmodel, bool &wireframeToggle, int &rotX, int &rotY);
void DrawAxis();
void DrawText(std::string &text, float &x, float &y, void *font);
void DrawModel(OBJClass &objmodel, bool &wireframeToggle);
void InitGL(int &width, int &height, float &fovangle, float &znear, float &zfar);

MyWindow::MyWindow()
{
	title = NULL;
	viewWindow = NULL;
	width = height = 0;
	fovAngle = zNear = zFar = 0.0f;	
}

void DrawModel(OBJClass &objmodel, bool &wireframeToggle) 
{    
	if (objmodel.GetVertexBuffer() != NULL)
	{
		if (wireframeToggle)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glColor3f(1.0f,1.0f,1.0f);	
 		glEnableClientState(GL_VERTEX_ARRAY);		// Enable vertex arrays
 	
		if (objmodel.HasNormals())
		{
			glEnableClientState(GL_NORMAL_ARRAY);		// Enable normal arrays
			//glVertexPointer(4,GL_FLOAT,	0, objmodel.GetFacesTriangles());// Vertex Pt to triangle array
			glVertexPointer(4,GL_FLOAT,	0, objmodel.GetVertexBuffer());
			glNormalPointer(GL_FLOAT, 0, objmodel.GetNormalBuffer());						// Normal pointer to normal array
			//glDrawArrays(GL_TRIANGLES, 0, objmodel.mFaceCount*3);		// Draw the triangles
			glDrawElements(GL_TRIANGLES, objmodel.GetTotalConnectTriangles(), GL_UNSIGNED_INT, objmodel.GetIndexBufferV());
			glDisableClientState(GL_NORMAL_ARRAY);		// Disable normal arrays	
		}
		else
		{
			glVertexPointer(4,GL_FLOAT,	0, objmodel.GetVertexBuffer());
			glDrawElements(GL_TRIANGLES, objmodel.GetTotalConnectTriangles(), GL_UNSIGNED_INT, objmodel.GetIndexBufferV());
		}
		glDisableClientState(GL_VERTEX_ARRAY);	// Disable vertex arrays			
	}
}

void DrawAxis()
{
	const float linewidth = 5.0f;
    glLineWidth(5.0f);
	glColor3f(1.0f,0,0);

	glBegin(GL_LINE_STRIP); //X
	glVertex3f(0.0f, 0.0f, 0.0f);    
	glVertex3f( linewidth, 0.0f, 0.0f);    
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(0.0f, 0.0f, 0.0f);    
	glVertex3f( linewidth, 0.0f, 0.0f);    
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f( linewidth - 0.2f , -0.2f, 0.0f);   
	glVertex3f( linewidth + 0.2f , 0.2f, 0.0f);  
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f( linewidth - 0.2f , 0.2f, 0.0f);    
	glVertex3f( linewidth + 0.2f , - 0.2f, 0.0f);  
	glEnd();

	///////////
	glColor3f(0.0f,1.0f,0);

	glBegin(GL_LINE_STRIP); //Y
	glVertex3f(0.0f, 0.0f, 0.0f);    
	glVertex3f( 0.0f, linewidth, 0.0f);
	glEnd();	

	////////////
	glColor3f(0.0f,0,1.0f);

	glBegin(GL_LINE_STRIP); //Z
	glVertex3f(0.0f, 0.0f, 0.0f);   
	glVertex3f( 0.0f, 0.0f, linewidth);
	glEnd();

	glBegin(GL_LINE_STRIP); 
	glVertex3f( 0.0f, -0.2f, linewidth + 0.2f);    
	glVertex3f( 0.0f, 0.2f, linewidth - 0.2f);
	glEnd();

	glBegin(GL_LINE_STRIP); 
	glVertex3f( 0.0f, -0.2f, linewidth -0.2f);    
	glVertex3f( 0.0f, -0.2f, linewidth + 0.2f);
	glEnd();

	glBegin(GL_LINE_STRIP); 
	glVertex3f( 0.0f, 0.2f, linewidth -0.2f);    
	glVertex3f( 0.0f, 0.2f, linewidth +0.2f);
	glEnd();

	glLineWidth(1.0f);
}

void MoveCamera (int &rotX, int &rotY) 
{
    glRotatef( (GLfloat)rotX,1.0f,0.0f,0.0f);  //rotate our camera on teh x-axis (left and right)
    glRotatef( (GLfloat)rotY,0.0f,1.0f,0.0f);  //rotate our camera on the y-axis (up and down)
}

void Display(OBJClass &objmodel, bool &wireframeToggle, int &rotX, int &rotY) 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt( 10,3,10, 0, 0, 0, 0, 1, 0);
	glPushMatrix();
		
	MoveCamera(rotX, rotY);
	DrawAxis();

	glColor3f(1.0f,1.0f,1.0f);
	glLineWidth(1.0f);
	
	DrawModel(objmodel, wireframeToggle);
	
	glPopMatrix();
} 

//setting up matrices, lights, shading, etc
void InitGL(int &width, int &height, float &fovangle, float &znear, float &zfar) 
{
    glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, width, height);
	GLfloat aspect = (GLfloat) width/height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(fovangle, aspect, znear, zfar);
    glMatrixMode(GL_MODELVIEW);
    glShadeModel( GL_SMOOTH );
    glClearColor( 0.0f, 1.0f, 0.5f, 0.5f );
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
 
    GLfloat amb_light[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat diffuse[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    GLfloat specular[] = { 0.7f, 0.7f, 0.3f, 1.0f };
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, amb_light );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
    glLightfv( GL_LIGHT0, GL_SPECULAR, specular );
    glEnable( GL_LIGHT0 );
    glEnable( GL_COLOR_MATERIAL );
    glShadeModel( GL_SMOOTH );
    glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );
    glDepthFunc( GL_LEQUAL );
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
 
int main(int argc, char *argv[])
{
	bool boolToExit = false;
	SDL_Event event;
	SDL_GLContext context;
	const unsigned char *version;
	
	bool isRotatingCamera = false, wireframeToggle = false;	
	int mousePosition[2] = {0, 0};
	int mouseDiff[2] = {0, 0};
	int rotation[2] = {0, 0};
	
	OPENFILENAME opdlg = {0}; //ZeroMemory(&opdlg, sizeof(opdlg)); 
	wchar_t fileName[250];
	const wchar_t filter[] = L"OBJ Files\0*.obj\0All Files\0*.*\0";
	MyWindow window1;
	OBJClass obj;
	
	opdlg.lStructSize = sizeof(opdlg);
	opdlg.hwndOwner = GetForegroundWindow(); //=NULL;
	opdlg.lpstrFile = fileName;
	opdlg.lpstrFile[0] = '\0';
	opdlg.nMaxFile = sizeof(fileName);
	opdlg.lpstrFilter = filter;
	opdlg.nFilterIndex = 1;
	opdlg.lpstrFileTitle = NULL;
	opdlg.nMaxFileTitle = 0;
	opdlg.lpstrInitialDir = NULL;
	opdlg.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
	
	if (!GetOpenFileName(&opdlg)) //using windows-specific file menu
	{
		std::cerr << "Can't open file name" << std::endl;
		return 1;
	}
		
	if (obj.Load(fileName) == -1)	
	{
		obj.Release();
		std::cerr << "Model incomplete" << std::endl;
		return 1;
	}		
    
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) 
	{
		obj.Release();
		std::cerr << "There was an error initing SDL2: " << SDL_GetError() << std::endl;
		return 1;
	}
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, MAJOR_GL);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, MINOR_GL);
	
	window1.width = SCREENWIDTH;
	window1.height = SCREENHEIGHT;
	window1.title = "OpenGL+SDL OBJ Viewer."; 
	window1.fovAngle = 45;
	window1.zNear = 1.0f;
	window1.zFar = 500.0f;	

	window1.viewWindow = SDL_CreateWindow(window1.title, 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		window1.width, window1.height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
 
	if (window1.viewWindow == NULL) 
	{
		obj.Release();
		std::cerr << "There was an error creating the window: " << SDL_GetError() << std::endl;
		return 1;
	}
	
	context = SDL_GL_CreateContext(window1.viewWindow);
	if (context == NULL) 
	{
		obj.Release();
		SDL_DestroyWindow(window1.viewWindow);
		std::cerr << "There was an error creating OpenGL context: " << SDL_GetError() << std::endl;
		return 1;
	}
	
	version = glGetString(GL_VERSION);
	if (version == NULL) 
	{
		obj.Release();
		SDL_DestroyWindow(window1.viewWindow);
		std::cerr << "There was an error with OpenGL configuration:" << std::endl;
		return 1;
	}
  
	SDL_GL_MakeCurrent(window1.viewWindow, context);
 
	InitGL(window1.width, window1.height, window1.fovAngle, window1.zNear, window1.zFar);
	
	//drawing on the 1st frame, 
	//only redraw when rotating camera, setting wireframe mode, resetting camera, and restoring window
	Display(obj, wireframeToggle, rotation[0], rotation[1]);
	SDL_GL_SwapWindow(window1.viewWindow);	
	
	while (!boolToExit)
	{
		while (SDL_PollEvent(&event)) 
		{
			switch (event.type)
			{
				case SDL_QUIT:
					boolToExit = true;
					break;
				
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) 
					{
						// case SDLK_LEFT:
							// key_left = false;
							// break;
						default:							
							break;	
			        }
					break;
					
				case SDL_KEYUP:
					break;
					
				case SDL_MOUSEMOTION:
					if (isRotatingCamera) //if left mouse button is down + mouse moves, rotate camera and redraw the scene
					{
						mouseDiff[0] = event.motion.y - mousePosition[1];
						mouseDiff[1] = event.motion.x - mousePosition[0]; //mouse left and right affect y axis rotation
						
						rotation[0] += mouseDiff[0]; //set the xrot to xrot with the addition	 of the difference in the y position
						rotation[1] += mouseDiff[1];    //set the xrot to yrot with the addition	 of the difference in the x position
						if (rotation[0] < -360) rotation[0] += 360;
						if (rotation[0] > 360) rotation[0] -= 360;
						if (rotation[1] >360)	rotation[1] -= 360;
						if (rotation[1] <-360)	rotation[1] += 360;
						
						Display(obj, wireframeToggle, rotation[0], rotation[1]);
						SDL_GL_SwapWindow(window1.viewWindow);
					}					
				
					mousePosition[0] = event.motion.x;
			        mousePosition[1] = event.motion.y;
					
					break;
					
				case SDL_MOUSEBUTTONDOWN:
					switch (event.button.button) 
					{
						case SDL_BUTTON_LEFT: //left button for rotating camera
							isRotatingCamera = true;
							SDL_GetMouseState(&mousePosition[0], &mousePosition[1]);
							break;
						case SDL_BUTTON_MIDDLE: //middle button for wireframe
							wireframeToggle = !wireframeToggle;

							Display(obj, wireframeToggle, rotation[0], rotation[1]);
							SDL_GL_SwapWindow(window1.viewWindow);
							break;
						case SDL_BUTTON_RIGHT: //right button for resetting camera
							rotation[0] = rotation[1] = 0;
							
							Display(obj, wireframeToggle, rotation[0], rotation[1]);
							SDL_GL_SwapWindow(window1.viewWindow);							
							break;
						default:							
							break;
			        }					
					break;
					
				case SDL_MOUSEBUTTONUP:
					isRotatingCamera = false;
					//SDL_GetMouseState(&mousePosition[0], &mousePosition[1]);
					break;
					
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_RESTORED)
					{
						Display(obj, wireframeToggle, rotation[0], rotation[1]);
						SDL_GL_SwapWindow(window1.viewWindow);
					}
					break;
					
				default:
					break;
			}		
		}			
	}
	
	SDL_StopTextInput();

	obj.Release();	
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window1.viewWindow);
	SDL_Quit();
 
    return 0;
}
