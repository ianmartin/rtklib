//---------------------------------------------------------------------------
#ifndef plotmainH
#define plotmainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include <Buttons.hpp>
#include <Menus.hpp>
#include "graph.h"
#include "rtklib.h"

#define MAXNFILE    32                  // max number of solution files
#define MAXSTRBUFF  1024                // max length of stream buffer
#define MAXMAPPNT   10                  // max number of points

//---------------------------------------------------------------------------
class TIMEPOS
{
private:
    int nmax_;
public:
    int n;
    gtime_t *t;
    double *x,*y,*z,*xs,*ys,*zs,*xys;
    int *q;
    TIMEPOS(int nmax, int sflg);
    ~TIMEPOS();
    TIMEPOS *tdiff(void);
    TIMEPOS *diff(const TIMEPOS *pos2);
};
//---------------------------------------------------------------------------
class TPlot : public TForm
{
__published:
    TPanel *Panel1;
    TPanel *Panel2;
    TPanel *Panel11;
    TPanel *Panel21;
    TPanel *Panel22;
    TPanel *Panel10;
    TPanel *Panel12;
    TPanel *StrStatus;
    
    TComboBox *PlotTypeS;
    TComboBox *QFlag;
    TComboBox *ObsType;
    TComboBox *DopType;
    TComboBox *SatList1;
    TComboBox *SatList2;
    TSpeedButton *BtnConnect;
    TSpeedButton *BtnSol1;
    TSpeedButton *BtnSol2;
    TSpeedButton *BtnSol12;
    TSpeedButton *BtnFitHoriz;
    TSpeedButton *BtnFitVert;
    TSpeedButton *BtnCenterOri;
    TSpeedButton *BtnFixHoriz;
    TSpeedButton *BtnFixVert;
    TSpeedButton *BtnShowTrack;
    TSpeedButton *BtnShowPoint;
    TSpeedButton *BtnOn1;
    TSpeedButton *BtnOn2;
    TSpeedButton *BtnOn3;
    TSpeedButton *BtnAnimate;
    TSpeedButton *BtnClear;
    TSpeedButton *BtnReload;
    TScrollBar *SolScroll;
    TScrollBar *ObsScroll;
    
    TPaintBox *Disp;
    
    TLabel *ConnectMsg;
    TLabel *Message1;
    TLabel *Message2;
    TLabel *QL1;
    TLabel *QL2;
    TLabel *QL3;
    TLabel *QL4;
    TLabel *QL5;
    TLabel *QL6;
    TLabel *QL7;
    
    TMainMenu *MainMenu;
    TMenuItem *MenuFile;
    TMenuItem *MenuOpenSol1;
    TMenuItem *MenuOpenSol2;
    TMenuItem *MenuOpenNav;
    TMenuItem *MenuElevMask;
    TMenuItem *MenuReload;
    TMenuItem *MenuConnect;
    TMenuItem *MenuDisconnect;
    TMenuItem *MenuPort;
    TMenuItem *MenuClear;
    TMenuItem *MenuQuit;
    TMenuItem *MenuEdit;
    TMenuItem *MenuTime;
    TMenuItem *MenyCopy;
    TMenuItem *MenuMonitor;
    TMenuItem *MenuQcObs;
    TMenuItem *MenuSrcSol;
    TMenuItem *MenuSrcObs;
    TMenuItem *MenuPoint;
    TMenuItem *MenuOptions;
    TMenuItem *MenuView;
    TMenuItem *MenuToolBar;
    TMenuItem *MenuStatusBar;
    TMenuItem *MenuOpenObs;
    TMenuItem *MenuCenterOri;
    TMenuItem *MenuFitHoriz;
    TMenuItem *MenuFitVert;
    TMenuItem *MenuShowTrack;
    TMenuItem *MenuFixHoriz;
    TMenuItem *MenuFixVert;
    TMenuItem *MenuShowPoint;
    TMenuItem *MenuAnimStart;
    TMenuItem *MenuAnimStop;
    TMenuItem *MenuHelp;
    TMenuItem *MenuAbout;
    
    TMenuItem *N1;
    TMenuItem *N2;
    TMenuItem *N3;
    TMenuItem *N5;
    TMenuItem *N6;
    TMenuItem *N7;
    TMenuItem *N8;
    TMenuItem *N9;
    TMenuItem *N10;
    TMenuItem *N11;
    TMenuItem *N12;
    TMenuItem *N13;
    
    TTimer *Timer;
    
    TOpenDialog *OpenSolDialog;
    TOpenDialog *OpenObsDialog;
    TOpenDialog *OpenElMaskDialog;
    
    void __fastcall FormCreate          (TObject *Sender);
    void __fastcall FormShow            (TObject *Sender);
    void __fastcall FormResize          (TObject *Sender);
    
    void __fastcall MenuOpenSol1Click   (TObject *Sender);
    void __fastcall MenuOpenSol2Click   (TObject *Sender);
    void __fastcall MenuOpenObsClick    (TObject *Sender);
    void __fastcall MenuOpenNavClick    (TObject *Sender);
    void __fastcall MenuElevMaskClick   (TObject *Sender);
    void __fastcall MenuConnectClick    (TObject *Sender);
    void __fastcall MenuDisconnectClick (TObject *Sender);
    void __fastcall MenuPortClick       (TObject *Sender);
    void __fastcall MenuReloadClick     (TObject *Sender);
    void __fastcall MenuClearClick      (TObject *Sender);
    void __fastcall MenuQuitClick       (TObject *Sender);
    
    void __fastcall MenuTimeClick       (TObject *Sender);
    void __fastcall MenyCopyClick       (TObject *Sender);
    void __fastcall MenuSrcSolClick     (TObject *Sender);
    void __fastcall MenuSrcObsClick     (TObject *Sender);
    void __fastcall MenuQcObsClick      (TObject *Sender);
    void __fastcall MenuPointClick      (TObject *Sender);
    void __fastcall MenuOptionsClick    (TObject *Sender);
    
    void __fastcall MenuToolBarClick    (TObject *Sender);
    void __fastcall MenuStatusBarClick  (TObject *Sender);
    void __fastcall MenuMonitorClick    (TObject *Sender);
    void __fastcall MenuCenterOriClick  (TObject *Sender);
    void __fastcall MenuFitHorizClick   (TObject *Sender);
    void __fastcall MenuFitVertClick    (TObject *Sender);
    void __fastcall MenuShowTrackClick  (TObject *Sender);
    void __fastcall MenuFixHorizClick   (TObject *Sender);
    void __fastcall MenuFixVertClick    (TObject *Sender);
    void __fastcall MenuShowPointClick  (TObject *Sender);
    void __fastcall MenuAnimStartClick  (TObject *Sender);
    void __fastcall MenuAnimStopClick   (TObject *Sender);
    void __fastcall MenuAboutClick      (TObject *Sender);
    
    void __fastcall BtnConnectClick     (TObject *Sender);
    void __fastcall BtnSol1Click        (TObject *Sender);
    void __fastcall BtnSol2Click        (TObject *Sender);
    void __fastcall BtnSol12Click       (TObject *Sender);
    void __fastcall BtnSol1DblClick     (TObject *Sender);
    void __fastcall BtnSol2DblClick     (TObject *Sender);
    void __fastcall BtnOn1Click         (TObject *Sender);
    void __fastcall BtnOn2Click         (TObject *Sender);
    void __fastcall BtnOn3Click         (TObject *Sender);
    void __fastcall BtnCenterOriClick   (TObject *Sender);
    void __fastcall BtnFitHorizClick    (TObject *Sender);
    void __fastcall BtnFitVertClick     (TObject *Sender);
    void __fastcall BtnShowTrackClick   (TObject *Sender);
    void __fastcall BtnFixHorizClick    (TObject *Sender);
    void __fastcall BtnFixVertClick     (TObject *Sender);
    void __fastcall BtnShowPointClick   (TObject *Sender);
    void __fastcall BtnAnimateClick     (TObject *Sender);
    void __fastcall BtnClearClick       (TObject *Sender);
    void __fastcall BtnReloadClick      (TObject *Sender);
    
    void __fastcall PlotTypeSChange     (TObject *Sender);
    void __fastcall QFlagChange         (TObject *Sender);
    void __fastcall ObsTypeChange       (TObject *Sender);
    void __fastcall DopTypeChange       (TObject *Sender);
    void __fastcall SatListChange       (TObject *Sender);
    void __fastcall SolScrollChange     (TObject *Sender);
    
    void __fastcall TimerTimer          (TObject *Sender);
    
    void __fastcall DispPaint           (TObject *Sender);
    void __fastcall DispMouseLeave      (TObject *Sender);
    void __fastcall DispMouseMove       (TObject *Sender, TShiftState Shift,
                                         int X, int Y);
    void __fastcall DispMouseDown       (TObject *Sender, TMouseButton Button,
                                         TShiftState Shift, int X, int Y);
    void __fastcall DispMouseUp         (TObject *Sender, TMouseButton Button,
                                         TShiftState Shift, int X, int Y);
    void __fastcall MouseWheel          (TObject *Sender, TShiftState Shift,
                                         int WheelDelta, TPoint &MousePos, bool &Handled);

private:
    Graphics::TBitmap *Buff;
    TGraph *GraphT;
    TGraph *GraphG[3];
    TGraph *GraphR;
    TGraph *GraphS;
    TStrings *SolFiles[2];
    TStrings *ObsFiles;
    TStrings *NavFiles;
    
    stream_t Stream;
    solbuf_t SolData[2];
    solstatbuf_t SolStat[2];
    obs_t Obs;
    nav_t Nav;
    double *Az,*El,ElMaskData[361];
    
    gtime_t OEpoch;
    int ConnectState;
    int NObs,*IndexObs;
    int Week;
    int Flush,PlotType,OPosType;
    int NSolF1,NSolF2,NObsF,NNavF;
    int SatMask[MAXSAT];
    double OPos[3],OVel[3];
    
    int Drag,Xn,Yn;
    double X0,Y0,Xc,Yc,Xs,Ys,Xcent,Xcent0;
    
    void __fastcall DropFiles    (TWMDropFiles msg);
    
    void __fastcall ReadSol      (TStrings *files, int sel);
    void __fastcall ReadSolStat  (TStrings *files, int sel);
    void __fastcall ReadObs      (TStrings *files);
    int  __fastcall ReadObsRnx   (TStrings *files, obs_t *obs, nav_t *nav);
    void __fastcall ReadNav      (TStrings *files);
    void __fastcall ReadElMaskData(AnsiString file);
    void __fastcall Connect      (void);
    void __fastcall Disconnect   (void);
    void __fastcall ConnectPath  (const char *path);
    int  __fastcall CheckObs     (AnsiString file);
    void __fastcall UpdateObs    (int nobs);
    void __fastcall Clear        (void);
    void __fastcall Refresh      (void);
    void __fastcall Reload       (void);
    
    void __fastcall UpdateDisp   (void);
    void __fastcall UpdateType   (int type);
    void __fastcall UpdatePlot   (void);
    void __fastcall UpdateSize   (void);
    void __fastcall UpdateColor  (void);
    void __fastcall UpdateTimeExt(void);
    void __fastcall UpdateOrigin (void);
    void __fastcall UpdateSatMask(void);
    void __fastcall UpdateInfo   (void);
    void __fastcall UpdateTimeSol(void);
    void __fastcall UpdateTimeObs(void);
    void __fastcall UpdateInfoSol(void);
    void __fastcall UpdateInfoObs(void);
    void __fastcall UpdatePoint  (int x, int y);
    void __fastcall UpdateEnable (void);
    void __fastcall UpdatePlotType(void);
    void __fastcall FitTime      (void);
    void __fastcall SetRange     (int all);
    void __fastcall FitRange     (int all);
    
    void __fastcall SetCentX     (double c);
    void __fastcall SetScaleX    (double s);
    void __fastcall MouseDownTrk (int X, int Y);
    void __fastcall MouseDownSol (int X, int Y);
    void __fastcall MouseDownObs (int X, int Y);
    void __fastcall MouseMoveTrk (int X, int Y, double dx, double dy, double dxs, double dys);
    void __fastcall MouseMoveSol (int X, int Y, double dx, double dy, double dxs, double dys);
    void __fastcall MouseMoveObs (int X, int Y, double dx, double dy, double dxs, double dys);
    
    void __fastcall DrawTrk      (int level);
    void __fastcall DrawTrkPnt   (const TIMEPOS *pos, int level, int style);
    void __fastcall DrawTrkPos   (const double *rr, int type, AnsiString label);
    void __fastcall DrawTrkStat  (const TIMEPOS *pos, AnsiString header, int p);
    void __fastcall DrawTrkError (const TIMEPOS *pos, int style);
    void __fastcall DrawTrkArrow (const TIMEPOS *pos);
    void __fastcall DrawTrkVel   (const TIMEPOS *vel);
    void __fastcall DrawSol      (int level, int type);
    void __fastcall DrawSolPnt   (const TIMEPOS *pos, int level, int style);
    void __fastcall DrawSolStat  (const TIMEPOS *pos, AnsiString unit, int p);
    void __fastcall DrawNsat     (int level);
    void __fastcall DrawRes      (int level, int frq);
    void __fastcall DrawPolyS    (TGraph *graph, double *x, double *y, int n,
                                  TColor color, int style);
    
    void __fastcall DrawObs      (int level);
    void __fastcall DrawObsSlip  (double *yp);
    void __fastcall DrawObsEphem (double *yp);
    void __fastcall DrawSky      (int level);
    void __fastcall DrawDop      (int level);
    void __fastcall DrawDopStat  (double *dop, int *ns, int n);
    
    TIMEPOS * __fastcall SolToPos (solbuf_t *sol, int index, int qflag, int type);
    TIMEPOS * __fastcall SolToNsat(solbuf_t *sol, int index, int qflag);
    
    void __fastcall PosToXyz     (gtime_t time, const double *rr, int type, double *xyz);
    void __fastcall CovToXyz     (const double *rr, const float *qr, int type,
                                  double *xyzs);
    void __fastcall CalcStats    (const double *x, int n, double ref, double &ave,
                                  double &std, double &rms);
    int __fastcall  FitPos       (gtime_t *time, double *opos, double *ovel);
    
    int  __fastcall ObsColor     (const obsd_t *obs, double az, double el);
    void __fastcall ReadStaPos   (const char *file, const char *sta, double *rr);
    int  __fastcall SearchPos    (int x, int y);
    void __fastcall TimeSpan     (gtime_t *ts, gtime_t *te, double *tint);
    double __fastcall TimePos    (gtime_t time);
    AnsiString __fastcall TimeStr(gtime_t time, int n, int tsys);
    void __fastcall ShowMsg      (AnsiString msg);
    int  __fastcall ExecCmd      (AnsiString cmd);
    void __fastcall ShowLegend   (AnsiString *msgs);
    void __fastcall LoadOpt      (void);
    void __fastcall SaveOpt      (void);
    
    BEGIN_MESSAGE_MAP // for files drop
    MESSAGE_HANDLER(WM_DROPFILES,TWMDropFiles,DropFiles);
    END_MESSAGE_MAP(TForm);
public:
    // connection settings
    int RtStream;
    AnsiString RtPath;
    AnsiString StrPaths[3];
    AnsiString StrCmds [2];
    int StrCmdEna[2];
    int RtFormat;
    int RtConnType;
    int RtTimeForm;
    int RtDegForm;
    AnsiString RtFieldSep;
    int RtTimeOutTime;
    int RtReConnTime;
    
    // time options 
    int TimeEna[3];
    gtime_t TimeStart;
    gtime_t TimeEnd;
    double TimeInt;
    
    // plot options 
    int TimeLabel;
    int ShowStats;
    int ShowSlip;
    int ShowHalfC;
    int ShowEph;
    double ElMask;
    int ElMaskP;
    int HideLowSat;
    double MaxDop;
    int NavSys;
    AnsiString ExSats;
    int ShowErr;
    int ShowArrow;
    int ShowGLabel;
    int ShowLabel;
    int ShowCompass;
    int ShowScale;
    int AutoScale;
    double YRange;
    int RtBuffSize;
    int Origin;
    double OOPos[3];
    TColor MColor[8]; // {mark1-7}
    TColor CColor[4]; // {line,text,grid,backgnd}
    int PlotStyle;
    int MarkSize;
    int AnimCycle;
    int RefCycle;
    int Trace;
    AnsiString QcCmd,QcOpt;
    
    AnsiString Title;
    AnsiString PntName[MAXMAPPNT];
    double PntPos[MAXMAPPNT][3];
    int NMapPnt;
    
    AnsiString StrHistory [10];
    AnsiString StrMntpHist[10];
    
    __fastcall TPlot(TComponent* Owner);
    int __fastcall GetCurrentPos(double *rr);
};
//---------------------------------------------------------------------------
extern PACKAGE TPlot *Plot;
//---------------------------------------------------------------------------
#endif
