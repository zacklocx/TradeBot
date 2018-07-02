
#include "imgui_glut.h"

#include <chrono>

#include <GL/freeglut.h>

#include "imgui.h"

namespace
{
	void imgui_glut_draw(ImDrawData* data)
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
		glOrtho(0, io.DisplaySize.x, io.DisplaySize.y, 0, -1, 1);

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
					glDrawElements(GL_TRIANGLES, cmd->ElemCount, (2 == sizeof(ImDrawIdx))? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
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

		glBindTexture(GL_TEXTURE_2D, (unsigned int)last_tex);
	}
}

void imgui_glut_init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	io.RenderDrawListsFn = imgui_glut_draw;

	io.KeyMap[ImGuiKey_Tab]			= 9;
	io.KeyMap[ImGuiKey_LeftArrow]	= GLUT_KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow]	= GLUT_KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow]		= GLUT_KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow]	= GLUT_KEY_DOWN;
	io.KeyMap[ImGuiKey_PageUp]		= GLUT_KEY_PAGE_UP;
	io.KeyMap[ImGuiKey_PageDown]	= GLUT_KEY_PAGE_DOWN;
	io.KeyMap[ImGuiKey_Home]		= GLUT_KEY_HOME;
	io.KeyMap[ImGuiKey_End]			= GLUT_KEY_END;
	io.KeyMap[ImGuiKey_Delete]		= 127;
	io.KeyMap[ImGuiKey_Backspace]	= 8;
	io.KeyMap[ImGuiKey_Enter]		= 13;
	io.KeyMap[ImGuiKey_Escape]		= 27;
	io.KeyMap[ImGuiKey_A]			= 1;
	io.KeyMap[ImGuiKey_C]			= 3;
	io.KeyMap[ImGuiKey_V]			= 22;
	io.KeyMap[ImGuiKey_X]			= 24;
	io.KeyMap[ImGuiKey_Y]			= 25;
	io.KeyMap[ImGuiKey_Z]			= 26;
}

void imgui_glut_prepare(int width, int height)
{
	ImGuiIO& io = ImGui::GetIO();

	if(!ImGui::GetIO().Fonts->TexID)
	{
		int last_tex;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_tex);

		int width, height;
		unsigned char* pixels;

		io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

		unsigned int font_tex;

		glGenTextures(1, &font_tex);
		glBindTexture(GL_TEXTURE_2D, font_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);

		io.Fonts->TexID = (void*)(intptr_t)font_tex;

		glBindTexture(GL_TEXTURE_2D, (unsigned int)last_tex);
	}

	io.DisplaySize = ImVec2(width, height);

	static auto base_time = std::chrono::system_clock::now();
	auto now = std::chrono::system_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - base_time);

	io.DeltaTime = duration.count() / 1000.0f;

	base_time = now;

	ImGui::NewFrame();
}

void imgui_glut_shutdown()
{
	unsigned int font_tex = (intptr_t)ImGui::GetIO().Fonts->TexID;

	if(font_tex)
	{
		glDeleteTextures(1, &font_tex);
		ImGui::GetIO().Fonts->TexID = 0;
	}
}
