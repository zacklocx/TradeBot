
#ifndef RENDERER_INCLUDED
#define RENDERER_INCLUDED

class renderer_t
{
public:
	static int screen_width();
	static int screen_height();

	static int window_width();
	static int window_height();

	static int mouse_x();
	static int mouse_y();
	static int mouse_button();

	static void start(int width, int height, int bg_color);
	static void stop();
};

#endif /* RENDERER_INCLUDED */
