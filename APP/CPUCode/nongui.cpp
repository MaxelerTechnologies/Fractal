#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "maxapp.h"
#include "cmdline.h"

void display(FractalEngine *mb, int Xs, int Ys) {
    int x, y, i;
    i = 0;
    for (y = 0; y < Ys; y++) {
        printf("[ ");
        for (x = 0; x < Xs; x++) {
            if (mb->outputArray[i] > 8) printf(".");
            else if (mb->outputArray[i] != 0) printf("%d", mb->outputArray[i]);
            else printf(" ");
            /*if (mb->outputArray[i] > 8) printf(".");
            else printf(" ");*/
            i++;
        }
        printf(" ]\n");
    }
    printf("\n\n");
}


int main(int argc, char *argv[]) {
    const int Xs = 128;
    const int Ys = 60;
    FractalEngine *fe;

    struct gengetopt_args_info args_info;

    if(cmdline_parser(argc, argv, &args_info) != 0) {
        exit(EXIT_FAILURE);
    }

    fe = new FractalEngine();
    fe->init(Xs, Ys);
    fe->computeImage(-3.2, -2.4, 3.2, 2.4); 
    double t;
    fe->waitComputeFinished(t);
    display(fe, Xs, Ys);

    delete fe;

    exit(EXIT_SUCCESS);
}

