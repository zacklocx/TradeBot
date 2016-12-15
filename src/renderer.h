
#ifndef RENDERER_INCLUDED
#define RENDERER_INCLUDED

class renderer_t
{
public:
	static int screen_width();
	static int screen_height();

	static int window_width();
	static int window_height();

	static void start(int width = 0, int height = 0, int color = 0);
	static void stop();
};

#endif /* RENDERER_INCLUDED */
