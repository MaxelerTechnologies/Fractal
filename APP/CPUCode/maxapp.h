/*********************************************************************
 * Maxeler Technologies: Fractal Demo                                *
 *                                                                   *
 * Version: 1.7                                                      *
 * Date:    14 August 2013                                           *
 *                                                                   *
 * CPU code source file                                              *
 *                                                                   *
 *********************************************************************/

/*********************************************************************
 * Fractal Calculation Engine
 *
 * The FractalEngine class provides an application-level interface
 * to Fractal computation, supporting both standard software and
 * accelerated implementations.
 *********************************************************************/
 
#ifndef _MAX_APP_H
#define _MAX_APP_H

#include <sys/time.h>
#include "Fractal.h"

class FractalEngine {
    protected:
        max_run_t* run;
        bool ledStatus;
        struct timeval startTime;
        bool usingDFE, usingDFElast;
        char *redMap, *greenMap, *blueMap;
        int iterations;
        int outputSize;
        int height, width; 
        double lastSwTime;
        bool isMandelbrot;
        double c_real, c_imag;

        short iterate(double X, double Y); 
        void colourise();
        int computeDFE(double x1, double y1, double x2, double y2);
        int computeSW(double x1, double y1, double x2, double y2);
        double computeDFESync(void);
    
        double __attribute__ ((aligned (16))) inputBuffer[2][4];
        uint16_t *outputBuffer[2];
        int currentBuffer;


    public:
        uint16_t *outputArray;

        FractalEngine();
        virtual ~FractalEngine();
        virtual int init(int xSize, int ySize, int iters = 128); // Initialise engine, return 0 if successful, 1 if DFE unavailable
        virtual int computeImage(double x1, double y1, double x2, double y2); // Start non-blocking compute, return 0 if successful
        virtual int waitComputeFinished(double &computeTime); // Finish computing and report compute time in seconds
        virtual void useDFE(bool yesOrNo); // true to use DFE, false otherwise
        virtual int getIterations();
        virtual void setMandelbrot(void);
        virtual void setJulia(double c_real, double c_imag);
};

extern "C" double timediff(struct timeval begin, struct timeval end);

#endif /* _MAX_APP_H */
