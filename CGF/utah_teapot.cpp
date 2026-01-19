#include <GL/glut.h>
#include <iostream>
#include <cmath>

// --- GLOBAL VARIABLES ---

// Rotation angles for user interaction in Window 1 
float angleX = 0.0f;
float angleY = 0.0f;
int lastX = 0, lastY = 0;
bool isDragging = false;

// View management for Window 2 
int viewMode = 1;
float red = 1.0f, green = 1.0f, blue = 1.0f; // Variables for different view colors 
bool showTexture = true;

// Window identifiers to manage two separate contexts 
int window1, window2;

// --- HD TEXTURE SETTINGS ---
// High resolution settings for quality porcelain rendering 
#define TEX_WIDTH 512  
#define TEX_HEIGHT 512
GLubyte textureImage[TEX_HEIGHT][TEX_WIDTH][3];
GLuint texID;

// --- TEXTURE GENERATION: CIRCULAR CHINESE MOTIF ---
// Procedurally generates a floral mandala pattern for the porcelain surface 
void makeHDFlorealTexture() {
    float cx = TEX_WIDTH / 2.0f;
    float cy = TEX_HEIGHT / 2.0f;

    for (int i = 0; i < TEX_HEIGHT; i++) {
        for (int j = 0; j < TEX_WIDTH; j++) {
            // Coordinate calculation relative to the center
            float dy = i - cy;
            float dx = j - cx;
            float dist = sqrt(dx * dx + dy * dy);
            float angle = atan2(dy, dx);

            // Mathematical formula to create 12 petals and ring patterns
            float pattern = sin(angle * 12.0f + dist * 0.05f) + cos(dist * 0.1f);
            pattern = pattern * pattern;

            // Mapping pattern values to Cobalt Blue and Porcelain White 
            if (pattern > 0.8f) {
                textureImage[i][j][0] = 0;   textureImage[i][j][1] = 40;  textureImage[i][j][2] = 120;
            }
            else if (pattern > 0.2f) {
                textureImage[i][j][0] = 100; textureImage[i][j][1] = 150; textureImage[i][j][2] = 255;
            }
            else {
                textureImage[i][j][0] = 245; textureImage[i][j][1] = 245; textureImage[i][j][2] = 250;
            }
        }
    }
}

// --- PORCELAIN MATERIAL SETTINGS ---
// Defines surface properties for a realistic glassy reflection (Bonus Feature) 
void setPorcelainMaterial() {
    GLfloat mat_ambient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_shininess[] = { 120.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_DIFFUSE);
}

// --- INITIALIZATION ---
void init() {
    glClearColor(0.15, 0.15, 0.15, 1.0);
    glEnable(GL_DEPTH_TEST); // Enable depth buffering for realistic display 

    // Setup Lighting with two sources for 3D depth 
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat lightPos0[] = { 10.0f, 10.0f, 10.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

    glEnable(GL_LIGHT1); // Secondary fill light
    GLfloat lightPos1[] = { -10.0f, -5.0f, 5.0f, 1.0f };
    GLfloat dimLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, dimLight);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);

    setPorcelainMaterial();
    makeHDFlorealTexture();

    // Texture binding and linear filtering for smooth visual results 
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TEX_WIDTH, TEX_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, textureImage);

    // Object Linear Texture Generation to map pattern correctly on teapot geometry
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    float scale = 0.4f;
    GLfloat s_params[] = { scale, 0.0f, 0.0f, 0.0f };
    GLfloat t_params[] = { 0.0f, scale, 0.0f, 0.0f };
    glTexGenfv(GL_S, GL_OBJECT_PLANE, s_params);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, t_params);
}

// --- RENDERING TEAPOT ---
void drawTexturedTeapot() {
    // Switch between textured porcelain and solid colors based on menu selection 
    if (showTexture) {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glBindTexture(GL_TEXTURE_2D, texID);
        glColor3f(1.0, 1.0, 1.0);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glColor3f(red, green, blue);
    }

    glutSolidTeapot(1.2); // Using the predefined Utah teapot as required

    glDisable(GL_TEXTURE_2D);
}

// --- WINDOW 1: INTERACTIVE VIEW ---
void display1() {
    glutSetWindow(window1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    // Applying geometric transformations based on mouse dragging
    glRotatef(angleY, 0.0, 1.0, 0.0);
    drawTexturedTeapot();
    glutSwapBuffers();
}

// Handling mouse interactions for Window 1 rotation 
void mouse1(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        isDragging = (state == GLUT_DOWN);
        lastX = x; lastY = y;
    }
}

void motion1(int x, int y) {
    if (isDragging) {
        angleY += (x - lastX) * 0.5f;
        angleX += (y - lastY) * 0.5f;
        lastX = x; lastY = y;
        glutPostRedisplay();
    }
}

void reshape1(int w, int h) {
    glutSetWindow(window1);
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

// --- WINDOW 2: STATIC VIEWS CONTROL PANEL ---
void display2() {
    glutSetWindow(window2);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Matrix management for different orthographic-like views 
    switch (viewMode) {
    case 1: gluLookAt(0.0, 6.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0); break; // Top View
    case 2: gluLookAt(0.0, -6.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0); break; // Bottom View
    case 3: gluLookAt(0.0, 0.0, -6.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); break; // Back View
    case 4: gluLookAt(0.0, 0.0, 6.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); break; // Front View
    case 5: gluLookAt(4.0, 4.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); break; // Perspective View
    }

    drawTexturedTeapot();
    glutSwapBuffers();
}

// --- MENU CALLBACKS ---
void menuView(int id) {
    viewMode = id;
    glutSetWindow(window2); glutPostRedisplay();
}

void menuColor(int id) {
    showTexture = false;
    // Defining different colors for different views to separate them 
    switch (id) {
    case 1: red = 1.0f; green = 1.0f; blue = 1.0f; break;
    case 2: red = 0.8f; green = 0.0f; blue = 0.0f; break;
    case 3: red = 0.0f; green = 0.6f; blue = 0.0f; break;
    case 4: red = 0.0f; green = 0.4f; blue = 0.8f; break;
    case 5: red = 0.83f; green = 0.68f; blue = 0.21f; break;
    case 6: red = 0.75f; green = 0.75f; blue = 0.75f; break;
    }
    glutSetWindow(window2); glutPostRedisplay();
    glutSetWindow(window1); glutPostRedisplay();
}

void menuTexture(int id) {
    showTexture = true;
    glutSetWindow(window2); glutPostRedisplay();
    glutSetWindow(window1); glutPostRedisplay();
}

void menuMain(int id) {}

void reshape2(int w, int h) {
    glutSetWindow(window2);
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

// --- MAIN FUNCTION ---
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Initializing First Window with mouse interaction 
    glutInitWindowSize(400, 400);
    glutInitWindowPosition(50, 100);
    window1 = glutCreateWindow("HD Porcelain View");
    init();
    glutDisplayFunc(display1);
    glutReshapeFunc(reshape1);
    glutMouseFunc(mouse1);
    glutMotionFunc(motion1);

    // Initializing Second Window with no left mouse interaction 
    glutInitWindowSize(400, 400);
    glutInitWindowPosition(460, 100);
    window2 = glutCreateWindow("Control Panel");
    init();
    glutDisplayFunc(display2);
    glutReshapeFunc(reshape2);

    // Creating Submenus for the right-click menu system 
    int subMenuViews = glutCreateMenu(menuView);
    glutAddMenuEntry("Top", 1);
    glutAddMenuEntry("Bottom", 2);
    glutAddMenuEntry("Left", 3);
    glutAddMenuEntry("Right", 4);
    glutAddMenuEntry("Perspective", 5);

    int subMenuColors = glutCreateMenu(menuColor);
    glutAddMenuEntry("White", 1);
    glutAddMenuEntry("Ruby", 2);
    glutAddMenuEntry("Emerald", 3);
    glutAddMenuEntry("Ocean", 4);
    glutAddMenuEntry("Gold", 5);
    glutAddMenuEntry("Silver", 6);

    int subMenuStyle = glutCreateMenu(menuTexture);
    glutAddMenuEntry("Chinese Porcelain (HD Floral)", 1);

    // Attaching the main menu to the right mouse button in Window 2 
    glutCreateMenu(menuMain);
    glutAddSubMenu("Camera Angles", subMenuViews);
    glutAddSubMenu("Special Texture", subMenuStyle);
    glutAddSubMenu("Solid Colors", subMenuColors);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    std::cout << "Generating HD Texture..." << std::endl;
    std::cout << "Ready!" << std::endl;

    glutMainLoop();
    return 0;
}