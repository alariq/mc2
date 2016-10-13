#ifndef GRAPHICS_H
#define GRAPHICS_H


#include <GL/glew.h>
#include <GL/gl.h>

#ifndef PLATFORM_WINDOWS
	// because APIENTRY is undef'ed at the end of glew.h and we use it in shader_builder.h
	#ifndef APIENTRY
		#define APIENTRY
	#endif
#else
	#include<windows.h>
#endif



#endif //GRAPHICS_H
