// Dear ImGui: standalone example application for GLUT/FreeGLUT + OpenGL2, using legacy fixed pipeline
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// !!! GLUT/FreeGLUT IS OBSOLETE PREHISTORIC SOFTWARE. Using GLUT is not recommended unless you really miss the 90's. !!!
// !!! If someone or something is teaching you GLUT today, you are being abused. Please show some resistance. !!!
// !!! Nowadays, prefer using GLFW or SDL instead!

// On Windows, you can install Freeglut using vcpkg:
//   git clone https://github.com/Microsoft/vcpkg
//   cd vcpkg
//   bootstrap - vcpkg.bat
//   vcpkg install freeglut --triplet=x86-windows   ; for win32
//   vcpkg install freeglut --triplet=x64-windows   ; for win64
//   vcpkg integrate install                        ; register include and libs in Visual Studio

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include <armadillo>

using namespace arma;

#ifdef _MSC_VER
#pragma warning(disable : 4505) // unreferenced local function has been removed
#endif

// Our state
static bool show_demo_window = false;
static bool show_another_window = false;
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// void reshape(int,int);
float t = .5;

// variables globales pour OpenGL
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance = 0.;

// constantes pour les materieux
float no_mat[] = {0.0f, 0.0f, 0.0f, 1.0f};
float mat_ambient[] = {0.7f, 0.7f, 0.7f, 1.0f};
float mat_ambient_color[] = {0.8f, 0.8f, 0.2f, 1.0f};
float mat_diffuse[] = {0.1f, 0.5f, 0.8f, 1.0f};
float mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
float no_shininess = 0.0f;
float low_shininess = 5.0f;
float high_shininess = 100.0f;
float mat_emission[] = {0.3f, 0.2f, 0.2f, 0.0f};

// Control Points
mat p = {
    {0, 1, 0},
    {0, 1, 1},
    {1, 0, 1},
    {0, 0, 0}};

mat m = {
    {-1, 3, -3, 1},
    {3, -6, 3, 0},
    {-3, 3, 0, 0},
    {1, 0, 0, 0}};

mat bezier(float t)
{
    rowvec tVec = {pow(t, 3), pow(t, 2), t, 1};
    mat bVec = tVec * m;
    mat tmp = bVec * p;
    return tmp;
}
mat bezierDerivate(float t)
{
    rowvec tVec = {pow(t, 2) * 3, t * 2, 1, 0};
    mat bVec = tVec * m;
    mat tmp = bVec * p;
    return tmp;
}
mat bezier2nDerivate(float t)
{
    rowvec tVec = {t * 6, 2, 0, 0};
    mat bVec = tVec * m;
    mat tmp = bVec * p;
    return tmp;
}
void draw_circle(mat c, mat n, mat b)
{
    float r = 1;
    int num_segments = 100;

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < num_segments; i++)
    {
        float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);
        rowvec p_t = c + b * (r * cosf(theta)) + n * (r * sinf(theta));
        glVertex3f(p_t[0], p_t[1], p_t[2]);
    }
    glEnd();
}
void draw_cylindre(mat c, mat n, mat b)
{
    float r = 0.2;
    int num_segments = 100;

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < num_segments; i++)
    {
        float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);
        rowvec p_t = c + b * (r * cosf(theta)) + n * (r * sinf(theta));
        glVertex3f(p_t[0], p_t[1], p_t[2]);
    }
    glEnd();
}
float frenet = 0.5;
void initOpenGl()
{

    // lumiere

    glClearColor(.5, .5, 0.5, 0.0);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat l_pos[] = {3., 3.5, 3.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, l_pos);

    glLightfv(GL_LIGHT0, GL_DIFFUSE, l_pos);
    glLightfv(GL_LIGHT0, GL_SPECULAR, l_pos);
    glEnable(GL_COLOR_MATERIAL);

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    // glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    //  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE|GLUT_RGB);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)200 / (GLfloat)200, 0.1f, 10.0f);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(0., 0., 4., 0., 0., 0., 0., 1., 0.);
}

//------------------------------------------------------

void displayCourbe(void)
{
    glPointSize(10);
    glColor3f(1.0, 1.0, 1.0);
    rowvec prevPoint = p.row(0);

    for (float t = 0.01; t <= 1; t += 0.01)
    {
        rowvec currPoint = bezier(t);
        glBegin(GL_LINES);
        glVertex3f(prevPoint(0), prevPoint(1), prevPoint(2));
        glVertex3f(currPoint(0), currPoint(1), currPoint(2));
        glEnd();
        prevPoint = currPoint;
    }

    rowvec f = bezier(frenet);

    for (int i = 0; i < 4; i++)
    {
        glBegin(GL_POINTS);
        glColor3f(1.0, 1.0, 1.0);
        glVertex3f(p.row(i)(0), p.row(i)(1), p.row(i)(2));
        glEnd();
    }

    mat fPrime = bezierDerivate(frenet);
    mat f2Prime = bezier2nDerivate(frenet);
    mat t = normalise(fPrime, 2, 1);
    mat f2PrimeT = fPrime / pow(norm(fPrime), 2) * dot(fPrime, f2Prime);
    mat f2PrimeN = f2Prime - f2PrimeT;
    mat n = normalise(f2PrimeN, 2, 1);
    mat b = cross(t, n);

    glLineWidth(3.0);

    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(f(0), f(1), f(2));
    glVertex3f(f(0) + t(0), f(1) + t(1), f(2) + t(2));
    glEnd();

    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(f(0), f(1), f(2));
    glVertex3f(f(0) + b(0), f(1) + b(1), f(2) + b(2));
    glEnd();

    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
    glVertex3f(f(0), f(1), f(2));
    glVertex3f(f(0) + n(0), f(1) + n(1), f(2) + n(2));
    glEnd();

    glColor3f(0.0, 0.5, 0.5);
    rowvec c = f + n;
    draw_circle(c, n, t);
    glLineWidth(1.0);

    glColor3f(0.58, 0.0, 0.53);
    for (float t = 0.0; t <= 1; t += 0.01)
    {
        rowvec currPoint = bezier(t);
        mat fPrime_bis = bezierDerivate(t);
        mat f2Prime_bis = bezier2nDerivate(t);
        mat t_bis = normalise(fPrime_bis, 2, 1);
        mat f2PrimeT_bis = fPrime_bis / pow(norm(fPrime_bis), 2) * dot(fPrime_bis, f2Prime_bis);
        mat f2PrimeN_bis = f2Prime_bis - f2PrimeT_bis;
        mat n_bis = normalise(f2PrimeN_bis, 2, 1);
        mat b_bis = cross(t_bis, n_bis);
        draw_cylindre(currPoint, n_bis, b_bis);
        prevPoint = currPoint;
    }
}

//------------------------------------------------------
void affiche_repere(void)
{
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex2f(0., 0.);
    glVertex2f(1., 0.);
    glEnd();

    glBegin(GL_LINES);
    glColor3f(0.0, 1.0, 0.0);
    glVertex2f(0., 0.);
    glVertex2f(0., 1.);
    glEnd();
    glBegin(GL_LINES);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0., 0., 0.);
    glVertex3f(0., 0., 1.);
    glEnd();
}

//-----------------------------------------------------

//------------------------------------------------------
void affichage(void)
{
    
}

//------------------------------------------------------

//------------------------------------------------------
void clavier(unsigned char touche, int x, int y)
{

    switch (touche)
    {
    case '+': //
        frenet += 0.01;
        frenet = frenet > 1 ? 1 : frenet;
        cout << "Frenet =" << frenet << endl;
        glutPostRedisplay();
        break;
    case '-': //* ajustement du t
        frenet -= 0.01;
        frenet = frenet < 0 ? 0 : frenet;
        cout << "Frenet =" << frenet << endl;
        glutPostRedisplay();
        break;
    case 'f': //* affichage en mode fil de fer
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glutPostRedisplay();
        break;
    case 'p': //* affichage du carre plein
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glutPostRedisplay();
        break;
    case 's': //* Affichage en mode sommets seuls
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glutPostRedisplay();
        break;
    case 'l': //*la touche 'q' permet de quitter le programme
        exit(0);
    }
}
void mouse(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if (state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if (state == GLUT_UP)
            mouseRightDown = false;
    }

    else if (button == GLUT_MIDDLE_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseMiddleDown = true;
        }
        else if (state == GLUT_UP)
            mouseMiddleDown = false;
    }
}

void mouseMotion(int x, int y)
{
    if (mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if (mouseRightDown)
    {
        cameraDistance += (y - mouseY) * 0.2f;
        mouseY = y;
    }

    glutPostRedisplay();
}

void my_display_code()
{
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Config Window"); // Create a window called "Hello, world!" and append into it.

        ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
        //ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &frenet, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
        //ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

        // if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
        //     counter++;
        // ImGui::SameLine();
        // ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
}

void glut_display_func()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();

    my_display_code();

    // Rendering
    ImGui::Render();
    ImGuiIO &io = ImGui::GetIO();
    glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    // glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound, but prefer using the GL3+ code.

        glutSwapBuffers();


    glMatrixMode(GL_MODELVIEW);
    /* effacement de l'image avec la couleur de fond */
    //	glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //       glClearDepth(10.0f);                         // 0 is near, >0 is far

    glPushMatrix();
    glTranslatef(0, 0, cameraDistance);
    glRotatef(cameraAngleX, 1., 0., 0.);
    glRotatef(cameraAngleY, 0., 1., 0.);
    affiche_repere();
    displayCourbe();
    glPopMatrix();
    /* on force l'affichage du resultat */
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glFlush();
    glutSwapBuffers();
    glutPostRedisplay();
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.

int main(int argc, char **argv)
{
    // Create GLUT window
    glutInit(&argc, argv);
#ifdef __FREEGLUT_EXT_H__
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Dear ImGui GLUT+OpenGL2 Example");

    // Setup GLUT display function
    // We will also call ImGui_ImplGLUT_InstallFuncs() to get all the other functions installed for us,
    // otherwise it is possible to install our own functions and call the imgui_impl_glut.h functions ourselves.
    glutDisplayFunc(glut_display_func);
    glutKeyboardFunc(clavier);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    //-------------------------------

    //-------------------------------
    initOpenGl();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    // FIXME: Consider reworking this example to install our own GLUT funcs + forward calls ImGui_ImplGLUT_XXX ones, instead of using ImGui_ImplGLUT_InstallFuncs().
    ImGui_ImplGLUT_Init();
    ImGui_ImplGLUT_InstallFuncs();
    ImGui_ImplOpenGL3_Init();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != NULL);

    glutMainLoop();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
