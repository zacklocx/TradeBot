
#ifndef RENDERER_INCLUDED
#define RENDERER_INCLUDED

class renderer_t
{
public:
	static int width();
	static int height();

	static void start(int width = 0, int height = 0);
	static void refresh();
	static void stop();
};

#endif /* RENDERER_INCLUDED */
