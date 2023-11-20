#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <signal.h>


#include "../../baselayer.h"
#include "../geometry.h"
#include "../shaders.h"
#include "../ui.h"
#include "../swrender.h"
#include "../gameloop.h"


void RunProgram() {
    TimeFunction;
    printf("Executing glew & glfw link test ...\n");

    signal(SIGINT, CtrlCHandler);

    glewExperimental = GL_TRUE;
    auto glew_init = glewInit();
    auto glfw_init = glfwInit();
    printf("glew: %d glfw: %d\n", glew_init, glfw_init);

    GLFWwindow* window = glfwCreateWindow(640, 480, "glew & glfw link test window", NULL, NULL);

    while (true) {
        XSleep(100);
    }

}

void Test() {
    printf("Running tests ...\n");
}

int main (int argc, char **argv) {
    TimeProgram;

    if (CLAContainsArg("--help", argc, argv) || CLAContainsArg("-h", argc, argv)) {
        printf("--help:          display help (this text)\n");
        exit(0);
    }
    if (CLAContainsArg("--test", argc, argv)) {
        Test();
        exit(0);
    }

    RunProgram();
}
