
#include "renderer.h"

#include <GL/freeglut.h>

#include "imgui.h"
#include "imgui_glut.h"

#include "signals.h"

bool show_test_window = true;
bool show_another_window = false;

////////////////////////////////////////////////////////////////////////////////

static int cached_width = 0;
static int cached_height = 0;
static int cached_button = -1;

static void refresh()
{
	glutPostRedisplay();
}

static void stop()
{
	cached_width = cached_height = 0;
	cached_button = -1;

	imgui_glut_shutdown();

	glutLeaveMainLoop();
}

static void reshape(int width, int height)
{
	cached_width = width;
	cached_height = height;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, cached_width, cached_height);
	gluOrtho2D(0, cached_width, 0, cached_height);

	glMatrixMode(GL_MODELVIEW);
}

static void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	imgui_glut_newframe(cached_width, cached_height);

	sig_render();

	{
        static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        if (ImGui::Button("Test Window")) show_test_window ^= 1;
        if (ImGui::Button("Another Window")) show_another_window ^= 1;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }

    // 2. Show another simple window, this time using an explicit Begin/End pair
    if (show_another_window)
    {
        ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Another Window", &show_another_window);
        ImGui::Text("Hello");
        ImGui::End();
    }

    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
    if (show_test_window)
    {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
        ImGui::ShowTestWindow(&show_test_window);
    }

	ImGui::Render();

	glutSwapBuffers();
	refresh();
}

static void idle()
{
}

static void normal_key_down(unsigned char key, int x, int y)
{
	if(0x1B /* ESCAPE */ == key)
	{
		stop();
	}
	else
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter(key);

		refresh();
	}
}

static void normal_key_up(unsigned char key, int x, int y)
{
}

static void special_key_down(int key, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(key);
}

static void special_key_up(int key, int x, int y)
{
}

static void mouse_wheel(int dir, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(x, y);
	io.MouseWheel = -dir;
}

static void mouse_click(int button, int state, int x, int y)
{
	if(3 == button || 4 == button)
	{
		if(state != GLUT_UP)
		{
			mouse_wheel((3 == button)? -1 : 1, x, y);
		}
	}
	else
	{
		if(cached_button < 0)
		{
			if(GLUT_DOWN == state)
			{
				cached_button = button;
			}
		}
		else if(button == cached_button)
		{
			if(GLUT_UP == state)
			{
				cached_button = -1;
			}
		}

		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(x, y);

		if(GLUT_LEFT_BUTTON == button && GLUT_DOWN == state)
		{
			io.MouseDown[0] = true;
		}
		else
		{
			io.MouseDown[0] = false;
		}

		if(GLUT_RIGHT_BUTTON == button && GLUT_DOWN == state)
		{
			io.MouseDown[1] = true;
		}
		else
		{
			io.MouseDown[1] = false;
		}

		refresh();
	}
}

static void mouse_move(int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(x, y);

	refresh();
}

static void mouse_drag(int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(x, y);

	refresh();
}

////////////////////////////////////////////////////////////////////////////////

int renderer_t::width()
{
	return cached_width;
}

int renderer_t::height()
{
	return cached_height;
}

void renderer_t::start(int width, int height, int bg_color)
{
	int argc = 1;
	char _[] = "";
	char* argv[] = { _, 0 };

	glutInit(&argc, argv);

	glutSetOption(GLUT_INIT_WINDOW_WIDTH, 1024);
	glutSetOption(GLUT_INIT_WINDOW_HEIGHT, 768);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);

	if(width > 0 && height > 0)
	{
		glutInitWindowSize(width, height);
	}

	glutCreateWindow("");

	glutIgnoreKeyRepeat(1);

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(normal_key_down);
	glutKeyboardUpFunc(normal_key_up);
	glutSpecialFunc(special_key_down);
	glutSpecialUpFunc(special_key_down);
	glutMouseFunc(mouse_click);
	glutPassiveMotionFunc(mouse_move);
	glutMotionFunc(mouse_drag);

	glEnable(GL_MULTISAMPLE);

	double red = (bg_color / 256 / 256 % 256) / 255.0;
	double green = (bg_color / 256 % 256) / 255.0;
	double blue = (bg_color % 256) / 255.0;

	glClearColor(red, green, blue, 1.0);

	imgui_glut_init();

	glutMainLoop();
}

void renderer_t::refresh()
{
	::refresh();
}

void renderer_t::stop()
{
	::stop();
}
