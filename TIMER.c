#include <unistd.h>
#include <stdio.h>

int main() {
    char running = 1;
    while (running) {
        sleep(60*20);
        printf("YOU HAD BETTER STILL BE WORKING YOU LITTLE PROCRASTINATOR\n");
    }
    return 0;
}