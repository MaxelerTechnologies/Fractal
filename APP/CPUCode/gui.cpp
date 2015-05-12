#include "wx/wx.h"
#include "wx/glcanvas.h"
#include "wx/cmdline.h"
#include "maxapp.h"

#include "MaxelerIcon.xpm"

#include <sys/time.h>

#define DISP_WIDTH 1024
#define DISP_HEIGHT 768
#define ITERATIONS 128

// Demo set-up
//#define RECORD_DEMO

/*  DEMO ZOOM SEQUENCE 
    This is an array of 3-tuples {a, b, c} 
    a : zoom factor --> positive for zoom IN, negative for zoom OUT, "1" means pan
    b : x-coordinate of zoom
    c : y-coordinate of zoom
*/

#define DEMO_SEQUENCE \
    {4, -0.15, -0.95625}, {4, -0.160937, -1}, {4, -0.110547, -0.877734}, {4, -0.124902, -0.838281}, {4, -0.126733, -0.839868}, {4, -0.127197, -0.839307}, {1, -0.126425, -0.839326}, {1, -0.125656, -0.839127}, {1, -0.124899, -0.839148}, {1, -0.124187, -0.839011}, {1, -0.123437, -0.83902}, {1, -0.122697, -0.839061}, {1, -0.123257, -0.839613}, {1, -0.124014, -0.839842}, {1, -0.12477, -0.840233}, {1, -0.125496, -0.840475}, {1, -0.126213, -0.84081}, {1, -0.126974, -0.841177}, {1, -0.127579, -0.841737}, {1, -0.128328, -0.842172}, {1, -0.129076, -0.842401}, {-2, -0.129726, -0.842949}, {-2, -0.131093, -0.844099}, {2, -0.13293, -0.846394}, {2, -0.13401, -0.847102}, {4, -0.134041, -0.847031}, {-4, -0.134205, -0.847168}, {-4, -0.134351, -0.847733}, {-4, -0.133973, -0.849802}, {-4, -0.13451, -0.858347}, {4, -0.177381, -0.824558}, {4, -0.188563, -0.820871}, {4, -0.188752, -0.821634}, {4, -0.189358, -0.821903}, {-4, -0.189199, -0.822014}, {1, -0.189611, -0.821381}, {1, -0.188958, -0.820798}, {1, -0.188239, -0.820412}, {-4, -0.188122, -0.819927}, {-4, -0.18811, -0.81765}, {-4, -0.194213, -0.807738}, {-4, -0.211401, -0.768871}, {4, -0.161401, -0.640355}, {4, -0.161987, -0.65393}, {4, -0.159447, -0.654149}, {4, -0.159191, -0.654174}, {4, -0.159209, -0.654279}, {1, -0.15902, -0.654207}, {1, -0.15883, -0.654339}, {1, -0.158699, -0.654242}, {1, -0.158518, -0.654145}, {1, -0.158329, -0.654029}, {1, -0.158186, -0.654171}, {1, -0.158077, -0.654309}, {-2, -0.157981, -0.654434}, {-2, -0.157645, -0.654665}, {-4, -0.157632, -0.654556}, {-4, -0.157565, -0.654782}, {-4, -0.158005, -0.651877}, {-4, -0.165036, -0.65051}, {-4, -0.158005, -0.648557}
#define DEMO_SIZE 61

/* The golden ration */
#define PHI 1.6180339887498948482

double zoomSequence[][3] = { DEMO_SEQUENCE };

/* Demo speed counters */
struct timeval demoStartTime, demoEndTime;
double cumulativeDemoComputeTime;

class demoFrame;

class demoApp : public wxApp 
{
    virtual bool OnInit();
    virtual int OnExit();
    virtual void OnInitCmdLine(wxCmdLineParser& parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);

public:
};

class drawingPanel : public wxGLCanvas
{
public:
    #ifdef RECORD_DEMO
    static const bool useTargetPtr = false;
    #else
    static const bool useTargetPtr = true;
    #endif

    double zoomFactor;

    bool initialRedraw;
    bool flyZoom;
    bool overlap;
    
    /* Automatic demo stuff */
    bool autoDemoRunning;
    int demoIndex;
    int recordedDemoSize;
    
    int flySteps;
    double flyTargetX, flyTargetY, flyDeltaX, flyDeltaY;
    double flyZoomFactor;

    bool frameWaiting;
    
    /* Zoom capability constants */
    double largestViewValue, smallestViewValue;

    demoFrame *parentFrame;
    FractalEngine *fractalEngine;
    double x1, y1, x2, y2;

    bool isMandelbrot;
    double c_real, c_imag;
    
    GLfloat m_red[256], m_green[256], m_blue[256];

    drawingPanel(wxWindow *parent, wxWindow *topContainer, const wxSize& size, int differentColours) : wxGLCanvas(topContainer, -1, wxDefaultPosition, size)
    {
        int i;
        frameWaiting = false;
        overlap = true;
        parentFrame = (demoFrame*) parent;
#ifdef USE32BIT
        largestViewValue = 4;
        smallestViewValue = pow(2.00, -22);
        //fprintf(stderr,"(24)Setting view value to %g\n", smallestViewValue);
#else
        largestViewValue = 4;
        smallestViewValue = pow(2.00, -52);
        //fprintf(stderr,"(52)Setting view value to %g\n", smallestViewValue);
#endif
        autoDemoRunning = false;
        recordedDemoSize = 0;
        // set up GL colourmap
        for (i=0; i<256; i++) {
            GLfloat r,g,b;
            r = (i % differentColours) / (float)differentColours;
            g = (i % (differentColours / 2)) / (float)(differentColours / 2);
            b = (i % (differentColours / 4)) / (float)(differentColours / 4);
            m_red[i] = r; 
            m_green[i] = g;  
            m_blue[i] = b;
        }
        
        initialRedraw = true;
        zoomFactor = 4.0; /* default zoom factor */
        flyZoom = true; /* By default, zoom in steps rather than flying */
        flySteps = 0; 

        c_real = 0;
        c_imag = 0;
        isMandelbrot = true;
    }

    void OnLeftClick(wxMouseEvent& event); 
    void OnRightClick(wxMouseEvent& event); 
    void OnIdle(wxIdleEvent& event);
    void OnPaint(wxPaintEvent& event); 
	void OnScroll(wxMouseEvent& event);

    virtual void drawMB(uint16_t *values);

    void zoomIn(int x, int y, double zoomFactor, bool displayTarget);
    void zoomOut(int x, int y, double zoomFactor, bool displayTarget);
    int zoomInReal(double realX, double realY, double zoomFactor);
    int zoomOutReal(double realX, double realY, double zoomFactor);
    int render(double x1, double y1, double x2, double y2, double &computeTime);

    void setupFlyZoom(double realX, double realY, double zoomFactor);

    void initDisplay(); // Set initial zoom etc
    DECLARE_EVENT_TABLE()
};

class demoFrame: public wxFrame
{
public:
    drawingPanel *panel;
    wxMenuItem *dfeSelect, *swSelect, *demoItem;
    wxMenuItem **zoomItems, *flyItem;
    wxMenuItem **fractalTypeItems;
    wxMenu *menuFile;
	wxPanel *topPanel;
	wxRadioButton *realPart, *imagPart;
    FractalEngine *fractalEngine;
    demoApp *app;

    demoFrame(demoApp *app, const wxString& title, const wxPoint& pos);

    void OnQuit(wxCommandEvent& event); 
    void OnAbout(wxCommandEvent& event);
    void OnRunDemo(wxCommandEvent& event);
    void OnUseDFE(wxCommandEvent& event);
    void OnUseSoftware(wxCommandEvent& event);

    void setZoom1(wxCommandEvent& event);
    void setZoom15(wxCommandEvent& event);
    void setZoom2(wxCommandEvent& event);
    void setZoom4(wxCommandEvent& event);
    
    void OnFly(wxCommandEvent& event);
    void OnOverlap(wxCommandEvent& event);
    
    void setFractalType(wxCommandEvent& event);
    
    // Set GUI status for GUI
    void demoGo(); 
    void demoStop(bool finished = false); 
    
    DECLARE_EVENT_TABLE()
};
                    

enum {
    ID_Quit = 1,
    ID_About,
    ID_Software,
    ID_DFE,
    ID_RunDemo,
    ID_ZoomMenu,
    ID_Zoom1,
    ID_Zoom1Point5,
    ID_Zoom2,
    ID_Zoom4,
    ID_Fly,
    ID_Overlap,
    ID_FractalType,
    ID_Mandelbrot,
    ID_Julia1,
    ID_Julia2,
    ID_Julia3,
    ID_Julia4,
    ID_Julia5,
    ID_Julia6,
    ID_Julia7,
    ID_Julia8
};

BEGIN_EVENT_TABLE(demoFrame, wxFrame)
    EVT_MENU(ID_Quit, demoFrame::OnQuit)
    EVT_MENU(ID_About, demoFrame::OnAbout)
    EVT_MENU(ID_RunDemo, demoFrame::OnRunDemo)
    EVT_MENU(ID_Software, demoFrame::OnUseSoftware)
    EVT_MENU(ID_DFE, demoFrame::OnUseDFE)
    EVT_MENU(ID_Zoom1, demoFrame::setZoom1)
    EVT_MENU(ID_Zoom1Point5, demoFrame::setZoom15)
    EVT_MENU(ID_Zoom2, demoFrame::setZoom2)
    EVT_MENU(ID_Zoom4, demoFrame::setZoom4)
    EVT_MENU(ID_Fly, demoFrame::OnFly)
    EVT_MENU(ID_Overlap, demoFrame::OnOverlap)
    EVT_MENU(ID_Mandelbrot, demoFrame::setFractalType)
    EVT_MENU(ID_Julia1, demoFrame::setFractalType)
    EVT_MENU(ID_Julia2, demoFrame::setFractalType)
    EVT_MENU(ID_Julia3, demoFrame::setFractalType)
    EVT_MENU(ID_Julia4, demoFrame::setFractalType)
    EVT_MENU(ID_Julia5, demoFrame::setFractalType)
    EVT_MENU(ID_Julia6, demoFrame::setFractalType)
    EVT_MENU(ID_Julia7, demoFrame::setFractalType)
    EVT_MENU(ID_Julia8, demoFrame::setFractalType)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(drawingPanel, wxGLCanvas)
    EVT_PAINT(drawingPanel::OnPaint)
    EVT_LEFT_DOWN(drawingPanel::OnLeftClick)
    EVT_RIGHT_DOWN(drawingPanel::OnRightClick)
    EVT_MOUSEWHEEL(drawingPanel::OnScroll)
    EVT_IDLE(drawingPanel::OnIdle)
END_EVENT_TABLE()
IMPLEMENT_APP(demoApp)

void drawingPanel::OnLeftClick(wxMouseEvent &event) {
    int x, y;
    if (autoDemoRunning) {
        parentFrame->demoStop();
    } else {
        x = event.GetX();
        y = event.GetY();
        zoomIn(x, y, zoomFactor, useTargetPtr);
    }
    event.Skip(false);
}

void drawingPanel::OnRightClick(wxMouseEvent &event) {
    int x, y;
    if (autoDemoRunning) {
        parentFrame->demoStop();
    } else {
        x = event.GetX();
        y = event.GetY();
        zoomOut(x, y, zoomFactor, useTargetPtr);
    }        
    event.Skip(false);
}

void drawingPanel::OnScroll(wxMouseEvent &event) {
    if (isMandelbrot) {
        return;
    }

    double incr = 0.01 * event.GetWheelRotation() / event.GetWheelDelta();
    if (parentFrame->imagPart->GetValue()) {
        if (c_imag + incr < 2 && c_imag + incr > -2) {
            c_imag += incr;
        }
    } else {
        if (c_real + incr < 2 && c_real + incr > -2) {
            c_real += incr;
        }
    }
    fractalEngine->setJulia(c_real, c_imag);
    
    if (!frameWaiting) {
        double computeTime;

        fractalEngine->computeImage(x1, y1, x2, y2);
        fractalEngine->waitComputeFinished(computeTime);
        drawMB(fractalEngine->outputArray);

        char str[256];
        sprintf(str, "C (real, imag) = (%f, %f)", c_real, c_imag);
        parentFrame->SetStatusText(wxString::FromAscii(str));
    }
}

/* The idle event runs the demo - if the demo is currently running,
   otherwise it does nothing and waits for user input */
void drawingPanel::OnIdle(wxIdleEvent &event) {
    double *demoData;

    if (flySteps == 0 && overlap && frameWaiting && (!autoDemoRunning)) {
        drawMB(fractalEngine->outputArray);
        frameWaiting = false;
        event.RequestMore(true);
        return;
    }
    
    if (flySteps != 0) {
        int ret; 
        flySteps--;
        flyTargetX = flyTargetX + flyDeltaX;
        flyTargetY = flyTargetY + flyDeltaY;
        if (flyZoomFactor > 0) ret = zoomInReal(flyTargetX, flyTargetY, flyZoomFactor);
        else ret = zoomOutReal(flyTargetX, flyTargetY, -flyZoomFactor);
        if (ret == -1) {
            flySteps = 0; // Abort zoom sequence
        }       
    }
    if (flySteps == 0) {
        if (autoDemoRunning) {
            /* Run the next step of the demo */
            if (demoIndex >= DEMO_SIZE) {
                parentFrame->demoStop(true);
                return;
            }
        
            demoData = zoomSequence[demoIndex];
            demoIndex++;
        
            if (flyZoom)
                setupFlyZoom(demoData[1], demoData[2], demoData[0]);
            else {
                if (demoData[0] < 0)
                    zoomOutReal(demoData[1], demoData[2], -demoData[0]);
                else
                    zoomInReal(demoData[1], demoData[2], demoData[0]);
            }
            event.RequestMore(true);
        } 
    }
    if (initialRedraw) {
        initDisplay();
        initialRedraw = false;
    }
}

void drawingPanel::zoomIn(int x, int y, double zoomFactor, bool displayTarget __attribute__((unused))) {

    parentFrame->SetStatusText( _("Please wait") );

    double realX, realY;
    realX = (((double) x) / ((double) DISP_WIDTH))*(x2-x1) + x1;
    realY = (((double) y) / ((double) DISP_HEIGHT))*(y2-y1) + y1;

    if (flyZoom) {
        setupFlyZoom(realX, realY, zoomFactor);
    } else // single step zoom
        zoomInReal(realX, realY, zoomFactor);
}

void drawingPanel::setupFlyZoom(double realX, double realY, double zoomFactor) {
    // Zoom in, using lots of steps, this is actually processed in the idle handler
    int numSteps;
    double newZoomFactor;
    double stepMultiplier = 15;
    double dX, dY, delta;
    const double VIEW_CONST = 64; //32;
    double currX, currY;
 
    #ifdef RECORD_DEMO
    fprintf(stderr, "{%g, %g, %g}, ", zoomFactor, realX, realY); 
    recordedDemoSize++;
    #endif

    // Work out deltas to move view over
    currX = x1 + (x2-x1)/2;
    currY = y1 + (y2-y1)/2;
    dX = realX - currX;         
    dY = realY - currY;

    numSteps = (int) (stepMultiplier * fabs(zoomFactor)); // Number of steps to zoom using

    // Use fewer steps if there is insufficient precision
    delta = fabs(dX) / ((double) numSteps);
    int loopIters = 0;
    while ((delta <= (smallestViewValue * VIEW_CONST)) && numSteps > 4) {
        numSteps = (int) (stepMultiplier * fabs(zoomFactor));
        delta = fabs(dX) / ((double) numSteps);
        loopIters++;
        if (stepMultiplier != 1) stepMultiplier--;
    }

    // Now calculate delta for each step. Round to fixed point, then re-calc number of steps to achieve it
    dX = dX / (double) numSteps;
    dY = dY / (double) numSteps;
    newZoomFactor = pow(fabs(zoomFactor), 1/(double) numSteps);
    if (zoomFactor < 0) newZoomFactor = -newZoomFactor;

    flySteps = numSteps;
    flyZoomFactor = newZoomFactor;
    flyTargetX = currX; flyTargetY = currY;
    flyDeltaX = dX;
    flyDeltaY = dY;
}

int drawingPanel::render(double x1, double y1, double x2, double y2, double &computeTime) {

    fractalEngine->computeImage(x1, y1, x2, y2);

    if (overlap) drawMB(fractalEngine->outputArray);
    
    fractalEngine->waitComputeFinished(computeTime);

    if (!overlap) drawMB(fractalEngine->outputArray);

    if (computeTime == -1.0) {
        parentFrame->SetStatusText(_("Error encountered calculating fractal"));
        return -1;
    }

    return 0;
}

// Zoom in with REAL co-ordinates
int drawingPanel::zoomInReal(double realX, double realY, double zoomFactor) {
    double new_w, new_h, new_x1, new_y1, new_x2, new_y2;
    double computeTime;

    new_w = (x2 - x1) / zoomFactor;
    new_h = (y2 - y1) / zoomFactor;

    new_x1 = realX - new_w/2;
    new_y1 = realY - new_h/2;
    new_x2 = realX + new_w/2;
    new_y2 = realY + new_h/2;
    
    /* Check whether we're still in the allowable range of zoom values */
    if ((new_h / DISP_HEIGHT) <= smallestViewValue || (new_w / DISP_WIDTH) <= smallestViewValue) {
        parentFrame->SetStatusText( _("You are at maximum zoom") );
        return -1;
    }

     if ((fabs(new_x1) >= largestViewValue) || 
        ((new_x2) >= largestViewValue) || 
        (fabs(new_y1) >= largestViewValue) ||
        ((new_y2) >= largestViewValue)) {
        parentFrame->SetStatusText( _("You have reached the viewport boundaries") );
        return -1;
    }

    #ifdef RECORD_DEMO
    if (!flyZoom) { // If fly zoom, print this in set-up fly zoom function
        fprintf(stderr, "{%g, %g, %g}, ", zoomFactor, realX, realY); 
        recordedDemoSize++;
    }
    #endif

    render(new_x1, new_y1, new_x2, new_y2, computeTime);

    if (!autoDemoRunning) {
        char *charmsg = new char[300];
        sprintf(charmsg, "View centered on (%.5f, %.5f), calculation time %g seconds", realX, realY, computeTime);
        parentFrame->SetStatusText(wxString::FromAscii(charmsg));
        delete[] charmsg;
    } else cumulativeDemoComputeTime += computeTime;

    frameWaiting = true;

    x1 = new_x1;
    y1 = new_y1;
    x2 = new_x2;
    y2 = new_y2; 
    
    return 0;
}

void drawingPanel::zoomOut(int x, int y, double zoomFactor, bool displayTarget __attribute__((unused))) {
    
    double realX, realY;
    realX = (((double) x) / ((double) DISP_WIDTH))*(x2 - x1) + x1;
    realY = (((double) y) / ((double) DISP_HEIGHT))*(y2 - y1) + y1;

    if (flyZoom) 
        setupFlyZoom(realX, realY, -zoomFactor);
    else 
        zoomOutReal(realX, realY, zoomFactor);
}

int drawingPanel::zoomOutReal(double realX, double realY, double zoomFactor) {
    double new_w, new_h;
    double new_x1, new_y1, new_x2, new_y2;
    double computeTime;
   
    new_w = (x2 - x1) * zoomFactor;
    new_h = (y2 - y1) * zoomFactor;

    new_x1 = realX - new_w/2;
    new_y1 = realY - new_h/2;
    new_x2 = realX + new_w/2;
    new_y2 = realY + new_h/2;

    /* Check whether we're still in the allowable range of zoom values */

    if ((fabs(new_x1) >= largestViewValue) || 
        ((new_x2) >= largestViewValue) || 
        (fabs(new_y1) >= largestViewValue) ||
        ((new_y2) >= largestViewValue)) {
        parentFrame->SetStatusText( _("You have reached the viewport boundaries") );
        return -1;
    }
    
    #ifdef RECORD_DEMO
    if (!flyZoom) {
        fprintf(stderr, "{%g, %g, %g}, ", -zoomFactor, realX, realY);
        recordedDemoSize++;
    }
    #endif

    render(new_x1, new_y1, new_x2, new_y2, computeTime);
    
    if (!autoDemoRunning) {
        char *charmsg = new char[300];
        sprintf(charmsg, "View centered on (%.5f, %.5f), calculation time %g seconds", realX, realY, computeTime);
        parentFrame->SetStatusText(wxString::FromAscii(charmsg));
        delete[] charmsg;
    } else cumulativeDemoComputeTime += computeTime;

    frameWaiting = true;

    x1 = new_x1;
    y1 = new_y1;
    x2 = new_x2;
    y2 = new_y2; 

    return 0;
}

static const wxCmdLineEntryDesc g_cmdLineDesc[] = 
{
    {wxCMD_LINE_NONE}
};

bool demoApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    demoFrame *frame = new demoFrame( this, _("Maxeler Fractal Demo") , wxPoint(50,50));
    frame->Show(TRUE);
    SetTopWindow(frame);
    
    return TRUE;
} 

int demoApp::OnExit()
{
    return wxApp::OnExit();
}

void demoApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc(g_cmdLineDesc);
}

bool demoApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    wxString strValue;

    return wxApp::OnCmdLineParsed(parser);
}

demoFrame::demoFrame(demoApp *app, const wxString& title, const wxPoint& pos)
: wxFrame((wxFrame *)NULL, -1, title, pos, wxDefaultSize, wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{
    menuFile = new wxMenu;
    wxMenu *menuZoom = new wxMenu;
    wxMenu *menuFractalType = new wxMenu;
    wxMenuItem *overlapItem, *flyItem;

    swSelect = menuFile->Append( ID_Software, _("Use &Software"), _("Use software implementation"), wxITEM_RADIO );
    dfeSelect = menuFile->Append( ID_DFE, _("Use &DFE"), _("Use DFE implementation"), wxITEM_RADIO );
    dfeSelect->Check();
    menuFile->AppendSeparator();

    fractalTypeItems = new wxMenuItem*[8];	
    fractalTypeItems[0] = menuFractalType->Append( ID_Mandelbrot, _("Mandelbrot"), _("c = z0"), wxITEM_RADIO);
    fractalTypeItems[1] = menuFractalType->Append( ID_Julia1, _("Julia 1"), _("c = 1-phi"), wxITEM_RADIO);
    fractalTypeItems[2] = menuFractalType->Append( ID_Julia2, _("Julia 2"), _("c = (phi-2) + (phi-1)i"), wxITEM_RADIO);
    fractalTypeItems[3] = menuFractalType->Append( ID_Julia3, _("Julia 3"), _("c = 0.285"), wxITEM_RADIO);
    fractalTypeItems[4] = menuFractalType->Append( ID_Julia4, _("Julia 4"), _("c = 0.285 + 0.01i"), wxITEM_RADIO);
    fractalTypeItems[5] = menuFractalType->Append( ID_Julia5, _("Julia 5"), _("c = 0.45 + 0.1428i"), wxITEM_RADIO);
    fractalTypeItems[6] = menuFractalType->Append( ID_Julia6, _("Julia 6"), _("c = -0.70176 - 0.3842i"), wxITEM_RADIO);
    fractalTypeItems[7] = menuFractalType->Append( ID_Julia7, _("Julia 7"), _("c = -0.835 - 0.2321i"), wxITEM_RADIO);
    fractalTypeItems[8] = menuFractalType->Append( ID_Julia8, _("Julia 8"), _("c = -0.8 + 0.156i"), wxITEM_RADIO);
    fractalTypeItems[0]->Check(true);
	
    menuFile->Append(ID_FractalType, _("Fractal type"), menuFractalType, _("Select a fractal type"));
    menuFile->AppendSeparator();

    zoomItems = new wxMenuItem*[4];
    zoomItems[0] = menuZoom->Append( ID_Zoom1, _("&1 : 1 (Pan)"), _("Pan rather than zoom"), wxITEM_RADIO);
    zoomItems[1] = menuZoom->Append( ID_Zoom1Point5, _("1.&5 : 1"), _("Zoom by 1.5"), wxITEM_RADIO);
    zoomItems[2] = menuZoom->Append( ID_Zoom2, _("&2 : 1"), _("Zoom by 2"), wxITEM_RADIO);
    zoomItems[3] = menuZoom->Append( ID_Zoom4, _("&4 : 1"), _("Zoom by 4"), wxITEM_RADIO);
    zoomItems[3]->Check(true);

    flyItem = menuZoom->Append(ID_Fly, _("Zoom by &flying"), _("Fly in/out in small steps when zooming"), wxITEM_CHECK);
    flyItem->Check(true);
    
    menuFile->Append(ID_ZoomMenu, _("&Zoom Settings"), menuZoom, _("Configure zoom factor and single step/flying behaviour"));
    
    overlapItem = menuFile->Append(ID_Overlap, _("&Overlapping"), _("Overlap DFE execution with CPU"), wxITEM_CHECK);
    overlapItem->Check(true);

    menuFile->AppendSeparator();

    //menuFile->Append( ID_About, _("&About...") );
    demoItem = menuFile->Append( ID_RunDemo, _("Run &Demo..."), _("Run the automatic demo zoom sequence") );
    menuFile->AppendSeparator();
    menuFile->Append( ID_Quit, _("E&xit") );

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( menuFile, _("&Configuration") );

    SetMenuBar( menuBar );

    CreateStatusBar();
    SetStatusText( _("Welcome to Maxeler Fractal Demo...") );
    
    this->app = app;

	topPanel = new wxPanel(this);
	wxPanel *middlePanel = new wxPanel(this);

	wxBoxSizer *frameSizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(frameSizer);
	frameSizer->Add(topPanel);
	frameSizer->Add(middlePanel);

	wxBoxSizer *topPanelSizer = new wxBoxSizer(wxHORIZONTAL);
	topPanel->SetSizer(topPanelSizer);
	wxStaticText *wheelText = new wxStaticText(topPanel, -1, _("Use the mouse wheel to change the complex number c:"));
	realPart = new wxRadioButton(topPanel, -1, _("Re(c)"));
	imagPart = new wxRadioButton(topPanel, -1, _("Im(c)"));
	topPanelSizer->Add(wheelText, 0, wxALIGN_CENTER_VERTICAL);
	topPanelSizer->AddSpacer(30);
	topPanelSizer->Add(realPart, 0, wxALIGN_CENTER_VERTICAL);
	topPanelSizer->Add(imagPart, 0, wxALIGN_CENTER_VERTICAL);
	topPanel->Disable();


    // Initialise Fractal generator and generate initial fractal
    fractalEngine = new FractalEngine();
    fractalEngine->useDFE(true);
    if (fractalEngine->init(DISP_WIDTH, DISP_HEIGHT, ITERATIONS)) { // If returns 1, no DFE available
        dfeSelect->Enable(false);
        dfeSelect->Check(false);
        swSelect->Check(true);
        SetStatusText( _("No MAX-1 accelerator detected, running in software mode only") );
    }

    panel = new drawingPanel(this, middlePanel, wxSize(DISP_WIDTH,DISP_HEIGHT), fractalEngine->getIterations());
    panel->fractalEngine = fractalEngine;

	SetClientSize(frameSizer->ComputeFittingClientSize(this));
    SetIcon(wxIcon(MaxelerIcon_xpm));
}

void drawingPanel::OnPaint(wxPaintEvent& event __attribute__((unused)))
{
    wxPaintDC dc(this);
    SetCurrent();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0,0.0);
    glVertex2f(-1.0,1.0);
    glTexCoord2f(0.0,0.75); // FIXME! Non power-of-two or 4:3 aspect ratio
    glVertex2f(-1.0,-1.0);
    glTexCoord2f(1.0,0.75);
    glVertex2f(1.0,-1.0);
    glTexCoord2f(1.0,0.0);
    glVertex2f(1.0,1.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glFlush();
    SwapBuffers();
}

void drawingPanel::initDisplay() {
    x1 = -3.2;
    y1 = -2.4;
    x2 = 3.2;
    y2 = 2.4;
    fractalEngine->computeImage(-3.2, -2.4, 3.2, 2.4); // Initial display range
    double a;
    fractalEngine->waitComputeFinished(a);
    drawMB(fractalEngine->outputArray);
}

void drawingPanel::drawMB(uint16_t *values) {
    // generate OpenGL texture
    SetCurrent();

    // colour map from index to RGB space
    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 256, m_red);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 256, m_green);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 256, m_blue);
    glPixelTransferi(GL_MAP_COLOR, TRUE);

    // fractal texture: indexed, 1 short value per-pixel
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    // FIXME, size other than 1024x768, should choose next highest power of 2
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_COLOR_INDEX, GL_UNSIGNED_SHORT, values);
    Refresh(false);
}


void demoFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    delete fractalEngine;
#ifdef RECORD_DEMO
    fprintf(stderr,"\n\nDEMO_SIZE %d\n", panel->recordedDemoSize);
#endif
    Close(TRUE);
}


void demoFrame::OnUseDFE(wxCommandEvent& WXUNUSED(event))
{
    swSelect->Check(false);
    dfeSelect->Check(true);
    fractalEngine->useDFE(true);
    SetStatusText(_("Using DFE for compute"));
}

void demoFrame::setZoom1(wxCommandEvent& WXUNUSED(event)) {
    panel->zoomFactor = 1.0;
    SetStatusText(_("Set zoom factor to 1:1 (pan)"));
}
    
void demoFrame::setZoom15(wxCommandEvent& WXUNUSED(event)) {
    panel->zoomFactor = 1.5;
    SetStatusText(_("Set zoom factor to 1.5:1"));
}
    
void demoFrame::setZoom2(wxCommandEvent& WXUNUSED(event)) {
    panel->zoomFactor = 2.0;
    SetStatusText(_("Set zoom factor to 2:1"));
}

void demoFrame::setZoom4(wxCommandEvent& WXUNUSED(event)) {
    panel->zoomFactor = 4.0;
    SetStatusText(_("Set zoom factor to 4:1"));
}

void demoFrame::OnFly(wxCommandEvent& WXUNUSED(event)) {
    panel->flyZoom = !(panel->flyZoom);
}

void demoFrame::OnOverlap(wxCommandEvent& WXUNUSED(event)) {
    panel->overlap = !(panel->overlap);
}

void demoFrame::setFractalType(wxCommandEvent& event) {
    int itemId = event.GetId();
    if (itemId == ID_Mandelbrot) {
        panel->isMandelbrot = true;
		topPanel->Disable();
        fractalEngine->setMandelbrot();	
    } else {
        panel->isMandelbrot = false;
		topPanel->Enable();
        switch (itemId) {
            case ID_Julia1:
                panel->c_real = 1 - PHI;
                panel->c_imag = 0;
                break;
            case ID_Julia2:
                panel->c_real = PHI - 2;
                panel->c_imag = PHI - 1;
                break;
            case ID_Julia3:
                panel->c_real = 0.285;
                panel->c_imag = 0;
                break;
            case ID_Julia4:
                panel->c_real = 0.285;
                panel->c_imag = 0.01;
                break;
            case ID_Julia5:
                panel->c_real = 0.45;
                panel->c_imag = 0.1428;
                break;
            case ID_Julia6:
                panel->c_real = -0.70176;
                panel->c_imag = 0.3842;
                break;
            case ID_Julia7:
                panel->c_real = -0.835;
                panel->c_imag = -0.2321;
                break;
            case ID_Julia8:
                panel->c_real = -0.8;
                panel->c_imag = 0.156;
                break;
        }
        fractalEngine->setJulia(panel->c_real, panel->c_imag);
    }
    panel->flySteps = 0;
    panel->initDisplay();


}

void demoFrame::OnUseSoftware(wxCommandEvent& WXUNUSED(event))
{
    dfeSelect->Check(false);
    swSelect->Check(true);
    fractalEngine->useDFE(false);
    SetStatusText(_("Using CPU for compute"));
}

void demoFrame::OnRunDemo(wxCommandEvent& WXUNUSED(event)) {
    if (panel->autoDemoRunning) {
        demoStop();
    } else {
        demoGo();
        // Reset to default view before running demo
        panel->isMandelbrot = true;
        fractalEngine->setMandelbrot();
        panel->autoDemoRunning = true;
        panel->demoIndex = 0;
        panel->initDisplay();
    }
}

void demoFrame::demoGo() {
    demoItem->SetText(_("Stop &Demo..."));
    SetStatusText(_("Running automatic demo..."));
    fractalTypeItems[0]->Check(true);
    menuFile->Enable(ID_FractalType, false);
	topPanel->Disable();

    cumulativeDemoComputeTime = 0;
    gettimeofday(&demoStartTime, NULL);
}
 
void demoFrame::demoStop(bool finished) {
    char *charmsg = new char[300];
    char *beginmsg = new char[100];
    double realTime;
    if (finished) /* reached the end */
        sprintf(beginmsg, "Automatic demo finished. ");
    else
        sprintf(beginmsg,"Automatic demo stopped. ");

    gettimeofday(&demoEndTime, NULL);

    realTime = timediff(demoStartTime, demoEndTime);
    realTime = realTime / 1E6;

    sprintf(charmsg, "%s Total computeStart time was %.2f s, real time was %.2f s", beginmsg, cumulativeDemoComputeTime, realTime);
    SetStatusText(wxString::FromAscii(charmsg));

    delete[] charmsg;
    delete[] beginmsg;

    demoItem->SetText(_("Run &Demo...") );
    panel->autoDemoRunning = false;
    panel->flySteps = 0; // Stop flying as well
    
    menuFile->Enable(ID_FractalType, true);
	
	topPanel->Enable();
} 

void demoFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(_("Maxeler Technologies Fractal Demo"),
        _("About Maxeler Fractal Demo"), wxOK | wxICON_INFORMATION, this);
}

