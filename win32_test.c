#include "./src/noe.h"

int main(void){
    InitApplication();
    while(!WindowShouldClose()) {
        PollInputEvents();
    }
    DeinitApplication();
}
