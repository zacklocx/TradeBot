
#include "renderer.h"

#include <GL/freeglut.h>

#include "imgui.h"
#include "imgui_glut.h"

#include "signals.h"

////////////////////////////////////////////////////////////////////////////////

static void stop()
{
	imgui_glut_shutdown();
	glutLeaveMainLoop();
}

static void reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);

	glMatrixMode(GL_MODELVIEW);
}

static void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	imgui_glut_prepare(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

	ImGui::ShowTestWindow();

	sig_render();
	ImGui::Render();

	glutSwapBuffers();
}

static void idle()
{
	display();
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
	io.MouseWheel = dir;
}

static void mouse_click(int button, int state, int x, int y)
{
	if(3 == button || 4 == button)
	{
		if(GLUT_DOWN == state)
		{
			mouse_wheel((3 == button)? 1 : -1, x, y);
		}
	}
	else
	{
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
	}
}

static void mouse_move(int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(x, y);
}

static void mouse_drag(int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(x, y);
}

////////////////////////////////////////////////////////////////////////////////

int renderer_t::screen_width()
{
	return glutGet(GLUT_SCREEN_WIDTH);
}

int renderer_t::screen_height()
{
	return glutGet(GLUT_SCREEN_HEIGHT);
}

int renderer_t::window_width()
{
	return glutGet(GLUT_WINDOW_WIDTH);
}

int renderer_t::window_height()
{
	return glutGet(GLUT_WINDOW_HEIGHT);
}

void renderer_t::start(int width /* = 0 */, int height /* = 0 */, int color /* = 0 */)
{
	int argc = 1;
	char _[] = "";
	char* argv[] = { _, 0 };

	glutInit(&argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutInitWindowSize(width, height);
	glutCreateWindow("");

	if(!(width > 0 && height > 0))
	{
		glutFullScreen();
	}

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

	double red = (color / 65536 % 256) / 255.0;
	double green = (color / 256 % 256) / 255.0;
	double blue = (color % 256) / 255.0;

	glClearColor(red, green, blue, 1.0);

	imgui_glut_init();

	glutMainLoop();
}

void renderer_t::stop()
{
	::stop();
}
