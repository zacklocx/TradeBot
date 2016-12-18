
#include "renderer.h"

#include <cctype>

#include <GL/freeglut.h>

#include "imgui.h"
#include "imgui_glut.h"

#include "signals.h"

////////////////////////////////////////////////////////////////////////////////

static mouse_state_t mouse_state;

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

	// ImGui::ShowTestWindow();

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
	if(27 /* Escape */ == key)
	{
		stop();
	}
	else
	{
		ImGuiIO& io = ImGui::GetIO();

		if(isprint(key))
		{
			io.AddInputCharacter(key);
		}
		else
		{
			io.KeysDown[key] = true;
		}
	}
}

static void normal_key_up(unsigned char key, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();

	if(!isprint(key))
	{
		io.KeysDown[key] = false;
	}
}

static void special_key_down(int key, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeysDown[key] = true;
}

static void special_key_up(int key, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeysDown[key] = false;
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
		bool left_button = GLUT_LEFT_BUTTON == button;
		bool right_button = GLUT_RIGHT_BUTTON == button;

		mouse_state.x_ = x;
		mouse_state.y_ = glutGet(GLUT_WINDOW_HEIGHT) - 1 - y;
		mouse_state.button_ = left_button? 0 : right_button? 1 : -1;

		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(x, y);

		if(left_button && GLUT_DOWN == state)
		{
			io.MouseDown[0] = true;
		}
		else
		{
			io.MouseDown[0] = false;
		}

		if(right_button == button && GLUT_DOWN == state)
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
	mouse_state.x_ = x;
	mouse_state.y_ = glutGet(GLUT_WINDOW_HEIGHT) - 1 - y;
	mouse_state.button_ = -1;

	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(x, y);
}

static void mouse_drag(int x, int y)
{
	mouse_state.x_ = x;
	mouse_state.y_ = glutGet(GLUT_WINDOW_HEIGHT) - 1 - y;

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

mouse_state_t renderer_t::mouse_state()
{
	return ::mouse_state;
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

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(normal_key_down);
	glutKeyboardUpFunc(normal_key_up);
	glutSpecialFunc(special_key_down);
	glutSpecialUpFunc(special_key_up);
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
