#include "core/application.h"

int main(int argc, char* argv[]) {
    Application app;

    app.init();

    app.mainLoop();

    app.cleanup();

    return 0;
}