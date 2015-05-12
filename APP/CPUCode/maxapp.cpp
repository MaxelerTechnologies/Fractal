/*********************************************************************
 * Fractal Calculation Engine
 *
 * The FractalEngine class provides an application-level interface
 * to Fractal computation, supporting both standard software and
 * accelerated implementations.
 *********************************************************************/

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <complex>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include "maxapp.h"

using namespace std;

extern "C" double timediff(struct timeval begin, struct timeval end) {
    return (((double)end.tv_sec) * 1000000. + ((double)end.tv_usec)) - 
           (((double)begin.tv_sec) * 1000000. + ((double)begin.tv_usec));
}

FractalEngine::FractalEngine()
: run(NULL)
{
}

FractalEngine::~FractalEngine() {
    if (run != NULL) {
        max_wait(run);
        run = NULL;
    }
}

int FractalEngine::init(int xSize, int ySize, int iters) {
    height = ySize;
    width = xSize;
    iterations = iters;
    outputSize = xSize * ySize;
    currentBuffer = 0;

    if (height > 1024 || width > 1024) {
        fprintf(stderr,"The MB engine has buffers to compute up to 1024x1024");
        return -1;
    }

    outputBuffer[0] = (uint16_t*) malloc(sizeof(uint16_t) * 1024 * 1024);
    if (outputBuffer[0] == NULL) {
        fprintf(stderr, "error: failed to allocate memory\n");
        return -1;
    }

    outputBuffer[1] = (uint16_t*) malloc(sizeof(uint16_t) * 1024 * 1024);
    if (outputBuffer[1] == NULL) {
        fprintf(stderr, "error: failed to allocate memory\n");
        return -1;
    }
    outputArray = outputBuffer[1];

    iterations = (int) round((float)iters / Fractal_UNROLL_FACTOR) * Fractal_UNROLL_FACTOR;

    printf("Number of iterations: %d\n", iterations);

    if (width*height % Fractal_UNROLL_FACTOR != 0) {
        fprintf(stderr, "error: the unroll factor is incompatible with the computing area dimensions\n");
        return -1;
    }

    isMandelbrot = true;
    c_real = 0;
    c_imag = 0;

    ledStatus = 0;

    return 0;
}

short FractalEngine::iterate(double X, double Y) {
    short I = 0;
    complex<double> Z;
    complex<double> C;

    // Set initial values for iteration
    if (isMandelbrot) {
        C = complex<double>(X, Y);
        Z = complex<double>(0, 0);
    } else {
        C = complex<double>(c_real, c_imag);
        Z = complex<double>(X, Y);
    }
    
    do { // iterate
        Z = Z * Z + C;
        ++I;
    } while ((I < iterations) && (abs(Z) < 5.0));
    return I;
}

// Command from GUI
int FractalEngine::computeImage(double x1, double y1, double x2, double y2) {
    int ret;
    usingDFElast = usingDFE;
    if (usingDFE)
        ret = computeDFE(x1, y1, x2, y2);
    else
        ret = computeSW(x1, y1, x2, y2);
    return ret;
}

// Command from GUI
int FractalEngine::waitComputeFinished(double &t) {
    if (usingDFElast) t = computeDFESync();
    else t = lastSwTime;
    return 0;
}

// End of frame completion/cleanup
double FractalEngine::computeDFESync(void) {
    struct timeval endTime;
    double computeTime;
    max_wait(run);
    run = NULL;
    
    gettimeofday(&endTime, NULL);
    computeTime = timediff(startTime, endTime);
    
    // swap buffers
    outputArray = outputBuffer[currentBuffer];
    currentBuffer = 1-currentBuffer;

    return computeTime/1E6;
}

int FractalEngine::computeDFE(double x1, double y1, double x2, double y2) {
    double xStep, yStep;

    xStep = (x2 - x1) / width;
    yStep = (y2 - y1) / height;
    inputBuffer[currentBuffer][0] = x1;
    inputBuffer[currentBuffer][1] = y1;
    inputBuffer[currentBuffer][2] = xStep;
    inputBuffer[currentBuffer][3] = yStep;

    /** DFE Fractal accelerator parameters
     *
     *  x       : X co-ordinate of start point
     *  y       : Y co-ordinate of start point
     *  xstep   : X distance between points
     *  ystep   : Y distance between points
     *
     */
    gettimeofday(&startTime, NULL); // Time run

    run = Fractal_nonblock(c_imag, c_real, uint8_t(log(width)/log(2.0)),
            iterations, width*height, isMandelbrot, width,
            inputBuffer[currentBuffer], outputBuffer[currentBuffer]);

    return 0;
}

int FractalEngine::computeSW(double x1, double y1, double x2, double y2) {
    double xStep, yStep;
    double X, Y;
    double computeTime;
    int i;
    int xPos, yPos;
    struct timeval startTime, endTime;

    xStep = (x2 - x1) / width;
    yStep = (y2 - y1) / height;
    i = 0;

    gettimeofday(&startTime, NULL);
    X = x1; Y = y1;
    for (yPos = 0; yPos < height ; yPos++) {
        for (xPos = 0; xPos < width ; xPos++) {
            outputArray[i] = iterate(X, Y);
            i++;
            X+=xStep;
        }
        Y+=yStep;
        X=x1;
    }
    gettimeofday(&endTime, NULL);

    computeTime = timediff(startTime, endTime);
    computeTime = computeTime / 1E6; // Convert to seconds
     
    lastSwTime = computeTime;
    
    return 0;
}

void FractalEngine::useDFE(bool yesOrNo) {
    usingDFE = yesOrNo;
}

int FractalEngine::getIterations() {
    return iterations;
}

void FractalEngine::setMandelbrot(void) {
    isMandelbrot = true;
}

void FractalEngine::setJulia(double c_real, double c_imag) {
    isMandelbrot = false;
    this->c_real = c_real;
    this->c_imag = c_imag;
}

#if 0
/** Alternative Iteration function to emulate DFE functionality */
short FractalEngine::iterate(double X, double Y) {
    short I=0,i=0,STOP=0;
    double ZXold, ZX=0.0,ZY=0.0;
    // iterate
    for(i=0;i<iterations;i++){
       ZXold=ZX;
       ZX = STOP ? ZX : (ZX*ZX-ZY*ZY + X);
       ZY = STOP ? ZY : (ZXold*ZY+ZY*ZXold + Y);
       STOP = STOP ? STOP : ((ZX*ZX+ZY*ZY)>=5.0);
       I = STOP ? I : (I+1);
    }
    return I;
}
#endif

extern "C" FractalEngine* create_FractalEngine() {
    return new FractalEngine();
}

extern "C" void destroy_FractalEngine(FractalEngine *fe) {
    delete fe;
}
