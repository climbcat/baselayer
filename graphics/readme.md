wget https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
wget https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

g++ main_geom.cpp -lSDL2 -lGL -lGLEW
g++ main_geom.cpp -I C:/Users/garde/lib/glew-2.1.0/include/ -LC:/Users/garde/lib/glew-2.1.0/lib/Release/x64/ -lglew32 -I C:/Users/garde/lib/SDL2-2.28.4/include/ -LC:/Users/garde/lib/SDL2-2.28.4/lib/x64 -lSDL2 -L"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0/um/x64" -lOpenGL32

231017: Everything compiles and the exe starts, but glCompileShader faults, so I guess I have the wrong lib files after all, or windows driver.
231017: It is actually SDL_GL_CreateContext() that fails - returns a null pointer. So the OGL context could not be created for some reason. I seem to be using the correct opengl32.lib and .dll versions.
