
#ifndef RENDERER_INCLUDED
#define RENDERER_INCLUDED

class renderer_t
{
public:
	static int width();
	static int height();

	static void start(int width, int height, int bg_color);
	static void refresh();
	static void stop();
};

#endif /* RENDERER_INCLUDED */
