#include <stdlib.h>
#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <tinyxml2.h>

#ifdef __APPLE__
#else
#include <GL/glut.h>
#endif

using namespace std;
using namespace tinyxml2;


struct Vertex {
    float x, y, z;
};

struct Camera {
    float posX, posY, posZ;
    float lookAtX, lookAtY, lookAtZ;
    float upX, upY, upZ;
    float fov, nearPlane, farPlane;
    float angleAlfa = 45, angleBeta = 45, radius = 10;
};


list<Vertex> vertices;
int windowWidth = 800;
int windowHeight = 600;
Camera camera;

/**
 * Load a .3d model file
 */
bool loadModel(const char* filename, list<Vertex>& verts) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open model file " << filename << endl;
        return false;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        // Draw all vertex
        // TODO : Maybe add number of vertices of figure to avoid errors
        // (For example the figure missing 1 point and it loads a new one so the point connect)
        Vertex v;
        if (iss >> v.x >> v.y >> v.z) {
            verts.push_back(v);
        }
    }

    file.close();
    cout << "Loaded model: " << filename << " (" << verts.size() << " vertices)" << endl;
    return true;
}

/**
 * Load XML configuration file
 */
void loadConfigs(const char* filename) {
    XMLDocument doc;

    // Load the file
    if (doc.LoadFile(filename) != XML_SUCCESS) {
        cerr << "Error loading XML file: " << filename << endl;
        return;
    }

    // Get the world element
    XMLElement* root = doc.FirstChildElement("world");
    if (!root) {
        cerr << "Error: No 'world' element found in XML" << endl;
        return;
    }

    // Load window configuration
    XMLElement* window = root->FirstChildElement("window");
    if (window) {
        windowWidth = window->IntAttribute("width", 800);
        windowHeight = window->IntAttribute("height", 600);
        cout << "Window size: " << windowWidth << "x" << windowHeight << endl;
    }

    // Load camera configuration
    XMLElement* cameraElem = root->FirstChildElement("camera");
    if (cameraElem) {
        // Position
        XMLElement* position = cameraElem->FirstChildElement("position");
        if (position) {
            camera.posX = position->FloatAttribute("x", 10.0f);
            camera.posY = position->FloatAttribute("y", 10.0f);
            camera.posZ = position->FloatAttribute("z", 10.0f);
        }

        // LookAt
        XMLElement* lookAt = cameraElem->FirstChildElement("lookAt");
        if (lookAt) {
            camera.lookAtX = lookAt->FloatAttribute("x", 0.0f);
            camera.lookAtY = lookAt->FloatAttribute("y", 0.0f);
            camera.lookAtZ = lookAt->FloatAttribute("z", 0.0f);
        }

        // Up
        XMLElement* up = cameraElem->FirstChildElement("up");
        if (up) {
            camera.upX = up->FloatAttribute("x", 0.0f);
            camera.upY = up->FloatAttribute("y", 1.0f);
            camera.upZ = up->FloatAttribute("z", 0.0f);
        }

        // Projection
        XMLElement* projection = cameraElem->FirstChildElement("projection");
        if (projection) {
            camera.fov = projection->FloatAttribute("fov", 60.0f);
            camera.nearPlane = projection->FloatAttribute("near", 1.0f);
            camera.farPlane = projection->FloatAttribute("far", 1000.0f);
        }

        cout << "Camera loaded: pos(" << camera.posX << ", " << camera.posY << ", " << camera.posZ << ")" << endl;
    }

    // Load models from the group
    XMLElement* group = root->FirstChildElement("group");
    if (group) {
        XMLElement* models = group->FirstChildElement("models");
        if (models) {
            XMLElement* model = models->FirstChildElement("model");
            while (model != nullptr) {
                const char* file = model->Attribute("file");
                if (file) {
                    // Build full path: ../figures/filename
                    string modelPath = "../../figures/";
                    modelPath += file;
                    loadModel(modelPath.c_str(), vertices);
                }
                model = model->NextSiblingElement("model");
            }
        }
    }

    cout << "Configuration loaded successfully!" << endl;
}

// ============================================================================
// RENDERING
// ============================================================================

void changeSize(int w, int h) {
	// Prevent a divide by zero, when window is too short
	if(h == 0)
		h = 1;

	// compute window's aspect ratio 
	float ratio = w * 1.0 / h;

	// Set the projection matrix as current
	glMatrixMode(GL_PROJECTION);
	// Load Identity Matrix
	glLoadIdentity();
	
	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	// Set perspective using camera settings
	gluPerspective(camera.fov, ratio, camera.nearPlane, camera.farPlane);

	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}

void renderScene(void) {
	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera using loaded configuration
	glLoadIdentity();
    camera.posX = sin(camera.angleAlfa * M_PI / 180.0f) * cos(camera.angleBeta * M_PI / 180.0f) * camera.radius;
	camera.posZ = cos(camera.angleAlfa * M_PI / 180.0f) * cos(camera.angleBeta * M_PI / 180.0f) * camera.radius;
	camera.posY = sin(camera.angleBeta * M_PI / 180.0f) * camera.radius;
	gluLookAt(camera.posX, camera.posY, camera.posZ, 
		      camera.lookAtX, camera.lookAtY, camera.lookAtZ,
			  camera.upX, camera.upY, camera.upZ);

	// Draw axes for reference
    glBegin(GL_LINES);
        // X axis - Red
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-100.0f, 0.0f, 0.0f);
        glVertex3f(100.0f, 0.0f, 0.0f);
        // Y axis - Green
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, -100.0f, 0.0f);
        glVertex3f(0.0f, 100.0f, 0.0f);
        // Z axis - Blue
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, -100.0f);
        glVertex3f(0.0f, 0.0f, 100.0f);
    glEnd();

    // Draw loaded models
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    for (const auto& v : vertices) {
        glVertex3f(v.x, v.y, v.z);
    }
    glEnd();

	// End of frame
	glutSwapBuffers();
}


// ============================================================================
// KEY PROCESSING
// ============================================================================

void processKeys(unsigned char c, int xx, int yy) {
	if (c == 'w'){
		if(camera.angleBeta < 90.0f){
			camera.angleBeta += 15.0f;

		}
	}
	if (c == 's'){
		if(camera.angleBeta > -90.0f){
			camera.angleBeta -= 15.0f;
		}
	}
	if (c == 'a'){
		camera.angleAlfa -= 15.0f;
	}	
	if (c == 'd'){
		camera.angleAlfa += 15.0f;
	}
	if (c == '+'){
		camera.radius -= 1.0f;
	}
	if (c == '-'){
		camera.radius += 1.0f;
	}
	glutPostRedisplay();
}


// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char **argv) {
	// Check arguments
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " <config.xml>" << endl;
		return 1;
	}

	// Load configuration from XML
    string configPath = "../../configs/";
	loadConfigs((configPath + argv[1]).c_str());

	// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("SolariUM");

	// Required callback registry
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

    // Callback registration for keyboard processing
	glutKeyboardFunc(processKeys);

	// OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode
	
	// enter GLUT's main cycle
	glutMainLoop();
	
	return 0;
}
