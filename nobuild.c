#define NOBUILD_IMPLEMENTATION
#include "./nobuild.h"

#define CFLAGS "-Wall", "-Wextra", "-std=c99", "-pedantic"

void build_app(void)
{
    CMD("cc", CFLAGS, "-o", "font_previewer" , "src/main.c");
}




int main(int argc, char **argv)
{
    GO_REBUILD_URSELF(argc, argv);

    build_app();

    return 0;
}
