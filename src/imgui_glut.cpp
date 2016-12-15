
#include "imgui_glut.h"

#include <GL/freeglut.h>

#include "imgui.h"

static int elapsed = 0;
static unsigned int font_tex = 0;

static void imgui_glut_render(ImDrawData* data)
{
	ImGuiIO& io = ImGui::GetIO();

	int width = io.DisplaySize.x * io.DisplayFramebufferScale.x;
	int height = io.DisplaySize.y * io.DisplayFramebufferScale.y;

	if(0 == width || 0 == height)
	{
		return;
	}

	data->ScaleClipRects(io.DisplayFramebufferScale);

	int last_tex;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_tex);

	int last_viewport[4];
	glGetIntegerv(GL_VIEWPORT, last_viewport);

	int last_scissor[4];
	glGetIntegerv(GL_SCISSOR_BOX, last_scissor);

	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_SCISSOR_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, io.DisplaySize.x, io.DisplaySize.y, 0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE*)0)->ELEMENT))
	for(int i = 0; i < data->CmdListsCount; ++i)
	{
		const ImDrawList* cmd_list = data->CmdLists[i];
		const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
		const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;

		glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), ((const char*)vtx_buffer + OFFSETOF(ImDrawVert, pos)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), ((const char*)vtx_buffer + OFFSETOF(ImDrawVert, uv)));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), ((const char*)vtx_buffer + OFFSETOF(ImDrawVert, col)));

		for(int j = 0; j < cmd_list->CmdBuffer.Size; ++j)
		{
			const ImDrawCmd* cmd = &cmd_list->CmdBuffer[j];

			if(cmd->UserCallback)
			{
				cmd->UserCallback(cmd_list, cmd);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, (intptr_t)cmd->TextureId);
				glScissor(cmd->ClipRect.x, height - cmd->ClipRect.w, cmd->ClipRect.z - cmd->ClipRect.x, cmd->ClipRect.w - cmd->ClipRect.y);
				glDrawElements(GL_TRIANGLES, cmd->ElemCount, (sizeof(ImDrawIdx) == 2)? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
			}

			idx_buffer += cmd->ElemCount;
		}
	}
	#undef OFFSETOF

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glPopAttrib();

	glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
	glScissor(last_scissor[0], last_scissor[1], last_scissor[2], last_scissor[3]);

	glBindTexture(GL_TEXTURE_2D, last_tex);
}

static void imgui_glut_create()
{
	ImGuiIO& io = ImGui::GetIO();

	int last_tex;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_tex);

	int width, height;
	unsigned char* pixels;

	io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

	glGenTextures(1, &font_tex);
	glBindTexture(GL_TEXTURE_2D, font_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);

	io.Fonts->TexID = (void*)(intptr_t)font_tex;

	glBindTexture(GL_TEXTURE_2D, last_tex);
}

static void imgui_glut_destroy()
{
	ImGui::GetIO().Fonts->TexID = 0;

	if(font_tex)
	{
		glDeleteTextures(1, &font_tex);
		font_tex = 0;
	}
}

void imgui_glut_init()
{
	ImGuiIO& io = ImGui::GetIO();

	io.RenderDrawListsFn = imgui_glut_render;

	io.KeyMap[ImGuiKey_Tab]			= 9;					// tab
	io.KeyMap[ImGuiKey_LeftArrow]	= GLUT_KEY_LEFT;		// Left
	io.KeyMap[ImGuiKey_RightArrow]	= GLUT_KEY_RIGHT;		// Right
	io.KeyMap[ImGuiKey_UpArrow]		= GLUT_KEY_UP;			// Up
	io.KeyMap[ImGuiKey_DownArrow]	= GLUT_KEY_DOWN;		// Down
	io.KeyMap[ImGuiKey_PageUp]		= GLUT_KEY_PAGE_UP;		// PageUp
	io.KeyMap[ImGuiKey_PageDown]	= GLUT_KEY_PAGE_DOWN;	// PageDown
	io.KeyMap[ImGuiKey_Home]		= GLUT_KEY_HOME;		// Home
	io.KeyMap[ImGuiKey_End]			= GLUT_KEY_END;			// End
	io.KeyMap[ImGuiKey_Delete]		= 127;					// Delete
	io.KeyMap[ImGuiKey_Backspace]	= 8;					// Backspace
	io.KeyMap[ImGuiKey_Enter]		= 13;					// Enter
	io.KeyMap[ImGuiKey_Escape]		= 27;					// Escape
	io.KeyMap[11]					= 1;					// ctrl-A
	io.KeyMap[12]					= 3;					// ctrl-C
	io.KeyMap[13]					= 22;					// ctrl-V
	io.KeyMap[14]					= 24;					// ctrl-X
	io.KeyMap[15]					= 25;					// ctrl-Y
	io.KeyMap[16]					= 26;					// ctrl-Z
}

void imgui_glut_prepare(int width, int height)
{
	if(!font_tex)
	{
		imgui_glut_create();
	}

	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = ImVec2(width, height);

	int glut_elapsed = glutGet(GLUT_ELAPSED_TIME);
	float deltaTime = (glut_elapsed - elapsed) / 1000.0f;

	io.DeltaTime = (deltaTime > 0.0f)? deltaTime : 1.0f / 60.0f;
	elapsed = glut_elapsed;

	ImGui::NewFrame();
}

void imgui_glut_shutdown()
{
	imgui_glut_destroy();
	ImGui::Shutdown();
}
