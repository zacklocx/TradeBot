
#ifndef RENDERER_INCLUDED
#define RENDERER_INCLUDED

struct mouse_state_t
{
	mouse_state_t() : x_(0), y_(0), button_(-1) {}

	int x_, y_, button_;
};

class renderer_t
{
public:
	static int screen_width();
	static int screen_height();

	static int window_width();
	static int window_height();

	static mouse_state_t mouse_state();

	static void start(int width = 0, int height = 0, int color = 0);
	static void stop();
};

#endif /* RENDERER_INCLUDED */
