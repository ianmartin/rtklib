//---------------------------------------------------------------------------
// rtkplot : visualization of solution and obs data ap
//
//          Copyright (C) 2007-2010 by T.TAKASU, All rights reserved.
//
// options : rtkplot [-r][-p port][-t level][file ...]
//
//           file     solution files or rinex obs and nav file
//           -r       open file as obs and nav file
//           -p path  connect to path
//                      serial://port[:brate[:bsize[:parity[:stopb[:fctr]]]]]
//                      tcpsvr://:port
//                      tcpcli://addr[:port]
//                      ntrip://[user[:passwd]@]addr[:port][/mntpnt]
//                      file://path
//           -t title   window title
//           -x level debug trace level (0:off)
//
// version : $Revision: 1.1 $ $Date: 2008/07/17 22:15:27 $
// history : 2008/07/14  1.0 new
//           2009/11/27  1.1 rtklib 2.3.0
//           2010/07/18  1.2 rtklib 2.4.0
//---------------------------------------------------------------------------
#include <vcl.h>
#include <vcl\inifiles.hpp>
#include <vcl\Clipbrd.hpp>
#pragma hdrstop
#pragma package(smart_init)
#pragma resource "*.dfm"

#include "rtklib.h"
#include "plotmain.h"
#include "plotopt.h"
#include "graph.h"
#include "refdlg.h"
#include "tspandlg.h"
#include "satdlg.h"
#include "aboutdlg.h"
#include "conndlg.h"
#include "confdlg.h"
#include "console.h"
#include "pntdlg.h"
#include "viewer.h"

//---------------------------------------------------------------------------
TPlot *Plot;

//---------------------------------------------------------------------------

#define SQR(x)      ((x)*(x))
#define SQRT(x)     ((x)<0.0?0.0:sqrt(x))
#define MAX(x,y)    ((x)>(y)?(x):(y))
#define MIN(x,y)    ((x)<(y)?(x):(y))

#define PRGNAME     "RTKPLOT"           // program name

#define CHARDEG     "\260"              // character code of degree
#define CHARUP2     "\262"              // character code of ^2

#define DEFTSPAN    600.0               // default time span (s)
#define INTARROW    60.0                // direction arrow interval (s)
#define MAXTDIFF    60.0                // max differential time (s)
#define DOPLIM      30.0                // dop view limit
#define TTOL        DTTOL               // time-differnce tolerance (s)
#define TBRK        300.0               // time to recognize break (s)
#define THRESLIP    0.1                 // slip threshold of LG-jump (m)
#define SIZE_COMP   45                  // compass size (pixels)
#define SIZE_VELC   45                  // velocity circle size (pixels)

#define CLORANGE    (TColor)0x00AAFF

#define PLOT_TRK    0
#define PLOT_SOLP   1
#define PLOT_SOLV   2
#define PLOT_SOLA   3
#define PLOT_NSAT   4
#define PLOT_OBS    5
#define PLOT_SKY    6
#define PLOT_DOP    7
#define PLOT_RES1   8
#define PLOT_RES2   9
#define PLOT_RES3   10
#define PLOT_RES4   11

#define ORG_STARTPOS 0
#define ORG_ENDPOS  1
#define ORG_AVEPOS  2
#define ORG_FITPOS  3
#define ORG_REFPOS  4
#define ORG_LLHPOS  5
#define ORG_AUTOPOS 6
#define ORG_PNTPOS  7

#define TRACEFILE   "rtkplot.trace"     // trace file
#define QCTMPFILE   "rtkplot_qc.temp"   // tempolary file for qc
#define QCERRFILE   "rtkplot_qc.err"    // error file for qc

static const char *PTypes[]={
    "Gnd Trk","Position","Velocity","Accel","NSat/Age/Ratio","Raw Obs",
    "Skyplot","Sats/DOP","L1 Residuals","L2 Residuals","L5 Residuals",
    "L7 Residuals","L6 Residuals",""
};
//---------------------------------------------------------------------------
extern "C" {
int showmsg(char *format,...) {return 0;}
}
//---------------------------------------------------------------------------
// class : TIMEPOS
//---------------------------------------------------------------------------
TIMEPOS::TIMEPOS(int nmax, int sflg)
{
    nmax_=nmax;
    n=0;
    t=new gtime_t[nmax];
    x=new double [nmax];
    y=new double [nmax];
    z=new double [nmax];
    if (sflg) {
        xs =new double [nmax];
        ys =new double [nmax];
        zs =new double [nmax];
        xys=new double [nmax];
    }
    else xs=ys=zs=xys=NULL;
    q=new int [nmax];
}
//---------------------------------------------------------------------------
TIMEPOS::~TIMEPOS()
{
    delete [] t;
    delete [] x;
    delete [] y;
    delete [] z;
    if (xs) {
        delete [] xs;
        delete [] ys;
        delete [] zs;
        delete [] xys;
    }
    delete [] q;
}
//---------------------------------------------------------------------------
TIMEPOS * TIMEPOS::tdiff(void)
{
    TIMEPOS *pos=new TIMEPOS(n,1);
    double tt;
    int i;
    
    for (i=0;i<n-1;i++) {
        
        tt=timediff(t[i+1],t[i]);
        
        if (tt==0.0||fabs(tt)>MAXTDIFF) continue;
        
        pos->t[pos->n]=timeadd(t[i],tt/2.0);
        pos->x[pos->n]=(x[i+1]-x[i])/tt;
        pos->y[pos->n]=(y[i+1]-y[i])/tt;
        pos->z[pos->n]=(z[i+1]-z[i])/tt;
        if (xs) {
            pos->xs [pos->n]=SQR(xs [i+1])+SQR(xs [i]);
            pos->ys [pos->n]=SQR(ys [i+1])+SQR(ys [i]);
            pos->zs [pos->n]=SQR(zs [i+1])+SQR(zs [i]);
            pos->xys[pos->n]=SQR(xys[i+1])+SQR(xys[i]);
        }
        pos->q[pos->n]=MAX(q[i],q[i+1]);
        pos->n++;
    }
    return pos;
}
//---------------------------------------------------------------------------
TIMEPOS *TIMEPOS::diff(const TIMEPOS *pos2)
{
    TIMEPOS *pos1=this,*pos=new TIMEPOS(MIN(n,pos2->n),1);
    double tt;
    int i,j;
    
    for (i=0,j=0;i<pos1->n&&j<pos2->n;i++,j++) {
        
        tt=timediff(pos1->t[i],pos2->t[j]);
        
        if      (tt<-TTOL) {j--; continue;}
        else if (tt> TTOL) {i--; continue;}
        
        pos->t[pos->n]=pos1->t[i];
        pos->x[pos->n]=pos1->x[i]-pos2->x[i];
        pos->y[pos->n]=pos1->y[i]-pos2->y[i];
        pos->z[pos->n]=pos1->z[i]-pos2->z[i];
        if (pos->xs) {
            pos->xs [pos->n]=SQR(pos1->xs [i])+SQR(pos2->xs [i]);
            pos->ys [pos->n]=SQR(pos1->ys [i])+SQR(pos2->ys [i]);
            pos->zs [pos->n]=SQR(pos1->zs [i])+SQR(pos2->zs [i]);
            pos->xys[pos->n]=SQR(pos1->xys[i])+SQR(pos2->xys[i]);
        }
        pos->q[pos->n]=MAX(pos1->q[i],pos2->q[j]);
        pos->n++;
    }
    return pos;
}
//---------------------------------------------------------------------------
// class : TPlot event handlers
//---------------------------------------------------------------------------
__fastcall TPlot::TPlot(TComponent* Owner) : TForm(Owner)
{
    gtime_t t0={0};
    nav_t nav0={0};
    obs_t obs0={0};
    solstatbuf_t solstat0={0};
    AnsiString s;
    double ep[]={2000,1,1,0,0,0},xl[2],yl[2];
    double xs[]={-DEFTSPAN/2,DEFTSPAN/2};
    int i,freq[]={1,2,5,7,6};
    
    Drag=0; Xn=Yn=-1; NObs=0;
    IndexObs=NULL;
    Week=Flush=PlotType=0;
    AnimCycle=1;
    for (i=0;i<2;i++) {
        initsolbuf(SolData+i,0,0);
        SolStat[i]=solstat0;
    }
    Obs=obs0;
    Nav=nav0;
    
    X0=Y0=Xc=Yc=Xs=Ys=Xcent=0.0;
    OEpoch=t0;
    for (i=0;i<3;i++) OPos[i]=OVel[i]=0.0;
    Az=El=NULL;
    SolFiles[0]=new TStringList;
    SolFiles[1]=new TStringList;
    ObsFiles   =new TStringList;
    NavFiles   =new TStringList;
    Buff   =new Graphics::TBitmap;
    GraphT =new TGraph(Disp);
    GraphT->Fit=0; GraphT->XLPos=2; GraphT->YLPos=4;
    
    for (i=0;i<3;i++) {
        GraphG[i]=new TGraph(Disp);
        GraphG[i]->XLPos=0;
        GraphG[i]->GetLim(xl,yl);
        GraphG[i]->SetLim(xs,yl);
    }
    GraphR=new TGraph(Disp);
    GraphS=new TGraph(Disp);
    GraphR->GetLim(xl,yl);
    GraphR->SetLim(xs,yl);
    
    for (i=0;i<3;i++) TimeEna[i]=0;
    TimeLabel=AutoScale=ShowStats=0;
    ShowLabel=ShowGLabel=1;
    ShowArrow=ShowSlip=ShowHalfC=ShowErr=ShowEph=0;
    PlotStyle=MarkSize=Origin=0;
    TimeInt=ElMask=YRange=0.0;
    MaxDop=30.0;
    TimeStart=TimeEnd=epoch2time(ep);
    DoubleBuffered=true;
    
    ObsType->Items->Clear();
    ObsType->Items->Add("ALL");
    for (i=0;i<NFREQ;i++) ObsType->Items->Add(s.sprintf("L%d",freq[i]));
    for (i=0;i<NFREQ;i++) ObsType->Items->Add(s.sprintf("P%d",freq[i]));
    ObsType->ItemIndex=0;
    for (i=0;i<361;i++) ElMaskData[i]=0.0;
    
    Trace=0;
    ConnectState=0;
    RtConnType=0;
    strinitcom();
    strinit(&Stream);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::FormCreate(TObject *Sender)
{
    DragAcceptFiles(Handle,true);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::FormShow(TObject *Sender)
{
    TStringList *files=new TStringList;
    AnsiString s;
    int i,argc=0,raw=0;
    char *p,*argv[32],buff[1024],*path="";
    
    trace(3,"FormShow\n");
    
    LoadOpt();
    
    strcpy(buff,GetCommandLine());
    for (p=buff;*p&&argc<32;p++) {
        if (*p==' ') continue;
        if (*p=='"') {
            argv[argc++]=p+1;
            if (!(p=strchr(p+1,'"'))) break;
        }
        else {
            argv[argc++]=p;
            if (!(p=strchr(p+1,' '))) break;
        }
        *p='\0';
    }
    for (i=1;i<argc;i++) {
        if      (!strcmp(argv[i],"-r")) raw=1;
        else if (!strcmp(argv[i],"-p")&&i+1<argc) path=argv[++i];
        else if (!strcmp(argv[i],"-t")&&i+1<argc) Title=argv[++i];
        else if (!strcmp(argv[i],"-x")&&i+1<argc) Trace=atoi(argv[++i]);
        else files->Add(argv[i]);
    }
    UpdateColor();
    UpdateSatMask();
    UpdateOrigin();
    
    if (*path) {
        ConnectPath(path);
    }
    else if (files->Count>0) {
        if (CheckObs(files->Strings[0])||raw) ReadObs(files);
        else ReadSol(files,0);
    }
    else {
        Caption=Title!=""?Title:s.sprintf("%s ver.%s",PRGNAME,VER_RTKLIB);
        UpdatePlot();
    }
    delete files;
    
    if (Trace>0) {
        traceopen(TRACEFILE);
        tracelevel(Trace);
    }
    Timer->Interval=RefCycle;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::FormResize(TObject *Sender)
{
    UpdateSize();
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DropFiles(TWMDropFiles msg)
{
    TStringList *files=new TStringList;
    int i,n;
    char buff[1024];
    
    if (ConnectState) return;
    
    if ((n=DragQueryFile((HDROP)msg.Drop,0xFFFFFFFF,NULL,0))<=0) {
        return;
    }
    for (i=0;i<n;i++) {
        DragQueryFile((HDROP)msg.Drop,i,buff,sizeof(buff));
        files->Add(buff);
    }
    if (CheckObs(files->Strings[0])) {
        ReadObs(files);
    }
    else {
        ReadSol(files,0);
    }
    delete files;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuOpenSol1Click(TObject *Sender)
{
    if (!OpenSolDialog->Execute()) return;
    ReadSol(OpenSolDialog->Files,0);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuOpenSol2Click(TObject *Sender)
{
    if (!OpenSolDialog->Execute()) return;
    ReadSol(OpenSolDialog->Files,1);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuOpenObsClick(TObject *Sender)
{
    if (!OpenObsDialog->Execute()) return;
    ReadObs(OpenObsDialog->Files);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuOpenNavClick(TObject *Sender)
{
    if (!OpenObsDialog->Execute()) return;
    ReadNav(OpenObsDialog->Files);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuElevMaskClick(TObject *Sender)
{
    if (!OpenElMaskDialog->Execute()) return;
    ReadElMaskData(OpenElMaskDialog->FileName);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuConnectClick(TObject *Sender)
{
    Connect();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuDisconnectClick(TObject *Sender)
{
    Disconnect();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuPortClick(TObject *Sender)
{
    int i;
    
    ConnectDialog->Stream  =RtStream;
    ConnectDialog->Format  =RtFormat;
    ConnectDialog->TimeForm=RtTimeForm;
    ConnectDialog->DegForm =RtDegForm;
    ConnectDialog->FieldSep=RtFieldSep;
    ConnectDialog->TimeOutTime=RtTimeOutTime;
    ConnectDialog->ReConnTime =RtReConnTime;
    for (i=0;i< 3;i++) ConnectDialog->Paths [i]=StrPaths [i];
    for (i=0;i< 2;i++) ConnectDialog->Cmds  [i]=StrCmds  [i];
    for (i=0;i< 2;i++) ConnectDialog->CmdEna[i]=StrCmdEna[i];
    for (i=0;i<10;i++) ConnectDialog->TcpHistory [i]=StrHistory [i];
    for (i=0;i<10;i++) ConnectDialog->TcpMntpHist[i]=StrMntpHist[i];
    
    if (ConnectDialog->ShowModal()!=mrOk) return;
    
    RtStream  =ConnectDialog->Stream;
    RtFormat  =ConnectDialog->Format;
    RtTimeForm=ConnectDialog->TimeForm;
    RtDegForm =ConnectDialog->DegForm;
    RtFieldSep=ConnectDialog->FieldSep;
    RtTimeOutTime=ConnectDialog->TimeOutTime;
    RtReConnTime =ConnectDialog->ReConnTime;
    for (i=0;i< 3;i++) StrPaths [i]=ConnectDialog->Paths [i];
    for (i=0;i< 2;i++) StrCmds  [i]=ConnectDialog->Cmds  [i];
    for (i=0;i< 2;i++) StrCmdEna[i]=ConnectDialog->CmdEna[i];
    for (i=0;i<10;i++) StrHistory [i]=ConnectDialog->TcpHistory [i];
    for (i=0;i<10;i++) StrMntpHist[i]=ConnectDialog->TcpMntpHist[i];
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuReloadClick(TObject *Sender)
{
    Reload();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuClearClick(TObject *Sender)
{
    Clear();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuQuitClick(TObject *Sender)
{
    SaveOpt();
    Close();
    
    if (Trace>0) traceclose();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuTimeClick(TObject *Sender)
{
    sol_t *sols,*sole;
    int i;
    
    if (Obs.n>0) {
        TimeStart=Obs.data[0].time;
        TimeEnd  =Obs.data[Obs.n-1].time;
    }
    else if (BtnSol2->Down&&SolData[1].n>0) {
        sols=getsol(SolData+1,0);
        sole=getsol(SolData+1,SolData[1].n-1);
        TimeStart=sols->time;
        TimeEnd  =sole->time;
    }
    else if (SolData[0].n>0) {
        sols=getsol(SolData,0);
        sole=getsol(SolData,SolData[0].n-1);
        TimeStart=sols->time;
        TimeEnd  =sole->time;
    }
    for (i=0;i<3;i++) {
        SpanDialog->TimeEna[i]=TimeEna[i];
    }
    SpanDialog->TimeStart=TimeStart;
    SpanDialog->TimeEnd  =TimeEnd;
    SpanDialog->TimeInt  =TimeInt;
    SpanDialog->TimeVal[0]=!ConnectState;
    SpanDialog->TimeVal[1]=!ConnectState;
    
    if (SpanDialog->ShowModal()!=mrOk) return;
    
    if (TimeEna[0]!=SpanDialog->TimeEna[0]||
        TimeEna[1]!=SpanDialog->TimeEna[1]||
        TimeEna[2]!=SpanDialog->TimeEna[2]||
        timediff(TimeStart,SpanDialog->TimeStart)!=0.0||
        timediff(TimeEnd,SpanDialog->TimeEnd)!=0.0||
        TimeInt!=SpanDialog->TimeInt) {
        
        for (i=0;i<3;i++) TimeEna[i]=SpanDialog->TimeEna[i];
        
        TimeStart=SpanDialog->TimeStart;
        TimeEnd  =SpanDialog->TimeEnd;
        TimeInt  =SpanDialog->TimeInt;
        
        Reload();
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenyCopyClick(TObject *Sender)
{
    Clipboard()->Assign(Buff);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuSrcSolClick(TObject *Sender)
{
    TTextViewer *viewer=new TTextViewer(Application);
    int sel=!BtnSol1->Down&&BtnSol2->Down;
    if (SolFiles[sel]->Count<=0) return;
    viewer->Caption=SolFiles[sel]->Strings[0];
    viewer->Option=0;
    viewer->Show();
    viewer->Read(SolFiles[sel]->Strings[0]);
}
//---------------------------------------------------------------------------

void __fastcall TPlot::MenuSrcObsClick(TObject *Sender)
{
    TTextViewer *viewer;
    char tmpfile[1024];
    int cstat;
    if (ObsFiles->Count<=0) return;
    cstat=uncompress(ObsFiles->Strings[0].c_str(),tmpfile);
    viewer=new TTextViewer(Application);
    viewer->Caption=ObsFiles->Strings[0];
    viewer->Option=0;
    viewer->Show();
    viewer->Read(!cstat?ObsFiles->Strings[0].c_str():tmpfile);
    if (cstat) remove(tmpfile);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuQcObsClick(TObject *Sender)
{
    TTextViewer *viewer;
    AnsiString cmd=QcCmd,cmdexec,tmpfile=QCTMPFILE,errfile=QCERRFILE;
    int i,stat;
    
    if (ObsFiles->Count<=0||cmd=="") return;
    
    for (i=0;i<ObsFiles->Count;i++) cmd+=" \""+ObsFiles->Strings[i]+"\"";
    for (i=0;i<NavFiles->Count;i++) cmd+=" \""+NavFiles->Strings[i]+"\"";
    
    cmdexec=cmd+" > "+tmpfile;
    cmdexec+=" 2> "+errfile;
    stat=execcmd(cmdexec.c_str());
    
    viewer=new TTextViewer(Application);
    viewer->Option=0;
    viewer->Show();
    viewer->Read(stat?errfile:tmpfile);
    viewer->Caption=(stat?"QC Error: ":"")+cmd;
    remove(tmpfile.c_str());
    remove(errfile.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuPointClick(TObject *Sender)
{
    if (PntDialog->ShowModal()!=mrOk) return;
    if (PlotType==PLOT_TRK) Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuOptionsClick(TObject *Sender)
{
    PlotOptDialog->Left=Left+Width/2-PlotOptDialog->Width/2;
    PlotOptDialog->Top=Top+Height/2-PlotOptDialog->Height/2;
    PlotOptDialog->Plot=this;
    
    if (PlotOptDialog->ShowModal()!=mrOk) return;
    
    SaveOpt();
    UpdateColor();
    UpdateSize();
    UpdateOrigin();
    UpdateInfo();
    UpdateSatMask();
    Refresh();
    Timer->Interval=RefCycle;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuToolBarClick(TObject *Sender)
{
    MenuToolBar->Checked=!MenuToolBar->Checked;
    Panel1->Visible=MenuToolBar->Checked;
    UpdateSize();
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuStatusBarClick(TObject *Sender)
{
    MenuStatusBar->Checked=!MenuStatusBar->Checked;
    Panel2->Visible=MenuStatusBar->Checked;
    UpdateSize();
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuMonitorClick(TObject *Sender)
{
    Console->Caption="Monitor RT Input";
    Console->Show();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuCenterOriClick(TObject *Sender)
{
    SetRange(0);
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuFitHorizClick(TObject *Sender)
{
    if (PlotType==PLOT_TRK) FitRange(0); else FitTime();
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuFitVertClick(TObject *Sender)
{
    FitRange(0);
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuShowTrackClick(TObject *Sender)
{
    BtnShowTrack->Down=!BtnShowTrack->Down;
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuFixHorizClick(TObject *Sender)
{
    BtnFixHoriz->Down=!BtnFixHoriz->Down;
    Xcent=0.0;
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuFixVertClick(TObject *Sender)
{
    BtnFixVert->Down=!BtnFixVert->Down;
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuShowPointClick(TObject *Sender)
{
    BtnShowPoint->Down=!BtnShowPoint->Down;
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuAnimStartClick(TObject *Sender)
{
    BtnAnimate->Down=true;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuAnimStopClick(TObject *Sender)
{
    BtnAnimate->Down=false;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MenuAboutClick(TObject *Sender)
{
    AboutDialog->About=PRGNAME;
    AboutDialog->IconIndex=2;
    AboutDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnConnectClick(TObject *Sender)
{
    if (!ConnectState) MenuConnectClick(Sender);
    else MenuDisconnectClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnSol1Click(TObject *Sender)
{
    BtnSol12->Down=false;
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnSol2Click(TObject *Sender)
{
    BtnSol12->Down=false;
    UpdatePlot(); 
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnSol12Click(TObject *Sender)
{
    BtnSol1->Down=false;
    BtnSol2->Down=false;
    UpdatePlot(); 
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnSol1DblClick(TObject *Sender)
{
    MenuOpenSol1Click(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnSol2DblClick(TObject *Sender)
{
    MenuOpenSol2Click(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnOn1Click(TObject *Sender)
{
    UpdateSize();
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnOn2Click(TObject *Sender)
{
    UpdateSize();
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnOn3Click(TObject *Sender)
{
    UpdateSize();
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnCenterOriClick(TObject *Sender)
{
    MenuCenterOriClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnFitHorizClick(TObject *Sender)
{
	MenuFitHorizClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnFitVertClick(TObject *Sender)
{
    MenuFitVertClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnShowTrackClick(TObject *Sender)
{
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnFixHorizClick(TObject *Sender)
{
    Xcent=0.0;
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnFixVertClick(TObject *Sender)
{
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnShowPointClick(TObject *Sender)
{
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnAnimateClick(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnClearClick(TObject *Sender)
{
    MenuClearClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnReloadClick(TObject *Sender)
{
    MenuReloadClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::PlotTypeSChange(TObject *Sender)
{
    int i;
    for (i=0;*PTypes[i];i++) {
        if (PlotTypeS->Text==PTypes[i]) UpdateType(i);
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::QFlagChange(TObject *Sender)
{
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::ObsTypeChange(TObject *Sender)
{
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DopTypeChange(TObject *Sender)
{
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::SatListChange(TObject *Sender)
{
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::SolScrollChange(TObject *Sender)
{
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DispPaint(TObject *Sender)
{
    UpdateDisp();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DispMouseDown(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
    X0=X; Y0=Y; Xcent0=Xcent;
    
    Drag=Shift.Contains(ssLeft)?1:(Shift.Contains(ssRight)?11:0);
    
    if (PlotType==PLOT_TRK) {
        MouseDownTrk(X,Y);
    }
    else if (PlotType<=PLOT_NSAT||PlotType>=PLOT_RES1) {
        MouseDownSol(X,Y);
    }
    else if (PlotType==PLOT_OBS||PlotType==PLOT_DOP) {
        MouseDownObs(X,Y);
    }
    else Drag=0;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DispMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
    double x,y,xs,ys,dx,dy,dxs,dys;
    
    if (X==Xn&&Y==Yn) return;
    
    Xn=X; Yn=Y;
    dx=(X0-X)*Xs;
    dy=(Y-Y0)*Ys;
    dxs=pow(2.0,(X0-X)/100.0);
    dys=pow(2.0,(Y-Y0)/100.0);
    
    if (Drag==0) {
        UpdatePoint(X,Y);
    }
    else if (PlotType==PLOT_TRK) {
        MouseMoveTrk(X,Y,dx,dy,dxs,dys);
    }
    else if (PlotType<=PLOT_NSAT||PlotType>=PLOT_RES1) {
        MouseMoveSol(X,Y,dx,dy,dxs,dys);
    }
    else if (PlotType==PLOT_OBS||PlotType==PLOT_DOP) {
        MouseMoveObs(X,Y,dx,dy,dxs,dys);
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DispMouseUp(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
    Drag=0;
    Screen->Cursor=crDefault;
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DispMouseLeave(TObject *Sender)
{
    Xn=Yn=-1;
    Panel22->Visible=false;
    Message2->Caption="";
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MouseDownTrk(int X, int Y)
{
    int i;
    
    trace(3,"MouseDownTrk:X=%d Y=%d\n",X,Y);
    
    if (Drag==1&&(i=SearchPos(X,Y))>=0) {
        SolScroll->Position=i;
        UpdateInfo();
        Drag=0;
        Refresh();
    }
    else {
        GraphT->GetCent(Xc,Yc);
        GraphT->GetScale(Xs,Ys);
        Screen->Cursor=Drag==1?crSizeAll:crVSplit;
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MouseDownSol(int X, int Y)
{
    TSpeedButton *btn[]={BtnOn1,BtnOn2,BtnOn3};
    TPoint pnt,p(X,Y);
    sol_t *data;
    double x,xl[2],yl[2];
    int i,area=-1,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"MouseDownSol:X=%d Y=%d\n",X,Y);
    
    data=getsol(SolData+sel,SolScroll->Position);
    
    if (data&&!BtnFixHoriz->Down) {
        
        x=TimePos(data->time);
        
        GraphG[0]->GetLim(xl,yl);
        GraphG[0]->ToPoint(x,yl[1],pnt);
        
        if ((X-pnt.x)*(X-pnt.x)+(Y-pnt.y)*(Y-pnt.y)<25) {
            Screen->Cursor=crSizeWE;
            Drag=20;
            Refresh();
            return;
        }
    }
    for (i=0;i<3;i++) {
        if (!btn[i]->Down) continue;
        
        GraphG[i]->GetCent(Xc,Yc);
        GraphG[i]->GetScale(Xs,Ys);
        area=GraphG[i]->OnAxis(p);
        
        if (Drag==1&&area==0) {
            Screen->Cursor=crSizeAll;
            Drag+=i;
            return;
        }
        else if (area==1) {
            Screen->Cursor=Drag==1?crSizeNS:crVSplit;
            Drag+=i+4;
            return;
        }
        else if (area==0) break;
    }
    if (area==0||area==8) {
        Screen->Cursor=Drag==1?crSizeWE:crHSplit;
        Drag+=3;
    }
    else Drag=0;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MouseDownObs(int X, int Y)
{
    TPoint pnt,p(X,Y);
    double x,xl[2],yl[2];
    int area,ind=ObsScroll->Position;
    
    trace(3,"MouseDownObs:X=%d Y=%d\n",X,Y);
    
    if (0<=ind&&ind<NObs&&!BtnFixHoriz->Down) {
        
        x=TimePos(Obs.data[IndexObs[ind]].time);
        
        GraphR->GetLim(xl,yl);
        GraphR->ToPoint(x,yl[1],pnt);
        
        if ((X-pnt.x)*(X-pnt.x)+(Y-pnt.y)*(Y-pnt.y)<25) {
            Screen->Cursor=crSizeWE;
            Drag=20;
            Refresh();
            return;
        }
    }
    GraphR->GetCent(Xc,Yc);
    GraphR->GetScale(Xs,Ys);
    area=GraphR->OnAxis(p);
    
    if (area==0||area==8) {
        Screen->Cursor=Drag==1?crSizeWE:crHSplit;
        Drag+=3;
    }
    else Drag=0;
}

//---------------------------------------------------------------------------
void __fastcall TPlot::MouseMoveTrk(int X, int Y, double dx, double dy,
    double dxs, double dys)
{
    trace(4,"MouseMoveTrk: X=%d Y=%d\n",X,Y);
    
    if (Drag==1&&!BtnFixHoriz->Down) {
        GraphT->SetCent(Xc+dx,Yc+dy);
    }
    else if (Drag>1) {
        GraphT->SetScale(Xs*dys,Ys*dys);
    }
    BtnCenterOri->Down=false;
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::SetCentX(double c)
{
    double x,y;
    int i;
    GraphR->GetCent(x,y);
    GraphR->SetCent(c,y);
    for (i=0;i<3;i++) {
        GraphG[i]->GetCent(x,y);
        GraphG[i]->SetCent(c,y);
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::SetScaleX(double s)
{
    double xs,ys;
    int i;
    GraphR->GetScale(xs,ys);
    GraphR->SetScale(s ,ys);
    for (i=0;i<3;i++) {
        GraphG[i]->GetScale(xs,ys);
        GraphG[i]->SetScale(s, ys);
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MouseMoveSol(int X, int Y, double dx, double dy,
    double dxs, double dys)
{
    TPoint p1,p2,p(X,Y);
    double x,y,xs,ys;
    int i,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(4,"MouseMoveSol: X=%d Y=%d\n",X,Y);
    
    if (Drag<=4) {
        for (i=0;i<3;i++) {
            GraphG[i]->GetCent(x,y);
            if (!BtnFixHoriz->Down) {
                x=Xc+dx;
            }
            if (!BtnFixVert->Down||!BtnFixVert->Enabled) {
                y=i==Drag-1?Yc+dy:y;
            }
            GraphG[i]->SetCent(x,y);
            SetCentX(x);
        }
        if (BtnFixHoriz->Down) {
            GraphR->GetPos(p1,p2);
            Xcent=Xcent0+2.0*(X-X0)/(p2.x-p1.x);
            if (Xcent> 1.0) Xcent= 1.0;
            if (Xcent<-1.0) Xcent=-1.0;
        }
    }
    else if (Drag<=7) {
        GraphG[Drag-5]->GetCent(x,y);
        if (!BtnFixVert->Down||!BtnFixVert->Enabled) {
            y=Yc+dy;
        }
        GraphG[Drag-5]->SetCent(x,y);
    }
    else if (Drag<=14) {
        for (i=0;i<3;i++) {
            GraphG[i]->GetScale(xs,ys);
            GraphG[i]->SetScale(Xs*dxs,ys);
        }
        SetScaleX(Xs*dxs);
    }
    else if (Drag<=17) {
        GraphG[Drag-15]->GetScale(xs,ys);
        GraphG[Drag-15]->SetScale(xs,Ys*dys);
    }
    else if (Drag==20) {
        GraphG[0]->ToPos(p,x,y);
        for (i=0;i<SolData[sel].n;i++) {
            if (TimePos(SolData[sel].data[i].time)>=x) break;
        }
        SolScroll->Position=i;
    }
    BtnCenterOri->Down=false;
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MouseMoveObs(int X, int Y, double dx, double dy,
    double dxs, double dys)
{
    TPoint p1,p2,p(X,Y);
    double x,y,xs,ys;
    int i;
    
    trace(4,"MouseMoveObs: X=%d Y=%d\n",X,Y);
    
    if (Drag<=4) {
        GraphR->GetCent(x,y);
        if (!BtnFixHoriz->Down) x=Xc+dx;
        if (!BtnFixVert ->Down) y=Yc+dy;
        GraphR->SetCent(x,y);
        SetCentX(x);
        
        if (BtnFixHoriz->Down) {
            GraphR->GetPos(p1,p2);
            Xcent=Xcent0+2.0*(X-X0)/(p2.x-p1.x);
            if (Xcent> 1.0) Xcent= 1.0;
            if (Xcent<-1.0) Xcent=-1.0;
        }
    }
    else if (Drag<=14) {
        GraphR->GetScale(xs,ys);
        GraphR->SetScale(Xs*dxs,ys);
        SetScaleX(Xs*dxs);
    }
    else if (Drag==20) {
        GraphR->ToPos(p,x,y);
        for (i=0;i<NObs;i++) {
            if (TimePos(Obs.data[IndexObs[i]].time)>=x) break;
        }
        ObsScroll->Position=i;
    }
    BtnCenterOri->Down=false;
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::MouseWheel(TObject *Sender, TShiftState Shift,
      int WheelDelta, TPoint &MousePos, bool &Handled)
{
    TPoint p(Xn,Yn);
    double xs,ys,ds=pow(2.0,-WheelDelta/1200.0);
    int i,area=-1;
    
    Handled=true;
    
    if (Xn<0||Yn<0) return;
    
    if (PlotType==PLOT_TRK) {
        GraphT->GetScale(xs,ys);
        GraphT->SetScale(xs*ds,ys*ds);
    }
    else if (PlotType<=PLOT_NSAT||PlotType>=PLOT_RES1) {
        
        for (i=0;i<3;i++) {
            area=GraphG[i]->OnAxis(p);
            if (area==0||area==1||area==2) {
                GraphG[i]->GetScale(xs,ys);
                GraphG[i]->SetScale(xs,ys*ds);
            }
            else if (area==0) break;
        }
        if (area==8) {
            for (i=0;i<3;i++) {
                GraphG[i]->GetScale(xs,ys);
                GraphG[i]->SetScale(xs*ds,ys);
                SetScaleX(xs*ds);
            }
        }
    }
    else if (PlotType==PLOT_OBS||PlotType==PLOT_DOP) {
        area=GraphR->OnAxis(p);
        if (area==0||area==8) {
            GraphR->GetScale(xs,ys);
            GraphR->SetScale(xs*ds,ys);
            SetScaleX(xs*ds);
        }
    }
    else return;
    
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::TimerTimer(TObject *Sender)
{
    TColor color[]={clRed,clBtnFace,CLORANGE,clGreen,clLime};
    static unsigned char buff[16384];
    solopt_t opt=solopt_default;
    const gtime_t ts={0};
    double tint=TimeEna[2]?TimeInt:0.0;
    int i,n,inb,inr,cycle,nmsg=0,stat,istat;
    char msg[MAXSTRMSG]="",tstr[32];
    
    trace(4,"TimeTimer\n");
    
    if (!ConnectState) {
        StrStatus->Color=clBtnFace;
        ConnectMsg->Caption="";
    }
    if (ConnectState) {
        opt.posf =RtFormat;
        opt.times=RtTimeForm==0?0:RtTimeForm-1;
        opt.timef=RtTimeForm>=1;
        opt.degf =RtDegForm;
        strcpy(opt.sep,RtFieldSep.c_str());
        strsum(&Stream,&inb,&inr,NULL,NULL);
        stat=strstat(&Stream,msg);
        StrStatus->Color=color[stat<3?stat+1:3];
        if (!strcmp(msg,"localhost")) ConnectMsg->Caption="";
        else ConnectMsg->Caption=msg;
        
        while ((n=strread(&Stream,buff,sizeof(buff)))>0) {
            
            for (i=0;i<n;i++) {
                istat=inputsol(buff[i],ts,ts,tint,0,&opt,SolData);
                if (istat==0) continue;
                if (istat<0) { // disconnect received
                    Disconnect();
                    return;
                }
                if (Week==0&&SolData[0].n==1) { // first data
                    if (PlotType>PLOT_NSAT) {
                        UpdateType(PLOT_TRK);
                    }
                    time2gpst(SolData[0].time,&Week);
                    UpdateOrigin();
                    //FitTime();
                    SetRange(0);
                    UpdateTimeExt();
                }
                nmsg++;
            }
            Console->AddMsg(buff,n);
        }
        if (nmsg<=0) return;
        StrStatus->Color=color[4];
        SolScroll->Max=SolData[0].n-1;
        SolScroll->Position=SolData[0].n-1;
    }
    else if (BtnAnimate->Enabled&&BtnAnimate->Down) {
        cycle=AnimCycle<=0?1:AnimCycle;
        
        if (PlotType<=PLOT_NSAT||PlotType>=PLOT_RES1) {
            if (SolScroll->Position+cycle<=SolScroll->Max) {
                SolScroll->Position=SolScroll->Position+cycle;
            }
            else SolScroll->Position=SolScroll->Max;
            if (SolScroll->Position==SolScroll->Max) BtnAnimate->Down=false;
        }
        else {
            if (ObsScroll->Position+cycle<=ObsScroll->Max) {
                ObsScroll->Position=ObsScroll->Position+cycle;
            }
            else ObsScroll->Position=ObsScroll->Max;
            if (ObsScroll->Position==ObsScroll->Max) BtnAnimate->Down=false;
        }
    }
    else return;
    
    UpdatePlot();
}
//---------------------------------------------------------------------------
// class : TPlot data event handling
//---------------------------------------------------------------------------
void __fastcall TPlot::ReadSol(TStrings *files, int sel)
{
    solbuf_t sol={0};
    AnsiString s;
    gtime_t ts,te;
    double tint;
    int i,n=0;
    char *paths[MAXNFILE];
    
    trace(3,"ReadSol: sel=%d\n",sel);
    
    if (files->Count<=0) return;
    
    Screen->Cursor=crHourGlass;
    
    for (i=0;i<files->Count&&n<MAXNFILE;i++) {
        paths[n++]=files->Strings[i].c_str();
    }
    TimeSpan(&ts,&te,&tint);
    
    if (!readsolt(paths,n,ts,te,tint,0,&sol)) {
        Screen->Cursor=crDefault;
        ShowMsg(s.sprintf("no solution data : %s...",paths[0]));
        ShowLegend(NULL);
        return;
    }
    free(SolData[sel].data); SolData[sel]=sol;
    
    if (SolFiles[sel]!=files) {
        SolFiles[sel]->Assign(files);
    }
    Caption="";
    
    ReadSolStat(files,sel);
    
    for (i=0;i<2;i++) {
        if (SolFiles[i]->Count==0) continue;
        Caption=Caption+SolFiles[i]->Strings[0]+(SolFiles[i]->Count>1?"... ":" ");
    }
    BtnSol12->Down=False;
    if (sel==0) BtnSol1->Down=true;
    else        BtnSol2->Down=true;
    
    if (sel==0||SolData[0].n<=0) {
        time2gpst(SolData[sel].data[0].time,&Week);
        UpdateOrigin();
    }
    if (PlotType>PLOT_NSAT) {
        UpdateType(PLOT_TRK);
    }
    else {
        UpdatePlotType();
    }
    FitTime();
    if (AutoScale&&PlotType<=PLOT_SOLA) FitRange(1);
    else SetRange(1);
    UpdateTimeExt();
    UpdatePlot();
    Screen->Cursor=crDefault;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::ReadSolStat(TStrings *files, int sel)
{
    TComboBox *list=sel?SatList2:SatList1;
    gtime_t ts,te;
    double tint;
    int i,n=0,sats[MAXSAT]={0};
    char *paths[MAXNFILE],id[32];
    
    free(SolStat[sel].data);
    SolStat[sel].n=SolStat[sel].nmax=0;
    SolStat[sel].data=NULL;
    
    TimeSpan(&ts,&te,&tint);
    
    for (i=0;i<files->Count&&n<MAXNFILE;i++) {
        paths[n++]=files->Strings[i].c_str();
    }
    readsolstatt(paths,n,ts,te,tint,SolStat+sel);
    
    for (i=0;i<SolStat[sel].n;i++) {
        sats[SolStat[sel].data[i].sat-1]=1;
    }
    list->Items->Clear();
    list->Items->Add("ALL");
    for (i=0;i<MAXSAT;i++) {
        if (!sats[i]) continue;
        satno2id(i+1,id);
        list->Items->Add(id);
    }
    list->ItemIndex=0;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::ReadObs(TStrings *files)
{
    obs_t obs={0};
    nav_t nav={0};
    AnsiString s;
    int i,nobs;
    
    trace(3,"ReadObs\n");
    
    if (files->Count<=0) return;
    
    Screen->Cursor=crHourGlass;
    
    if ((nobs=ReadObsRnx(files,&obs,&nav))<=0) {
        Screen->Cursor=crDefault;
        ShowMsg(s.sprintf("no raw obs data: %s...",files->Strings[0].c_str()));
        ShowLegend(NULL);
        return;
    }
    free(Obs.data);
    free(Nav.eph );
    free(Nav.geph);
    free(Nav.seph);
    Obs=obs;
    Nav=nav;
    UpdateObs(nobs);
    
    if (ObsFiles!=files) {
        ObsFiles->Assign(files);
    }
    NavFiles->Clear();
    Caption=s.sprintf("%s%s",files->Strings[0].c_str(),files->Count>1?"...":"");
    
    BtnSol1->Down=true;
    time2gpst(Obs.data[0].time,&Week);
    
    if (PlotType<PLOT_OBS||PLOT_DOP<PlotType) {
        UpdateType(PLOT_OBS);
    }
    else {
        UpdatePlotType();
    }
    FitTime();
    UpdateTimeExt();
    UpdatePlot();
    Screen->Cursor=crDefault;
}
//---------------------------------------------------------------------------
int __fastcall TPlot::ReadObsRnx(TStrings *files, obs_t *obs, nav_t *nav)
{
    gtime_t ts,te;
    double tint;
    int i,n=0,m=0,nep,nobs=0;
    char *paths[MAXNFILE],*navs[MAXNFILE],*p;
    
    TimeSpan(&ts,&te,&tint);
    
    for (i=0;i<MAXNFILE;i++) navs[i]=new char [1024];
    
    for (i=0;i<files->Count&&n<MAXNFILE-2&&m<MAXNFILE-1;i++) {
        
        paths[n++]=files->Strings[i].c_str();
        
        if (!CheckObs(files->Strings[i])) continue;
        
        strcpy(navs[m],files->Strings[i].c_str());
        if (!(p=strrchr(navs[m],'.'))) continue;
        if      (!strcmp(p,".obs")) strcpy(p,".nav");
        else if (!strcmp(p,".OBS")) strcpy(p,".NAV");
        else if (!strcmp(p+3,"o" )) strcpy(p+3,"n" );
        else if (!strcmp(p+3,"d" )) strcpy(p+3,"n" );
        else if (!strcmp(p+3,"O" )) strcpy(p+3,"N" );
        else if (!strcmp(p+3,"D" )) strcpy(p+3,"N" );
        else continue;
        paths[n++]=navs[m++];
        
        strcpy(navs[m],files->Strings[i].c_str());
        if (!(p=strrchr(navs[m],'.'))) continue;
        if      (!strcmp(p,".obs")) strcpy(p,".gnav");
        else if (!strcmp(p,".OBS")) strcpy(p,".GNAV");
        else if (!strcmp(p+3,"o" )) strcpy(p+3,"g" );
        else if (!strcmp(p+3,"d" )) strcpy(p+3,"g" );
        else if (!strcmp(p+3,"O" )) strcpy(p+3,"G" );
        else if (!strcmp(p+3,"D" )) strcpy(p+3,"G" );
        else continue;
        paths[n++]=navs[m++];
        
        strcpy(navs[m],files->Strings[i].c_str());
        if (!(p=strrchr(navs[m],'.'))) continue;
        if      (!strcmp(p,".obs")) strcpy(p,".hnav");
        else if (!strcmp(p,".OBS")) strcpy(p,".HNAV");
        else if (!strcmp(p+3,"o" )) strcpy(p+3,"h" );
        else if (!strcmp(p+3,"d" )) strcpy(p+3,"h" );
        else if (!strcmp(p+3,"O" )) strcpy(p+3,"H" );
        else if (!strcmp(p+3,"D" )) strcpy(p+3,"H" );
        else continue;
        paths[n++]=navs[m++];
    }
    for (i=0;i<n;i++) {
        nep=readrnxt(paths[i],1,ts,te,tint,obs,nav,NULL);
        if (nep>nobs) nobs=nep;
    }
    for (i=0;i<MAXNFILE;i++) delete [] navs[i];
    
    if (obs->n<=0) {
        free(nav->eph );
        free(nav->geph);
        free(nav->seph);
    }
    return nobs;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::ReadNav(TStrings *files)
{
    AnsiString s;
    gtime_t ts,te;
    double tint;
    int i;
    
    trace(3,"ReadNav\n");
    
    if (files->Count<=0) return;
    
    Screen->Cursor=crHourGlass;
    
    TimeSpan(&ts,&te,&tint);
    
    free(Nav.eph ); Nav.eph =NULL; Nav.n =Nav.nmax =0;
    free(Nav.geph); Nav.geph=NULL; Nav.ng=Nav.ngmax=0;
    free(Nav.seph); Nav.seph=NULL; Nav.ns=Nav.nsmax=0;
    
    for (i=0;i<files->Count;i++) {
        readrnxt(files->Strings[i].c_str(),1,ts,te,tint,NULL,&Nav,NULL);
    }
    if (Nav.n<=0&&Nav.ng<=0&&Nav.ns<=0) {
        Screen->Cursor=crDefault;
        ShowLegend(NULL);
        ShowMsg(s.sprintf("no nav message: %s...",files->Strings[0].c_str()));
        return;
    }
    if (NavFiles!=files) {
        NavFiles->Assign(files);
    }
    UpdateObs(NObs);
    UpdatePlot();
    Screen->Cursor=crDefault;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::ReadElMaskData(AnsiString file)
{
    AnsiString s;
    FILE *fp;
    double az0=0.0,el0=0.0,az1,el1;
    int i,j;
    char buff[256];
    
    trace(3,"ReadElMaskData\n");
    
    for (i=0;i<=360;i++) ElMaskData[i]=0.0;
    
    if (!(fp=fopen(file.c_str(),"r"))) {
        ShowMsg(s.sprintf("no el mask data: %s...",file.c_str()));
        return;
    }
    while (fgets(buff,sizeof(buff),fp)) {
        
        if (buff[0]=='%'||sscanf(buff,"%lf %lf",&az1,&el1)<2) continue;
        
        if (az0<az1&&az1<=360.0&&0.0<=el1&&el1<=90.0) {
            
            for (j=(int)az0;j<=(int)az1;j++) ElMaskData[j]=el0*D2R;
        }
        az0=az1; el0=el1;
    }
    fclose(fp);
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::Connect(void)
{
    AnsiString s;
    char *cmd,*path,buff[MAXSTRPATH],*p;
    int mode=STR_MODE_R;
    
    trace(3,"Connect\n");
    
    if (ConnectState) return;
    
    if      (RtStream==STR_SERIAL  ) path=StrPaths[0].c_str();
    else if (RtStream==STR_FILE    ) path=StrPaths[2].c_str();
    else if (RtStream<=STR_NTRIPCLI) path=StrPaths[1].c_str();
    else return;
    
    if (RtStream==STR_FILE||!SolData[0].cyclic||SolData[0].nmax!=RtBuffSize+1) {
        Clear();
        initsolbuf(SolData,1,RtBuffSize+1);
    }
    if (RtStream==STR_SERIAL) mode|=STR_MODE_W;
    
    strcpy(buff,path);
    if ((p=strstr(buff,"::"))) *p='\0';
    if ((p=strstr(buff,"/:"))) *p='\0';
    if ((p=strstr(buff,"@"))) p++; else p=buff;
    
    if (!stropen(&Stream,RtStream,mode,path)) {
        ShowMsg(s.sprintf("connect error: %s",p));
        trace(1,"stream open error: type=%d path=%s\n",RtStream,path);
        return;
    }
    strsettimeout(&Stream,RtTimeOutTime,RtReConnTime);
    
    if (StrCmdEna[0]) {
        cmd=StrCmds[0].c_str();
        strwrite(&Stream,(unsigned char *)cmd,strlen(cmd));
    }
    ConnectState=1;
    
    if (Title!="") Caption=Title; else Caption=s.sprintf("CONNECT %s",p);
    BtnConnect->Down=true;
    BtnSol1   ->Down=true;
    BtnSol2   ->Down=false;
    BtnSol12  ->Down=false;
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::Disconnect(void)
{
    AnsiString s;
    char *cmd;
    
    trace(3,"Disconnect\n");
    
    if (!ConnectState) return;
    
    ConnectState=0;
    
    if (StrCmdEna[1]) {
        cmd=StrCmds[1].c_str();
        strwrite(&Stream,(unsigned char *)cmd,strlen(cmd));
    }
    strclose(&Stream);
    
    if (strstr(Caption.c_str(),"CONNECT")) {
        Caption=s.sprintf("DISCONNECT%s",Caption.c_str()+7);
    }
    UpdatePlot();
}
//---------------------------------------------------------------------------
int __fastcall TPlot::CheckObs(AnsiString file)
{
    char *p;
    
    trace(3,"CheckObs\n");
    
    if (!(p=strrchr(file.c_str(),'.'))) return 0;
    if (!strcmp(p,".z")||!strcmp(p,".gz")||!strcmp(p,".zip")||
        !strcmp(p,".Z")||!strcmp(p,".GZ")||!strcmp(p,".ZIP")) {
        return *(p-1)=='o'||*(p-1)=='O'||*(p-1)=='d'||*(p-1)=='D';
    }
    return !strcmp(p,".obs")||!strcmp(p,".OBS")||
           !strcmp(p+3,"o" )||!strcmp(p+3,"O" )||
           !strcmp(p+3,"d" )||!strcmp(p+3,"D" );
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateObs(int nobs)
{
    prcopt_t opt=prcopt_default;
    sol_t sol={0};
    double azel[MAXOBS*2]={0};
    int i,j,k;
    char msg[128];
    
    trace(3,"UpdateObs\n");
    
    delete [] IndexObs;
    delete [] Az;
    delete [] El;
    IndexObs=new int[nobs];
    Az=new double[Obs.n];
    El=new double[Obs.n];
    
    opt.err[0]=900.0;
    
    NObs=0;
    for (i=0;i<Obs.n;i=j) {
        
        for (j=i;j<Obs.n;j++) {
            if (timediff(Obs.data[j].time,Obs.data[i].time)>TTOL) break;
        }
        IndexObs[NObs++]=i;
        
        pntpos(Obs.data+i,j-i,&Nav,&opt,&sol,azel,NULL,msg);
        
        for (k=0;k<j-i;k++) {
            Az[i+k]=azel[  k*2];
            El[i+k]=azel[1+k*2];
            if (Az[i+k]<0.0) Az[i+k]+=2.0*PI;
        }
    }
    IndexObs[NObs]=Obs.n;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::ConnectPath(const char *path)
{
    int str;
    const char *p;
    
    if (!(p=strstr(path,"://"))) return;
    if      (!strncmp(path,"serial",6)) str=STR_SERIAL;
    else if (!strncmp(path,"tcpsvr",6)) str=STR_TCPSVR;
    else if (!strncmp(path,"tcpcli",6)) str=STR_TCPCLI;
    else if (!strncmp(path,"ntrip", 5)) str=STR_NTRIPCLI;
    else if (!strncmp(path,"file",  4)) str=STR_FILE;
    else return;
    
    RtStream=str;
    StrPaths[1]=p+3;
    RtFormat=SOLF_LLH;
    RtTimeForm=0;
    RtDegForm =0;
    RtFieldSep=" ";
    RtTimeOutTime=0;
    RtReConnTime =10000;
    
    BtnShowTrack->Down=true;
    BtnFixHoriz ->Down=true;
    BtnFixVert  ->Down=true;
    
    Connect();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::Clear(void)
{
    AnsiString s;
    double ep[]={2010,1,1,0,0,0};
    int i;
    
    trace(3,"Clear\n");
    
    Week=NObs=0;
    
    for (i=0;i<2;i++) {
        freesolbuf(SolData+i);
        initsolbuf(SolData+i,0,0);
        free(SolStat[i].data);
        SolStat[i].n=0;
        SolStat[i].data=NULL;
    }
    free(Obs.data);
    free(Nav.eph );
    free(Nav.geph);
    Obs.data=NULL;
    Nav.eph =NULL;
    Nav.geph=NULL;
    Obs.n=0;
    Nav.n=0;
    Nav.ng=0;
    delete [] IndexObs;
    delete [] Az;
    delete [] El;
    IndexObs=NULL;
    Az=NULL;
    El=NULL;
    SolFiles[0]->Clear();
    SolFiles[1]->Clear();
    ObsFiles->Clear();
    NavFiles->Clear();
    
    for (i=0;i<3;i++) {
        TimeEna[i]=0;
    }
    TimeStart=TimeEnd=epoch2time(ep);
    BtnAnimate->Down=false;
    
    if (PlotType>PLOT_NSAT) {
        UpdateType(PLOT_TRK);
    }
    if (!ConnectState) {
        Caption=Title!=""?Title:s.sprintf("%s ver.%s",PRGNAME,VER_RTKLIB);
        UpdateOrigin();
        UpdateTimeExt();
        FitTime();
        SetRange(1);
    }
    else {
        initsolbuf(SolData,1,RtBuffSize+1);
    }
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::Refresh(void)
{
    trace(3,"Refresh\n");
    
    Flush=1;
    Disp->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::Reload(void)
{
    trace(3,"Reload\n");
    
    ReadObs(ObsFiles);
    ReadNav(NavFiles);
    ReadSol(SolFiles[0],0);
    ReadSol(SolFiles[1],1);
}
//---------------------------------------------------------------------------
// class : TPlot display event handling
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateDisp(void)
{
    TCanvas *c=Disp->Canvas;
    TRect r=Disp->ClientRect;
    int level=Drag?0:1;
    
    trace(3,"UpdateDisp\n");
    
    if (Flush) {
        c->Pen  ->Color=CColor[0];
        c->Brush->Color=CColor[0];
        c->Pen  ->Style=psSolid;
        c->Brush->Style=bsSolid;
        c->FillRect(r);
        
        switch (PlotType) {
            case  PLOT_TRK : DrawTrk (level);   break;
            case  PLOT_SOLP: DrawSol (level,0); break;
            case  PLOT_SOLV: DrawSol (level,1); break;
            case  PLOT_SOLA: DrawSol (level,2); break;
            case  PLOT_NSAT: DrawNsat(level);   break;
            case  PLOT_OBS : DrawObs (level);   break;
            case  PLOT_SKY : DrawSky (level);   break;
            case  PLOT_DOP : DrawDop (level);   break;
            case  PLOT_RES1:
            case  PLOT_RES2:
            case  PLOT_RES3:
                DrawRes(level,PlotType-PLOT_RES1+1);
                break;
        }
        Buff->SetSize(Disp->ClientWidth,Disp->ClientHeight);
        Buff->Canvas->CopyRect(r,c,r);
    }
    else {
        c->CopyRect(r,Buff->Canvas,r);
    }
    Flush=0;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateType(int type)
{
    sol_t *data;
    int i=0,sel,ind,obs1,obs2;
    
    trace(3,"UpdateType: type=%d\n",type);
    
    obs1=PLOT_OBS<=PlotType&&PlotType<=PLOT_DOP;
    obs2=PLOT_OBS<=type    &&type    <=PLOT_DOP;
    PlotType=type;
    if (AutoScale&&PlotType<=PLOT_SOLA) FitRange(0);
    else SetRange(0);
    
    sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    if (SolData[sel].n<=0||NObs<=0) {
        ;
    }
    else if (!obs1&&obs2) {
        ind=SolScroll->Position;
        if ((data=getsol(SolData+sel,ind))) {
            for (i=0;i<NObs;i++) {
                if (timediff(Obs.data[IndexObs[i]].time,data->time)>=-TTOL) break;
            }
        }
        ObsScroll->Position=i<NObs?i:NObs-1;
    }
    else if (obs1&&!obs2) {
        ind=ObsScroll->Position;
        if (0<=ind&&ind<NObs) {
            for (i=0;i<SolData[sel].n;i++) {
                data=getsol(SolData+sel,i);
                if (timediff(data->time,Obs.data[IndexObs[ind]].time)>=-TTOL) break;
            }
        }
        SolScroll->Position=i<SolData[sel].n?i:SolData[sel].n-1;
    }
    UpdatePlotType();
    UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdatePlot(void)
{
    trace(3,"UpdatePlot\n");
    
    UpdateEnable();
    UpdateInfo();
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateSize(void)
{
    TSpeedButton *btn[]={BtnOn1,BtnOn2,BtnOn3};
    TPoint p1(0,0),p2(Disp->Width,Disp->Height);
    double xs,ys;
    int i,n=0,h,tmargin,bmargin,rmargin,lmargin;
    
    trace(3,"UpdateSize\n");
    
    tmargin=5;
    bmargin=(int)(Disp->Font->Size*1.5)+3;
    rmargin=5;
    lmargin=Disp->Font->Size*3+15;
    
    GraphT->SetPos(p1,p2);
    GraphS->SetPos(p1,p2);
    GraphS->GetScale(xs,ys);
    xs=MAX(xs,ys);
    GraphS->SetScale(xs,xs);
    p1.x+=lmargin; p1.y+=tmargin;
    p2.x-=rmargin; p2.y=p2.y-bmargin;
    GraphR->SetPos(p1,p2);
    p2.y=p1.y;
    
    for (i=0;i<3;i++) if (btn[i]->Down) n++;
    for (i=0;i<3;i++) {
        if (!btn[i]->Down) continue;
        h=(Disp->Height-tmargin-bmargin)/n;
        p2.y+=h;
        GraphG[i]->SetPos(p1,p2);
        p1.y+=h;
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateColor(void)
{
    int i;
    
    trace(3,"UpdateColor\n");
    
    for (i=0;i<3;i++) {
        GraphT   ->Color[i]=CColor[i];
        GraphR   ->Color[i]=CColor[i];
        GraphS   ->Color[i]=CColor[i];
        GraphG[0]->Color[i]=CColor[i];
        GraphG[1]->Color[i]=CColor[i];
        GraphG[2]->Color[i]=CColor[i];
    }
    Disp->Font->Assign(Font);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateTimeExt(void)
{
    int sel;
    
    trace(3,"UpdateTimeExt:\n");
    
    sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    SolScroll->Max=SolData[sel].n<=0?1:SolData[sel].n-1;
    ObsScroll->Max=NObs<=0?100:NObs-1;
    SolScroll->Position=0;
    ObsScroll->Position=0;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::ReadStaPos(const char *file, const char *sta,
                                  double *rr)
{
    FILE *fp;
    char buff[256],code[256],name[256];
    double pos[3];
    int sinex=0;
    
    if (!(fp=fopen(file,"r"))) return;
    
    while (fgets(buff,sizeof(buff),fp)) {
        if (strstr(buff,"%=SNX")==buff) sinex=1;
        if (buff[0]=='%'||buff[1]=='#') continue;
        if (sinex) {
            if (strlen(buff)<68||strncmp(buff+14,sta,4)) continue;
            if (!strncmp(buff+7,"STAX",4)) rr[0]=str2num(buff,47,21);
            if (!strncmp(buff+7,"STAY",4)) rr[1]=str2num(buff,47,21);
            if (!strncmp(buff+7,"STAZ",4)) {
                rr[2]=str2num(buff,47,21);
                break;
            }
        }
        else {
            if (sscanf(buff,"%lf %lf %lf %s",pos,pos+1,pos+2,code)<4) continue;
            if (strcmp(code,sta)) continue;
            pos[0]*=D2R;
            pos[1]*=D2R;
            pos2ecef(pos,rr);
            break;
        }
    }
	fclose(fp);
}
//---------------------------------------------------------------------------
int __fastcall TPlot::FitPos(gtime_t *time, double *opos, double *ovel)
{
    sol_t *sol;
    int i,j;
    double t,x[2],Ay[3][2]={{0}},AA[3][4]={{0}};
    
    if (SolData[0].n<=0) return 0;
    
    for (i=0;sol=getsol(SolData,i);i++) {
        if (sol->type!=0) continue;
        if (time->time==0) *time=sol->time;
        t=timediff(sol->time,*time);
        
        for (j=0;j<3;j++) {
            Ay[j][0]+=sol->rr[j];
            Ay[j][1]+=sol->rr[j]*t;
            AA[j][0]+=1.0;
            AA[j][1]+=t;
            AA[j][2]+=t;
            AA[j][3]+=t*t;
        }
    }
    for (i=0;i<3;i++) {
        if (solve("N",AA[i],Ay[i],2,1,x)) return 0;
        opos[i]=x[0];
        ovel[i]=x[1];
    }
    return 1;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateOrigin(void)
{
    gtime_t time={0};
    sol_t *sol;
    double opos[3]={0},ovel[3]={0};
    int i,j,n=0,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    char *file,sta[16]="",*p;
    
    trace(3,"UpdateOrigin\n");
    
    if (Origin==ORG_STARTPOS) {
        if (!(sol=getsol(SolData,0))||sol->type!=0) return;
        for (i=0;i<3;i++) opos[i]=sol->rr[i];
    }
    else if (Origin==ORG_ENDPOS) {
        if (!(sol=getsol(SolData,SolData[0].n-1))||sol->type!=0) return;
        for (i=0;i<3;i++) opos[i]=sol->rr[i];
    }
    else if (Origin==ORG_AVEPOS) {
        for (i=0;sol=getsol(SolData,i);i++) {
            if (sol->type!=0) continue;
            for (j=0;j<3;j++) opos[j]+=sol->rr[j];
            n++;
        }
        if (n>0) for (i=0;i<3;i++) opos[i]/=n;
    }
    else if (Origin==ORG_FITPOS) {
        if (!FitPos(&time,opos,ovel)) return;
    }
    else if (Origin==ORG_REFPOS) {
        if (norm(SolData[0].rb,3)>0.0) {
            for (i=0;i<3;i++) opos[i]=SolData[0].rb[i];
        }
        else {
            if (!(sol=getsol(SolData,0))||sol->type!=0) return;
            for (i=0;i<3;i++) opos[i]=sol->rr[i];
        }
    }
    else if (Origin==ORG_LLHPOS) {
        pos2ecef(OOPos,opos);
    }
    else if (Origin==ORG_AUTOPOS) {
        if (SolFiles[sel]->Count>0) {
            file=SolFiles[sel]->Strings[0].c_str();
            if ((p=strrchr(file,'\\'))) strncpy(sta,p+1,4);
            else strncpy(sta,file,4);
            for (p=sta;*p;p++) *p=(char)toupper(*p);
            
            ReadStaPos(RefDialog->StaPosFile.c_str(),sta,opos);
        }
    }
    else if (Origin-ORG_PNTPOS<MAXMAPPNT) {
        for (i=0;i<3;i++) opos[i]=PntPos[Origin-ORG_PNTPOS][i];
    }
    OEpoch=time;
    for (i=0;i<3;i++) {
        OPos[i]=opos[i];
        OVel[i]=ovel[i];
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateSatMask(void)
{
    int sat,prn;
    char buff[256],*p;
    
    trace(3,"UpdateSatMask\n");
    
    for (sat=0;sat<MAXSAT;sat++) SatMask[sat-1]=0;
    for (sat=0;sat<MAXSAT;sat++) {
        if (!(satsys(sat,&prn)&NavSys)) SatMask[sat-1]=1;
    }
    if (ExSats!="") {
        strcpy(buff,ExSats.c_str());
        
        for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
            if ((sat=satid2no(p))) SatMask[sat-1]=1;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateInfo(void)
{
    trace(3,"UpdateInfo:\n");
    
    if (BtnShowTrack->Down) {
        if (PLOT_OBS<=PlotType&&PlotType<=PLOT_DOP) UpdateTimeObs();
        else UpdateTimeSol();
    }
    else {
        if (PLOT_OBS<=PlotType&&PlotType<=PLOT_DOP) UpdateInfoObs();
        else UpdateInfoSol();
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateTimeObs(void)
{
    AnsiString msgs1[]={" OBS=L1/2 "," L1 "," L2 "," P1/2 "," P1 ",""," P2 "};
    AnsiString msgs2[]={" SNR=>45 "," >40 "," >35 "," >30 "," >25 ",""," <25 "};
    AnsiString msg,msgs[8],s;
    double azel[MAXOBS*2],dop[4]={0};
    int i,ns=0,no=0,ind=ObsScroll->Position;
    
    trace(3,"UpdateTimeObs\n");
    
    if (BtnSol1->Down&&0<=ind&&ind<NObs) {
        
        for (i=IndexObs[ind];i<Obs.n&&i<IndexObs[ind+1];i++,no++) {
            if (SatMask[Obs.data[i].sat-1]) continue;
            if (El[i]<ElMask*D2R) continue;
            if (ElMaskP&&El[i]<ElMaskData[(int)(Az[i]*R2D+0.5)]) continue;
            azel[  ns*2]=Az[i];
            azel[1+ns*2]=El[i];
            ns++;
        }
    }
    if (ns>0) {
        dops(ns,azel,ElMask*D2R,dop);
        
        msg.sprintf("[1]%s : N=%d ",TimeStr(Obs.data[IndexObs[ind]].time,3,1).c_str(),no);
        
        if (PlotType==PLOT_DOP) {
            msgs[0].sprintf("NSAT=%d",ns);
            msgs[1].sprintf(" GDOP=%.1f",dop[0]);
            msgs[2].sprintf(" PDOP=%.1f",dop[1]);
            msgs[3].sprintf(" HDOP=%.1f",dop[2]);
            msgs[4].sprintf(" VDOP=%.1f",dop[3]);
        }
        else {
            msg+=s.sprintf("NSAT=%d ",ns);
            for (i=0;i<7;i++) msgs[i]=ObsType->ItemIndex==0?msgs1[i]:msgs2[i];
        }
    }
    ShowMsg(msg);
    ShowLegend(msgs);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateTimeSol(void)
{
    const char *unit[]={"m","m/s","m/s2"},*u;
    const char *sol[]={"","FIX","FLOAT","SBAS","DGPS","Single","PPP"};
    AnsiString msg,msgs[8],s;
    sol_t *data;
    double xyz[3],pos[3],r,az,el;
    int sel=BtnSol1->Down||!BtnSol2->Down?0:1,ind=SolScroll->Position;
    
    trace(3,"UpdateTimeSol\n");
    
    if ((BtnSol1->Down||BtnSol2->Down||BtnSol12->Down)&&
        (data=getsol(SolData+sel,ind))) {
        
        if (!ConnectState) msg.sprintf("[%d]",sel+1); else msg="[R]";
        
        msg+=TimeStr(data->time,2,1)+" : ";
        
        if (PLOT_SOLP<=PlotType&&PlotType<=PLOT_SOLA) {
            PosToXyz(data->time,data->rr,data->type,xyz);
            u=unit[PlotType-PLOT_SOLP];
            msg+=s.sprintf("E=%7.4f%s N=%7.4f%s U=%7.4f%s Q=",
                           xyz[0],u,xyz[1],u,xyz[2],u);
        }
        else if (PlotType==PLOT_NSAT) {
            msg+=s.sprintf("NS=%d AGE=%.1f RATIO=%.1f Q=",data->ns,data->age,
                           data->ratio);
        }
        else if (!data->type) {
            ecef2pos(data->rr,pos);
            msg+=s.sprintf("%13.9f" CHARDEG " %14.9f" CHARDEG " %9.4fm  Q=",
                           pos[0]*R2D,pos[1]*R2D,pos[2]);
        }
        else {
            r=norm(data->rr,3);
            az=norm(data->rr,2)<=1E-12?0.0:atan2(data->rr[0],data->rr[1])*R2D;
            el=r<=1E-12?0.0:asin(data->rr[2]/r)*R2D;
            msg+=s.sprintf("B=%.3fm D=%6.2f" CHARDEG " %5.2f" CHARDEG "  Q=",
                           r,az<0.0?az+360.0:az,el);
        }
        if (1<=data->stat&&data->stat<=6) {
            msgs[data->stat-1]=s.sprintf("%d:%s",data->stat,sol[data->stat]);
        }
    }
    ShowMsg(msg);
    ShowLegend(msgs);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateInfoSol(void)
{
    AnsiString msg,msgs[8],s,s1,s2;
    TIMEPOS *pos=NULL,*pos1,*pos2;
    sol_t *data;
    gtime_t ts={0},te={0};
    double r[3],b,bl[2]={1E6,0.0};
    int i,j,n=0,nq[8]={0},sel=BtnSol1->Down||!BtnSol2->Down?0:1;
    
    trace(3,"UpdateInfoSol:\n");
    
    if (BtnSol1->Down||BtnSol2->Down) {
        pos=SolToPos(SolData+sel,-1,0,0);
    }
    else if (BtnSol12->Down) {
        pos1=SolToPos(SolData  ,-1,0,0);
        pos2=SolToPos(SolData+1,-1,0,0);
        pos=pos1->diff(pos2);
        delete pos1;
        delete pos2;
    }
    if (pos) {
        for (i=0;i<pos->n;i++) {
            if (ts.time==0) ts=pos->t[i]; te=pos->t[i];
            nq[pos->q[i]]++;
            n++; 
        }
        delete pos;
    }
    for (i=0;data=getsol(SolData+sel,i);i++) {
        if (data->type) {
            b=norm(data->rr,3);
        }
        else if (norm(SolData[sel].rb,3)>0.0) {
            for (j=0;j<3;j++) r[j]=data->rr[j]-SolData[sel].rb[j];
            b=norm(r,3);
        }
        else b=0.0;
        if (b<bl[0]) bl[0]=b;
        if (b>bl[1]) bl[1]=b;
    }
    if (n>0) {
        if (!ConnectState) msg.sprintf("[%d]",sel+1); else msg="[R]";
        
        s1=TimeStr(ts,0,0);
        s2=TimeStr(te,0,1);
        msg+=s.sprintf("%s-%s : N=%d",s1.c_str(),s2.c_str()+(TimeLabel?5:0),n);
        
        if (bl[0]+100.0<bl[1]) {
            msg+=s.sprintf(" B=%.1f-%.1fkm",bl[0]/1E3,bl[1]/1E3);
        }
        else {
            msg+=s.sprintf(" B=%.1fkm",bl[0]/1E3);
        }
        msg+=" Q=";
        
        for (i=1;i<=6;i++) {
            if (nq[i]<=0) continue;
            msgs[i-1].sprintf("%d:%d(%.1f%%) ",i,nq[i],(double)nq[i]/n*100.0);
        }
    }
    ShowMsg(msg);
    ShowLegend(msgs);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateInfoObs(void)
{
    AnsiString msgs0[]={"  NSAT"," GDOP"," PDOP"," HDOP"," VDOP","",""};
    AnsiString msgs1[]={" OBS=L1/2 "," L1 "," L2 "," P1/2 "," P1 ",""," P2 "};
    AnsiString msgs2[]={" SNR=>45 "," >40 "," >35 "," >30 "," >25 ",""," <25 "};
    AnsiString msg,msgs[8],s1,s2;
    gtime_t ts={0},te={0},t,tp={0};
    int i,n=0,ne=0;
    
    trace(3,"UpdateInfoObs:\n");
    
    if (BtnSol1->Down) {
        for (i=0;i<Obs.n;i++) {
            t=Obs.data[i].time;
            if (ts.time==0) ts=t; te=t;
            if (tp.time==0||timediff(t,tp)>TTOL) ne++; 
            n++; tp=t; 
        }
    }
    if (n>0) {
        s1=TimeStr(ts,0,0);
        s2=TimeStr(te,0,1);
        msg.sprintf("[1]%s-%s : EP=%d N=%d",s1.c_str(),s2.c_str()+(TimeLabel?5:0),ne,n);
        
        for (i=0;i<7;i++) {
            if (PlotType==PLOT_DOP) {
                msgs[i]=msgs0[i];
            }
            else {
                msgs[i]=ObsType->ItemIndex==0?msgs1[i]:msgs2[i];
            }
        }
    }
    ShowMsg(msg);
    ShowLegend(msgs);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdatePoint(int x, int y)
{
    AnsiString msg;
    gtime_t time;
    TPoint p(x,y);
    double enu[3]={0},rr[3],pos[3],xx,yy,r,xl[2],yl[2],q[2],az,el;
    int i;
    char s[64];
    
    trace(4,"UpdatePoint: x=%d y=%d\n",x,y);
    
    if (PlotType==PLOT_TRK) {
        
        if (norm(OPos,3)>0.0) {
            GraphT->ToPos(p,enu[0],enu[1]);
            ecef2pos(OPos,pos);
            enu2ecef(pos,enu,rr);
            for (i=0;i<3;i++) rr[i]+=OPos[i];
            ecef2pos(rr,pos);
            msg.sprintf("%12.8f" CHARDEG " %13.8f" CHARDEG,pos[0]*R2D,pos[1]*R2D);
        }
    }
    else if (PlotType==PLOT_SKY) { // sky-plot
        
        GraphS->GetLim(xl,yl);
        GraphS->ToPos(p,q[0],q[1]);
        r=(xl[1]-xl[0]<yl[1]-yl[0]?xl[1]-xl[0]:yl[1]-yl[0])*0.45;
        
        if ((el=90.0-90.0*norm(q,2)/r)>0.0) {
            az=el>=90.0?0.0:atan2(q[0],q[1])*R2D;
            if (az<0.0) az+=360.0;
            msg.sprintf("AZ=%5.1f" CHARDEG " EL=%4.1f" CHARDEG,az,el);
        }
    }
    else {
        GraphG[0]->ToPos(p,xx,yy);
        time=gpst2time(Week,xx);
        if      (TimeLabel==2) time=utc2gpst(time); // UTC
        else if (TimeLabel==3) time=timeadd(gpst2utc(time),-9*3600.0); // JST
        msg=TimeStr(time,0,1);
    }
    Panel22->Visible=true;
    Message2->Caption=msg;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdateEnable(void)
{
    AnsiString s;
    int data=BtnSol1->Down||BtnSol2->Down||BtnSol12->Down;
    int type=PLOT_OBS<=PlotType&&PlotType<=PLOT_DOP;
    int plot=PLOT_SOLP<=PlotType&&PlotType<=PLOT_NSAT;
    int sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"UpdateEnable\n");
    
    Panel1         ->Visible=MenuToolBar  ->Checked;
    Panel2         ->Visible=MenuStatusBar->Checked;
    
    QFlag          ->Visible=PlotType<=PLOT_NSAT;
    ObsType        ->Visible=PLOT_OBS<=PlotType&&PlotType<=PLOT_SKY;
    DopType        ->Visible=PlotType==PLOT_DOP;
    SatList1       ->Visible=PlotType>=PLOT_RES1;
    SatList2       ->Visible=PlotType>=PLOT_RES1;
    
    BtnConnect     ->Down   = ConnectState;
    BtnSol1        ->Enabled=!ConnectState;
    BtnSol2        ->Enabled=!ConnectState&&!type;
    BtnSol12       ->Enabled=!ConnectState&&!type&&SolData[0].n>0&&SolData[1].n>0;
    BtnOn1         ->Enabled=plot||PlotType>=PLOT_RES1;
    BtnOn2         ->Enabled=plot||PlotType>=PLOT_RES1;
    BtnOn3         ->Enabled=plot||PlotType>=PLOT_RES1;
    BtnCenterOri   ->Enabled=data&&(plot||PlotType==PLOT_TRK);
    BtnFitHoriz    ->Enabled=data&&PlotType!=PLOT_SKY;
    BtnFitVert     ->Enabled=data&&PlotType<=PLOT_SOLA;
    BtnShowTrack   ->Enabled=data;
    BtnFixHoriz    ->Enabled=data&&(PlotType<=PLOT_NSAT||
                             PlotType==PLOT_OBS||PlotType==PLOT_DOP||PlotType>=PLOT_RES1);
    BtnFixVert     ->Enabled=data&&(plot&&PlotType!=PLOT_NSAT);
    BtnShowPoint   ->Enabled=PlotType==PLOT_TRK;
    BtnAnimate     ->Enabled=data&&BtnShowTrack->Down;
    
    if (!BtnShowTrack->Down) {
        BtnAnimate ->Down   =false;
        BtnFixHoriz->Enabled=false;
        BtnFixVert ->Enabled=false;
    }
    MenuSrcSol     ->Enabled=SolFiles[sel]->Count>0;
    MenuSrcObs     ->Enabled=ObsFiles->Count>0;
    MenuQcObs      ->Enabled=ObsFiles->Count>0;
    
    BtnShowTrack->Enabled;
    MenuShowTrack  ->Enabled=BtnShowTrack->Enabled;
    MenuFitHoriz   ->Enabled=BtnFitHoriz ->Enabled;
    MenuFitVert    ->Enabled=BtnFitVert  ->Enabled;
    MenuCenterOri  ->Enabled=BtnCenterOri->Enabled;
    MenuFixHoriz   ->Enabled=BtnFixHoriz ->Enabled;
    MenuFixVert    ->Enabled=BtnFixVert  ->Enabled;
    MenuShowPoint  ->Enabled=BtnShowPoint->Enabled;
    
    MenuShowTrack  ->Checked=BtnShowTrack->Down;
    MenuFixHoriz   ->Checked=BtnFixHoriz ->Down;
    MenuFixVert    ->Checked=BtnFixVert  ->Down;
    MenuShowPoint  ->Checked=BtnShowPoint->Down;
    
    MenuAnimStart  ->Enabled=!ConnectState&&BtnAnimate->Enabled&&!BtnAnimate->Down;
    MenuAnimStop   ->Enabled=!ConnectState&&BtnAnimate->Enabled&& BtnAnimate->Down;
    SolScroll      ->Enabled=data&&BtnShowTrack->Down;
    ObsScroll      ->Enabled=data&&BtnShowTrack->Down;
    
    MenuOpenSol1   ->Enabled=!ConnectState;
    MenuOpenSol2   ->Enabled=!ConnectState;
    MenuConnect    ->Enabled=!ConnectState;
    MenuDisconnect ->Enabled= ConnectState;
    MenuPort       ->Enabled=!ConnectState;
    MenuOpenObs    ->Enabled=!ConnectState;
    MenuOpenNav    ->Enabled=!ConnectState;
    MenuElevMask   ->Enabled=!ConnectState;
    MenuReload     ->Enabled=!ConnectState;
    
    BtnReload      ->Visible=!ConnectState;
    StrStatus      ->Visible= ConnectState;
    SolScroll      ->Visible=!type;
    ObsScroll      ->Visible=type;
    Panel12        ->Visible=!ConnectState;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::UpdatePlotType(void)
{
    int i;
    
    PlotTypeS->Clear();
    if (SolData[0].n>0||SolData[1].n>0||
        (NObs<=0&&SolStat[0].n<=0&&SolStat[1].n<=0)) {
        PlotTypeS->AddItem(PTypes[PLOT_TRK ],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_SOLP],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_SOLV],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_SOLA],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_NSAT],NULL);
    }
    if (NObs>0) {
        PlotTypeS->AddItem(PTypes[PLOT_OBS ],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_SKY ],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_DOP ],NULL);
    }
    if (SolStat[0].n>0||SolStat[1].n>0) {
        for (i=0;i<NFREQ;i++) {
            PlotTypeS->AddItem(PTypes[PLOT_RES1+i],NULL);
        }
    }
    for (i=0;i<PlotTypeS->Items->Count;i++) {
        if (PlotTypeS->Items->Strings[i]!=PTypes[PlotType]) continue;
        PlotTypeS->ItemIndex=i;
        return;
    }
    PlotTypeS->ItemIndex=0;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::FitTime(void)
{
    sol_t *sols,*sole;
    double tl[2]={86400.0*7,0.0},tp[2],xl[2],yl[2],zl[2];
    int sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"FitTime\n");
    
    sols=getsol(SolData+sel,0);
    sole=getsol(SolData+sel,SolData[sel].n-1);
    if (sols&&sole) {
        tl[0]=MIN(tl[0],TimePos(sols->time));
        tl[1]=MAX(tl[1],TimePos(sole->time));
    }
    if (Obs.n>0) {
        tl[0]=MIN(tl[0],TimePos(Obs.data[0].time));
        tl[1]=MAX(tl[1],TimePos(Obs.data[Obs.n-1].time));
    }
    if (tl[0]==tl[1]) {
        tl[0]=tl[0]-DEFTSPAN/2.0;
        tl[1]=tl[0]+DEFTSPAN/2.0;
    }
    else if (tl[0]>tl[1]) {
        tl[0]=-DEFTSPAN/2.0;
        tl[1]= DEFTSPAN/2.0;
    }
    GraphG[0]->GetLim(tp,xl);
    GraphG[1]->GetLim(tp,yl);
    GraphG[2]->GetLim(tp,zl);
    GraphG[0]->SetLim(tl,xl);
    GraphG[1]->SetLim(tl,yl);
    GraphG[2]->SetLim(tl,zl);
    GraphR   ->GetLim(tp,xl);
    GraphR   ->SetLim(tl,xl);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::SetRange(int all)
{
    double xl[]={-YRange,YRange};
    double yl[]={-YRange,YRange};
    double zl[]={-YRange,YRange};
    double xs,ys,tl[2],xp[2];
    
    trace(3,"SetRange\n");
    
    if (all||PlotType==PLOT_TRK) {
        GraphT->SetLim(xl,yl);
        GraphT->GetScale(xs,ys);
        GraphT->SetScale(MAX(xs,ys),MAX(xs,ys));
    }
    if (all||(PLOT_SOLP<=PlotType&&PlotType<=PLOT_SOLA)) {
        GraphG[0]->GetLim(tl,xp);
        GraphG[0]->SetLim(tl,xl);
        GraphG[1]->SetLim(tl,yl);
        GraphG[2]->SetLim(tl,zl);
    }
    else if (PlotType==PLOT_NSAT) {
        GraphG[0]->GetLim(tl,xp);
        xl[0]=yl[0]=zl[0]=0.0;
        xl[1]=20.0;
        yl[1]=10.0;
        zl[1]=20.0;
        GraphG[0]->SetLim(tl,xl);
        GraphG[1]->SetLim(tl,yl);
        GraphG[2]->SetLim(tl,zl);
    }
    else if (PlotType>=PLOT_RES1) {
        GraphG[0]->GetLim(tl,xp);
        xl[0]=-10.0; xl[1]=10.0;
        yl[0]= -0.1; yl[1]= 0.1;
        zl[0]=  0.0; zl[1]=90.0;
        GraphG[0]->SetLim(tl,xl);
        GraphG[1]->SetLim(tl,yl);
        GraphG[2]->SetLim(tl,zl);
    }
}

//---------------------------------------------------------------------------
void __fastcall TPlot::FitRange(int all)
{
    TIMEPOS *pos,*pos1,*pos2;
    double tl[2],xl[]={1E8,-1E8},yl[2]={1E8,-1E8},zl[2]={1E8,-1E8};
    double xs,ys,xp[2];
    int i,type=PlotType-PLOT_SOLP;
    
    trace(3,"FitRange\n");
    
    if (BtnSol1->Down) {
        
        pos=SolToPos(SolData,-1,QFlag->ItemIndex,type);
        
        for (i=0;i<pos->n;i++) {
            xl[0]=MIN(xl[0],pos->x[i]);
            yl[0]=MIN(yl[0],pos->y[i]);
            zl[0]=MIN(zl[0],pos->z[i]);
            xl[1]=MAX(xl[1],pos->x[i]);
            yl[1]=MAX(yl[1],pos->y[i]);
            zl[1]=MAX(zl[1],pos->z[i]);
        }
        delete pos;
    }
    if (BtnSol2->Down) {
        
        pos=SolToPos(SolData+1,-1,QFlag->ItemIndex,type);
        
        for (i=0;i<pos->n;i++) {
            xl[0]=MIN(xl[0],pos->x[i]);
            yl[0]=MIN(yl[0],pos->y[i]);
            zl[0]=MIN(zl[0],pos->z[i]);
            xl[1]=MAX(xl[1],pos->x[i]);
            yl[1]=MAX(yl[1],pos->y[i]);
            zl[1]=MAX(zl[1],pos->z[i]);
        }
        delete pos;
    }
    if (BtnSol12->Down) {
        
        pos1=SolToPos(SolData  ,-1,QFlag->ItemIndex,type);
        pos2=SolToPos(SolData+1,-1,QFlag->ItemIndex,type);
        pos=pos1->diff(pos2);
        
        for (i=0;i<pos->n;i++) {
            xl[0]=MIN(xl[0],pos->x[i]);
            yl[0]=MIN(yl[0],pos->y[i]);
            zl[0]=MIN(zl[0],pos->z[i]);
            xl[1]=MAX(xl[1],pos->x[i]);
            yl[1]=MAX(yl[1],pos->y[i]);
            zl[1]=MAX(zl[1],pos->z[i]);
        }
        delete pos1;
        delete pos2;
        delete pos;
    }
    xl[0]-=0.05;
    xl[1]+=0.05;
    yl[0]-=0.05;
    yl[1]+=0.05;
    zl[0]-=0.05;
    zl[1]+=0.05;
    
    if (all||PlotType==PLOT_TRK) {
        GraphT->SetLim(xl,yl);
        GraphT->GetScale(xs,ys);
        GraphT->SetScale(MAX(xs,ys),MAX(xs,ys));
    }
    if (all||(PlotType<=PLOT_SOLA||PlotType>=PLOT_RES1)) {
        GraphG[0]->GetLim(tl,xp);
        GraphG[0]->SetLim(tl,xl);
        GraphG[1]->SetLim(tl,yl);
        GraphG[2]->SetLim(tl,zl);
    }
}
//---------------------------------------------------------------------------
int __fastcall TPlot::GetCurrentPos(double *rr)
{
    sol_t *data;
    int i,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"GetCurrentPos\n");
    
    if (PLOT_OBS<=PlotType&&PlotType<=PLOT_DOP) return 0;
    if (!(data=getsol(SolData+sel,SolScroll->Position))) return 0;
    if (data->type) return 0;
    for (i=0;i<3;i++) rr[i]=data->rr[i];
    return 1;
}
//---------------------------------------------------------------------------
// class : TPlot draw plots
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawTrk(int level)
{
    AnsiString label,header;
    TIMEPOS *pos,*pos1,*pos2,*vel;
    TPoint p1,p2;
    TColor color;
    double xt,yt,sx,sy,opos[3];
    int i,sel=!BtnSol1->Down&&BtnSol2->Down?1:0,p=0;
    
    trace(3,"DrawTrk: level=%d\n",level);
    
    if (BtnShowTrack->Down&&BtnFixHoriz->Down&&!BtnSol12->Down) {
        pos=SolToPos(SolData+sel,SolScroll->Position,0,0);
        if (pos->n>0) {
            GraphT->SetCent(pos->x[0],pos->y[0]);
        }
        delete pos;
    }
    GraphT->DrawAxis(ShowLabel,ShowGLabel);
    
    if (ShowCompass) {
        GraphT->GetPos(p1,p2);
        p1.x+=SIZE_COMP/2+25;
        p1.y+=SIZE_COMP/2+35;
        GraphT->DrawMark(p1,13,CColor[2],SIZE_COMP,0);
    }
    if (ShowArrow) {
        vel=SolToPos(SolData+sel,SolScroll->Position,0,1);
        DrawTrkVel(vel);
        delete vel;
    }
    if (ShowScale) {
        GraphT->GetPos(p1,p2);
        GraphT->GetTick(xt,yt);
        GraphT->GetScale(sx,sy);
        p2.x-=70;
        p2.y-=25;
        GraphT->DrawMark(p2,11,CColor[2],(int)(xt/sx+0.5),0);
        p2.y-=3;
        if      (xt<0.01  ) label.sprintf("%.0f mm",xt*1000.0);
        else if (xt<1.0   ) label.sprintf("%.0f cm",xt*100.0);
        else if (xt<1000.0) label.sprintf("%.0f m" ,xt);
        else                label.sprintf("%.0f km",xt/1000.0);
        GraphT->DrawText(p2,label,CColor[2],0,1,0);
    }
    if (norm(OPos,3)>0.0) {
        ecef2pos(OPos,opos);
        header.sprintf("ORI=%.9f" CHARDEG " %.9f" CHARDEG " %.4fm",opos[0]*R2D,
                       opos[1]*R2D,opos[2]);
    }
    if (BtnShowPoint->Down) {
        for (i=0;i<NMapPnt;i++) {
            DrawTrkPos(PntPos[i],0,PntName[i]);
        }
    }
    if (BtnSol1->Down) {
        pos=SolToPos(SolData,-1,QFlag->ItemIndex,0);
        DrawTrkPnt(pos,level,0);
        DrawTrkPos(SolData[0].rb,0,"Base Station 1");
        DrawTrkStat(pos,header,p++);
        delete pos;
    }
    if (BtnSol2->Down) {
        pos=SolToPos(SolData+1,-1,QFlag->ItemIndex,0);
        DrawTrkPnt(pos,level,1);
        DrawTrkPos(SolData[1].rb,0,"Base Station 2");
        DrawTrkStat(pos,header,p++);
        delete pos;
    }
    if (BtnSol12->Down) {
        pos1=SolToPos(SolData  ,-1,QFlag->ItemIndex,0);
        pos2=SolToPos(SolData+1,-1,QFlag->ItemIndex,0);
        pos=pos1->diff(pos2);
        DrawTrkPnt(pos,level,0);
        DrawTrkStat(pos,"",p++);
        delete pos;
        delete pos1;
        delete pos2;
    }
    if (!level) { // center +
        GraphT->GetPos(p1,p2);
        p1.x=(p1.x+p2.x)/2;
        p1.y=(p1.y+p2.y)/2;
        GraphT->DrawMark(p1,5,CColor[1],20,0);
    }
    if (BtnShowTrack->Down&&((BtnSol1->Down&&!sel)||(BtnSol2->Down&&sel))) {
        
        pos=SolToPos(SolData+sel,SolScroll->Position,0,0);
        
        if (pos->n>0) {
            pos->n=1;
            DrawTrkError(pos,0);
            GraphT->DrawMark(pos->x[0],pos->y[0],0,CColor[0],MarkSize*2+10,0);
            GraphT->DrawMark(pos->x[0],pos->y[0],1,CColor[2],MarkSize*2+10,0);
            GraphT->DrawMark(pos->x[0],pos->y[0],5,CColor[2],MarkSize*2+14,0);
            GraphT->DrawMark(pos->x[0],pos->y[0],0,CColor[2],MarkSize*2+6,0);
            GraphT->DrawMark(pos->x[0],pos->y[0],0,MColor[pos->q[0]],MarkSize*2+4,0);
        }
        delete pos;
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawTrkPnt(const TIMEPOS *pos, int level, int style)
{
    int i;
    
    trace(3,"DrawTrkPnt: level=%d style=%d\n",level,style);
    
    if (level) DrawTrkArrow(pos);
    
    if (level&&PlotStyle<=1&&!BtnShowTrack->Down) { // error circle
        DrawTrkError(pos,style);
    }
    if (!level||!(PlotStyle%2)) {
        GraphT->DrawPoly(pos->x,pos->y,pos->n,CColor[3],style);
    }
    if (level&&PlotStyle<2) {
        for (i=0;i<pos->n;i++) {
            GraphT->DrawMark(pos->x[i],pos->y[i],style,MColor[pos->q[i]],MarkSize,0);
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawTrkPos(const double *rr, int type, AnsiString label)
{
    gtime_t time;
    double xyz[3],xs,ys;
    
    trace(3,"DrawTrkPos\n");
    
    if (norm(rr,3)>0.0) {
        GraphT->GetScale(xs,ys);
        PosToXyz(time,rr,type,xyz);
        GraphT->DrawMark(xyz[0],xyz[1],5,CColor[1],14,0);
        GraphT->DrawMark(xyz[0],xyz[1],0,CColor[1],6,0);
        xyz[1]-=4*xs;
        GraphT->DrawText(xyz[0],xyz[1],label,CColor[2],0,2,0);
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawTrkStat(const TIMEPOS *pos, AnsiString header, int p)
{
    AnsiString s[6];
    TPoint p1,p2;
    double *d,ave[4],std[4],rms[4],opos[3];
    int i,n=0,fonth=(int)(Disp->Font->Size*1.5);
    
    trace(3,"DrawTrkStat\n");
    
    if (!ShowStats) return;
    
    if (p==0&&header!="") s[n++]=header;
    
    if (pos->n>0) {
        d=new double[pos->n];
        for (i=0;i<pos->n;i++) {
            d[i]=SQRT(SQR(pos->x[i])+SQR(pos->y[i]));
        }
        CalcStats(pos->x,pos->n,0.0,ave[0],std[0],rms[0]);
        CalcStats(pos->y,pos->n,0.0,ave[1],std[1],rms[1]);
        CalcStats(pos->z,pos->n,0.0,ave[2],std[2],rms[2]);
        CalcStats(d     ,pos->n,0.0,ave[3],std[3],rms[3]);
        s[n++].sprintf("AVE=E:%7.4fm N:%7.4fm U:%7.4fm",ave[0],ave[1],ave[2]);
        s[n++].sprintf("STD=E:%7.4fm N:%7.4fm U:%7.4fm",std[0],std[1],std[2]);
        s[n++].sprintf("RMS=E:%7.4fm N:%7.4fm U:%7.4fm 2D:%7.4fm",
                       rms[0],rms[1],rms[2],2.0*rms[3]);
        delete [] d;
    }
    GraphT->GetPos(p1,p2);
    p1.x=p2.x-10;
    p1.y+=8+fonth*4*p;
    for (i=0;i<n;i++,p1.y+=fonth) {
        GraphT->DrawText(p1,s[i],CColor[2],2,2,0);
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawTrkError(const TIMEPOS *pos, int style)
{
    const double sint[36]={
         0.0000, 0.1736, 0.3420, 0.5000, 0.6428, 0.7660, 0.8660, 0.9397, 0.9848,
         1.0000, 0.9848, 0.9397, 0.8660, 0.7660, 0.6428, 0.5000, 0.3420, 0.1736,
         0.0000,-0.1736,-0.3420,-0.5000,-0.6428,-0.7660,-0.8660,-0.9397,-0.9848,
        -1.0000,-0.9848,-0.9397,-0.8660,-0.7660,-0.6428,-0.5000,-0.3420,-0.1736
    };
    double xc[37],yc[37],a,b,s,c;
    int i,j;
    
    trace(3,"DrawTrkError\n");
    
    if (!ShowErr) return;
    
    for (i=0;i<pos->n;i++) {
        if (pos->xs[i]<=0.0||pos->ys[i]<=0.0) continue;
        
        a=pos->xys[i]/SQRT(pos->xs[i]);
        
        if ((b=pos->ys[i]-a*a)>=0.0) b=SQRT(b); else continue;
        
        for (j=0;j<37;j++) {
            s=sint[j%36];
            c=sint[(45-j)%36];
            xc[j]=pos->x[i]+SQRT(pos->xs[i])*c;
            yc[j]=pos->y[i]+a*c+b*s;
        }
        GraphT->DrawPoly(xc,yc,37,CColor[3],ShowErr==1?0:1);
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawTrkArrow(const TIMEPOS *pos)
{
    TPoint p;
    double tt,d[2],dist,dt,vel;
    int i,off=8;
    
    trace(3,"DrawTrkArrow\n");
    
    if (!ShowArrow) return;
    
    for (i=1;i<pos->n-1;i++) {
        tt=time2gpst(pos->t[i],NULL);
        d[0]=pos->x[i+1]-pos->x[i-1];
        d[1]=pos->y[i+1]-pos->y[i-1];
        dist=norm(d,2);
        dt=timediff(pos->t[i+1],pos->t[i-1]);
        vel=dt==0.0?0.0:dist/dt;
        
        if (vel<0.5||fmod(tt+0.005,INTARROW)>=0.01) continue;
        
        GraphT->ToPoint(pos->x[i],pos->y[i],p);
        p.x-=(int)(off*d[1]/dist);
        p.y-=(int)(off*d[0]/dist);
        GraphT->DrawMark(p,10,CColor[3],15,(int)(atan2(d[1],d[0])*R2D));
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawTrkVel(const TIMEPOS *vel)
{
    AnsiString label;
    TPoint p1,p2;
    double v=0.0,dir=0.0;
    
    trace(3,"DrawTrkVel\n");
    
    if (vel&&vel->n>0) {
        if ((v=sqrt(SQR(vel->x[0])+SQR(vel->y[0])))>1.0) {
            dir=atan2(vel->x[0],vel->y[0])*R2D;
        }
    }
    GraphT->GetPos(p1,p2);
    p1.x+=SIZE_VELC/2+30;
    p1.y=p2.y-SIZE_VELC/2-30;
    GraphT->DrawMark(p1,1,CColor[1],SIZE_VELC,0);
    p1.y+=SIZE_VELC/2;
    label.sprintf("%.0f km/h",v*3600.0/1000.0);
    GraphT->DrawText(p1,label,CColor[2],0,2,0);
    p1.y-=SIZE_VELC/2;
    if (v>=1.0) GraphT->DrawMark(p1,10,CColor[2],SIZE_VELC,90-(int)dir);
    GraphT->DrawMark(p1,0,CColor[0],10,0);
    GraphT->DrawMark(p1,1,v>=1.0?CColor[2]:CColor[1],10,0);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawSol(int level, int type)
{
    AnsiString label[]={"E-W","N-S","U-D"},unit[]={"m","m/s","m/s" CHARUP2};
    TSpeedButton *btn[]={BtnOn1,BtnOn2,BtnOn3};
    TIMEPOS *pos,*pos1,*pos2;
    TPoint p1,p2;
    double xc,yc,xl[2],yl[2],off,y;
    int i,j,k,sel=!BtnSol1->Down&&BtnSol2->Down?1:0,p=0;
    
    trace(3,"DrawSol: level=%d\n",level);
    
    if (BtnShowTrack->Down&&(BtnFixHoriz->Down||BtnFixVert->Down)) {
        
        pos=SolToPos(SolData+sel,SolScroll->Position,0,type);
        
        for (i=0;i<3&&pos->n>0;i++) {
            GraphG[i]->GetCent(xc,yc);
            if (BtnFixVert->Down) {
                yc=i==0?pos->x[0]:(i==1?pos->y[0]:pos->z[0]);
            }
            if (BtnFixHoriz->Down) {
                GraphG[i]->GetLim(xl,yl);
                off=Xcent*(xl[1]-xl[0])/2.0;
                GraphG[i]->SetCent(TimePos(pos->t[0])-off,yc);
            }
            else {
                GraphG[i]->SetCent(xc,yc);
            }
        }
        delete pos;
    }
    j=-1;
    for (i=0;i<3;i++) if (btn[i]->Down) j=i;
    for (i=0;i<3;i++) {
        if (!btn[i]->Down) continue;
        GraphG[i]->XLPos=TimeLabel?(i==j?6:5):(i==j?1:0);
        GraphG[i]->Week=Week;
        GraphG[i]->DrawAxis(ShowLabel,ShowLabel);
        GraphG[i]->GetPos(p1,p2);
        p1.x+=5;
        p1.y+=3;
        GraphG[i]->DrawText(p1,label[i]+" ("+unit[type]+")",CColor[2],1,2,0);
    }
    if (BtnSol1->Down) {
        pos=SolToPos(SolData,-1,QFlag->ItemIndex,type);
        DrawSolPnt(pos,level,0);
        DrawSolStat(pos,unit[type],p++);
        delete pos;
    }
    if (BtnSol2->Down) {
        pos=SolToPos(SolData+1,-1,QFlag->ItemIndex,type);
        DrawSolPnt(pos,level,1);
        DrawSolStat(pos,unit[type],p++);
        delete pos;
    }
    if (BtnSol12->Down) {
        pos1=SolToPos(SolData  ,-1,QFlag->ItemIndex,type);
        pos2=SolToPos(SolData+1,-1,QFlag->ItemIndex,type);
        pos=pos1->diff(pos2);
        DrawSolPnt(pos,level,0);
        DrawSolStat(pos,unit[type],p++);
        delete pos;
        delete pos1;
        delete pos2;
    }
    if (BtnShowTrack->Down&&(BtnSol1->Down||BtnSol2->Down||BtnSol12->Down)) {
        
        pos=SolToPos(SolData+sel,SolScroll->Position,0,type);
        
        for (j=k=0;j<3&&pos->n>0;j++) {
            
            if (!btn[j]->Down) continue;
            
            GraphG[j]->GetLim(xl,yl);
            xl[0]=xl[1]=TimePos(pos->t[0]);
            y=j==0?pos->x[0]:(j==1?pos->y[0]:pos->z[0]);
            GraphG[j]->DrawPoly(xl,yl,2,CColor[2],0);
            if (!BtnSol12->Down) {
                GraphG[j]->DrawMark(xl[0],y,0,CColor[0],MarkSize*2+6,0);
                GraphG[j]->DrawMark(xl[0],y,1,CColor[2],MarkSize*2+6,0);
                GraphG[j]->DrawMark(xl[0],y,1,CColor[2],MarkSize*2+2,0);
                GraphG[j]->DrawMark(xl[0],y,0,MColor[pos->q[0]],MarkSize*2,0);
            }
            if (k++==0) {
                GraphG[j]->DrawMark(xl[0],yl[1]-1E-6,0,CColor[2],5,0);
                
                if (!BtnFixHoriz->Down) {
                    GraphG[j]->DrawMark(xl[0],yl[1]-1E-6,1,CColor[2],9,0);
                }
            }
        }
        delete pos;
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawSolPnt(const TIMEPOS *pos, int level, int style)
{
    TSpeedButton *btn[]={BtnOn1,BtnOn2,BtnOn3};
    double *x,*y,*s,xs,ys,*yy;
    int i,j;
    
    trace(3,"DrawSolPnt\n");
    
    x=new double [pos->n];
    
    for (i=0;i<pos->n;i++) {
        x[i]=TimePos(pos->t[i]);
    }
    for (i=0;i<3;i++) {
        if (!btn[i]->Down) continue;
        
        y=i==0?pos->x :(i==1?pos->y :pos->z );
        s=i==0?pos->xs:(i==1?pos->ys:pos->zs);
        
        if (!level||!(PlotStyle%2)) {
            DrawPolyS(GraphG[i],x,y,pos->n,CColor[3],style);
        }
        if (level&&ShowErr&&PlotType<=PLOT_SOLA&&PlotStyle<2) {
            
            GraphG[i]->GetScale(xs,ys);
            
            if (ShowErr==1) {
                for (j=0;j<pos->n;j++) {
                    GraphG[i]->DrawMark(x[j],y[j],12,CColor[3],(int)(SQRT(s[j])*2.0/ys),0);
                }
            }
            else {
                yy=new double [pos->n];
                
                for (j=0;j<pos->n;j++) yy[j]=y[j]-SQRT(s[j]);
                DrawPolyS(GraphG[i],x,yy,pos->n,CColor[3],1);
                
                for (j=0;j<pos->n;j++) yy[j]=y[j]+SQRT(s[j]);
                DrawPolyS(GraphG[i],x,yy,pos->n,CColor[3],1);
                
                delete [] yy;
            }
        }
        if (level&&PlotStyle<2) {
            for (j=0;j<pos->n;j++) {
                TColor color=MColor[pos->q[j]];
                GraphG[i]->DrawMark(x[j],y[j],style,color,MarkSize,0);
            }
        }
    }
    delete [] x;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawSolStat(const TIMEPOS *pos, AnsiString unit, int p)
{
    TSpeedButton *btn[]={BtnOn1,BtnOn2,BtnOn3};
    TPoint p1,p2;
    double ave,std,rms,*y,opos[3];
    int i,j=0,k=0,fonth=(int)(Disp->Font->Size*1.5);
    char *u;
    AnsiString s;
    
    trace(3,"DrawSolStat\n");
    
    if (!ShowStats||pos->n<=0) return;
    
    for (i=0;i<3;i++) {
        if (!btn[i]->Down) continue;
        
        y=i==0?pos->x:(i==1?pos->y:pos->z);
        CalcStats(y,pos->n,0.0,ave,std,rms);
        GraphG[i]->GetPos(p1,p2);
        p1.x=p2.x-5;
        p1.y+=3+fonth*(p+(!k++&&p>0?1:0));
        
        if (j==0&&p==0) {
            
            if (norm(OPos,3)>0.0) {
                ecef2pos(OPos,opos);
                s.sprintf("ORI=%.9f" CHARDEG " %.9f" CHARDEG " %.4fm",
                          opos[0]*R2D,opos[1]*R2D,opos[2]);
                GraphG[i]->DrawText(p1,s,CColor[2],2,2,0);
                j++; p1.y+=fonth;
            }
        }
        u=unit.c_str();
        s.sprintf("AVE=%.4f%s STD=%.4f%s RMS=%.4f%s",ave,u,std,u,rms,u);
        GraphG[i]->DrawText(p1,s,CColor[2],2,2,0);
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawNsat(int level)
{
    AnsiString label[]={
        "# of Valid Satellites",
        "Age of Differential (s)",
        "Ratio Factor for AR Validation"
    };
    TSpeedButton *btn[]={BtnOn1,BtnOn2,BtnOn3};
    TIMEPOS *ns;
    TPoint p1,p2;
    double xc,yc,y,xl[2],yl[2],off;
    int i,j,k,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"DrawNsat: level=%d\n",level);
    
    if (BtnShowTrack->Down&&BtnFixHoriz->Down) {
        
        ns=SolToNsat(SolData+sel,SolScroll->Position,0);
        
        for (i=0;i<3;i++) {
            if (BtnFixHoriz->Down) {
                GraphG[i]->GetLim(xl,yl);
                off=Xcent*(xl[1]-xl[0])/2.0;
                GraphG[i]->GetCent(xc,yc);
                GraphG[i]->SetCent(TimePos(ns->t[0])-off,yc);
            }
            else {
                GraphG[i]->GetRight(xc,yc);
                GraphG[i]->SetRight(TimePos(ns->t[0]),yc);
            }
        }
        delete ns;
    }
    j=-1;
    for (i=0;i<3;i++) if (btn[i]->Down) j=i;
    for (i=0;i<3;i++) {
        if (!btn[i]->Down) continue;
        GraphG[i]->XLPos=TimeLabel?(i==j?6:5):(i==j?1:0);
        GraphG[i]->Week=Week;
        GraphG[i]->DrawAxis(ShowLabel,ShowLabel);
        GraphG[i]->GetPos(p1,p2);
        p1.x+=5;
        p1.y+=3;
        GraphG[i]->DrawText(p1,label[i],CColor[2],1,2,0);
    }
    if (BtnSol1->Down) {
        ns=SolToNsat(SolData,-1,QFlag->ItemIndex);
        DrawSolPnt(ns,level,0);
        delete ns;
    }
    if (BtnSol2->Down) {
        ns=SolToNsat(SolData+1,-1,QFlag->ItemIndex);
        DrawSolPnt(ns,level,1);
        delete ns;
    }
    if (BtnShowTrack->Down&&(BtnSol1->Down||BtnSol2->Down)) {
        
        ns=SolToNsat(SolData+sel,SolScroll->Position,0);
        
        for (j=k=0;j<3&&ns->n>0;j++) {
            
            if (!btn[j]->Down) continue;
            
            y=j==0?ns->x[0]:(j==1?ns->y[0]:ns->z[0]);
            GraphG[j]->GetLim(xl,yl);
            xl[0]=xl[1]=TimePos(ns->t[0]);
            
            GraphG[j]->DrawPoly(xl,yl,2,CColor[2],0);
            GraphG[j]->DrawMark(xl[0],y,0,CColor[0],MarkSize*2+6,0);
            GraphG[j]->DrawMark(xl[0],y,1,CColor[2],MarkSize*2+6,0);
            GraphG[j]->DrawMark(xl[0],y,1,CColor[2],MarkSize*2+2,0);
            GraphG[j]->DrawMark(xl[0],y,0,MColor[ns->q[0]],MarkSize*2,0);
            
            if (k++==0) {
                GraphG[j]->DrawMark(xl[0],yl[1]-1E-6,0,CColor[2],5,0);
                
                if (!BtnFixHoriz->Down) {
                    GraphG[j]->DrawMark(xl[0],yl[1]-1E-6,1,CColor[2],9,0);
                }
            }
        }
        delete ns;
    }
}

//---------------------------------------------------------------------------
void __fastcall TPlot::DrawObs(int level)
{
    AnsiString label;
    TPoint p1,p2,p;
    gtime_t time;
    obsd_t *obs;
    double xs,ys,xt,xl[2],yl[2],tt[MAXSAT]={0},xp,xc,yc,yp[MAXSAT]={0};
    int i,j,m=0,sats[MAXSAT]={0},ind=ObsScroll->Position,prn,color;
    char id[16];
    
    trace(3,"DrawObs: level=%d\n",level);
    
    for (i=0;i<Obs.n;i++) {
        if (SatMask[Obs.data[i].sat-1]) continue;
        sats[Obs.data[i].sat-1]=1;
    }
    for (i=0;i<MAXSAT;i++) if (sats[i]) m++;
    
    GraphR->XLPos=TimeLabel?6:1; 
    GraphR->YLPos=0;
    GraphR->Week=Week;
    GraphR->GetLim(xl,yl);
    yl[0]=0.5;
    yl[1]=m>0?m+0.5:m+10.5;
    GraphR->SetLim(xl,yl);
    GraphR->SetTick(0.0,1.0);
    
    if (0<=ind&&ind<NObs&&BtnShowTrack->Down&&BtnFixHoriz->Down) {
        xp=TimePos(Obs.data[IndexObs[ind]].time);
        if (BtnFixHoriz->Down) {
            double xl[2],yl[2],off;
            GraphR->GetLim(xl,yl);
            off=Xcent*(xl[1]-xl[0])/2.0;
            GraphR->GetCent(xc,yc);
            GraphR->SetCent(xp-off,yc);
        }
        else {
            GraphR->GetRight(xc,yc);
            GraphR->SetRight(xp,yc);
        }
    }
    GraphR->DrawAxis(1,1);
    GraphR->GetPos(p1,p2);
    
    for (i=0,j=0;i<MAXSAT;i++) {
        if (!sats[i]) continue;
        p.x=p1.x;
        p.y=p1.y+(int)((p2.y-p1.y)*(j+0.5)/m);
        yp[i]=m-(j++);
        satno2id(i+1,id);
        label=id;
        GraphR->DrawText(p,label,CColor[2],2,0,0);
    }
    p1.x=Disp->Font->Size;
    p1.y=(p1.y+p2.y)/2;
    GraphR->DrawText(p1,"SATELLITE NO",CColor[2],0,0,90);
    
    if (!BtnSol1->Down) return;
    
    if (level&&PlotStyle<=2) {
        GraphR->GetScale(xs,ys);
        for (i=0;i<Obs.n;i++) {
            obs=&Obs.data[i];
            if ((color=ObsColor(obs,Az[i],El[i]))<0) continue;
            
            xt=TimePos(obs->time);
            if (fabs(xt-tt[obs->sat-1])/xs>=1.0) {
                GraphR->DrawMark(xt,yp[obs->sat-1],0,PlotStyle<2?MColor[color]:CColor[3],
                                 PlotStyle<2?MarkSize:0,0);
                tt[obs->sat-1]=xt;
            }
        }
    }
    if (level&&PlotStyle<=2) {
        DrawObsSlip(yp);
        DrawObsEphem(yp);
    }
    if (BtnShowTrack->Down&&0<=ind&&ind<NObs) {
        i=IndexObs[ind];
        time=Obs.data[i].time;
        
        GraphR->GetLim(xl,yl);
        xl[0]=xl[1]=TimePos(Obs.data[i].time);
        GraphR->DrawPoly(xl,yl,2,CColor[2],0);
        
        for (;i<Obs.n&&timediff(Obs.data[i].time,time)==0.0;i++) {
            obs=&Obs.data[i];
            if ((color=ObsColor(obs,Az[i],El[i]))<0) continue;
            GraphR->DrawMark(xl[0],yp[obs->sat-1],0,MColor[color],MarkSize*2+2,0);
        }
        GraphR->DrawMark(xl[0],yl[1]-1E-6,0,CColor[2],5,0);
        if (!BtnFixHoriz->Down) {
            GraphR->DrawMark(xl[0],yl[1]-1E-6,1,CColor[2],9,0);
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawObsSlip(double *yp)
{
    obsd_t *obs;
    TPoint ps[2];
    double gfp[MAXSAT]={0},gf;
    int slip[3];
    
    trace(3,"DrawObsSlip\n");
    
    if (ShowHalfC) {
        for (int i=0;i<Obs.n;i++) {
            obs=&Obs.data[i];
            
            if (El[i]<ElMask*D2R) continue;
            if (ElMaskP&&El[i]<ElMaskData[(int)(Az[i]*R2D+0.5)]) continue;
            if (!((ObsType->ItemIndex==0||ObsType->ItemIndex==1)&&(obs->LLI[0]&2))&&
                !((ObsType->ItemIndex==0||ObsType->ItemIndex==2)&&(obs->LLI[1]&2))) continue;
            
            if (GraphR->ToPoint(TimePos(obs->time),yp[obs->sat-1],ps[0])) {
                ps[1].x=ps[0].x;
                ps[1].y=ps[0].y+MarkSize*3/2+1;
                ps[0].y=ps[0].y-MarkSize*3/2;
                GraphR->DrawPoly(ps,2,MColor[0],0);
            }
        }
    }
    if (ShowSlip) {
        for (int i=0;i<Obs.n;i++) {
            obs=&Obs.data[i];
            slip[0]=obs->LLI[0]&1;
            slip[1]=obs->LLI[1]&1;
            slip[2]=0;
            if (obs->L[0]!=0.0&&obs->L[1]!=0.0) {
                gf=CLIGHT*(obs->L[0]/FREQ1-obs->L[1]/FREQ2);
                if (fabs(gfp[obs->sat-1]-gf)>THRESLIP) slip[2]=1;
                gfp[obs->sat-1]=gf;
            }
            if (El[i]<ElMask*D2R) continue;
            if (ElMaskP&&El[i]<ElMaskData[(int)(Az[i]*R2D+0.5)]) continue;
            
            if (!(ShowSlip==2&&(ObsType->ItemIndex==0||ObsType->ItemIndex==1)&&slip[0])&&
                !(ShowSlip==2&&(ObsType->ItemIndex==0||ObsType->ItemIndex==2)&&slip[1])&&
                !(ShowSlip==1&&ObsType->ItemIndex<=2&&slip[2])) continue;
            
            if (GraphR->ToPoint(TimePos(obs->time),yp[obs->sat-1],ps[0])) {
                ps[1].x=ps[0].x;
                ps[1].y=ps[0].y+MarkSize*3/2+1;
                ps[0].y=ps[0].y-MarkSize*3/2;
                GraphR->DrawPoly(ps,2,MColor[5],0);
            }
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawObsEphem(double *yp)
{
    TPoint ps[3];
    int i,j,k,in,off[MAXSAT]={0};
        
    
    trace(3,"DrawObsEphem\n");
    
    if (!ShowEph) return;
    
    for (i=0;i<MAXSAT;i++) {
        for (j=0;j<Nav.n;j++) {
            if (Nav.eph[j].sat!=i+1) continue;
            GraphR->ToPoint(TimePos(Nav.eph[j].ttr),yp[i],ps[0]);
            in=GraphR->ToPoint(TimePos(Nav.eph[j].toe),yp[i],ps[2]);
            ps[1]=ps[0];
            off[Nav.eph[j].sat-1]=off[Nav.eph[j].sat-1]?0:3;
            
            for (k=0;k<3;k++) ps[k].y+=MarkSize+2+off[Nav.eph[j].sat-1];
            ps[0].y-=2;
            
            GraphR->DrawPoly(ps,3,Nav.eph[j].svh?MColor[5]:CColor[1],0);
            
            if (in) GraphR->DrawMark(ps[2],0,Nav.eph[j].svh?MColor[5]:CColor[1],
                                     Nav.eph[j].svh?4:3,0);
        }
        for (j=0;j<Nav.ng;j++) {
            if (Nav.geph[j].sat!=i+1) continue;
            GraphR->ToPoint(TimePos(Nav.geph[j].tof),yp[i],ps[0]);
            in=GraphR->ToPoint(TimePos(Nav.geph[j].toe),yp[i],ps[2]);
            ps[1]=ps[0];
            off[Nav.geph[j].sat-1]=off[Nav.geph[j].sat-1]?0:3;
            for (k=0;k<3;k++) ps[k].y+=MarkSize+2+off[Nav.geph[j].sat-1];
            ps[0].y-=2;
            
            GraphR->DrawPoly(ps,3,Nav.geph[j].svh?MColor[5]:CColor[1],0);
            
            if (in) GraphR->DrawMark(ps[2],0,Nav.geph[j].svh?MColor[5]:CColor[1],
                                     Nav.geph[j].svh?4:3,0);
        }
        for (j=0;j<Nav.ns;j++) {
            if (Nav.seph[j].sat!=i+1) continue;
            GraphR->ToPoint(TimePos(Nav.seph[j].tof),yp[i],ps[0]);
            in=GraphR->ToPoint(TimePos(Nav.seph[j].t0),yp[i],ps[2]);
            ps[1]=ps[0];
            off[Nav.seph[j].sat-1]=off[Nav.seph[j].sat-1]?0:3;
            for (k=0;k<3;k++) ps[k].y+=MarkSize+2+off[Nav.seph[j].sat-1];
            ps[0].y-=2;
            
            GraphR->DrawPoly(ps,3,Nav.seph[j].svh?MColor[5]:CColor[1],0);
            
            if (in) GraphR->DrawMark(ps[2],0,Nav.seph[j].svh?MColor[5]:CColor[1],
                                     Nav.seph[j].svh?4:3,0);
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawSky(int level)
{
    TPoint p1,p2;
    AnsiString s,ss;
    obsd_t *obs;
    gtime_t t[MAXSAT]={{0}};
    double x,y,xp,yp,xs,ys,dt,dx,dy,xl[2],yl[2],r,p0[MAXSAT][2]={{0}};
    double p[MAXSAT][2]={{0}},gfp[MAXSAT]={0},gf;
    int i,j,ind=ObsScroll->Position,type=ObsType->ItemIndex;
    int hh=(int)(Disp->Font->Size*1.5),prn,color,slip[3];
    char id[16];
    
    trace(3,"DrawSky: level=%d\n",level);
    
    GraphS->GetLim(xl,yl);
    r=(xl[1]-xl[0]<yl[1]-yl[0]?xl[1]-xl[0]:yl[1]-yl[0])*0.45;
    GraphS->DrawSkyPlot(0.0,0.0,CColor[1],CColor[2],r*2.0);
    
    if (!BtnSol1->Down) return;
    
    GraphS->GetScale(xs,ys);
    
    if (PlotStyle<=2) {
        for (i=0;i<Obs.n;i++) {
            obs=&Obs.data[i];
            if (SatMask[obs->sat-1]||El[i]<=0.0) continue;
            if ((color=ObsColor(obs,Az[i],El[i]))<0) continue;
            
            x =r*sin(Az[i])*(1.0-2.0*El[i]/PI);
            y =r*cos(Az[i])*(1.0-2.0*El[i]/PI);
            xp=p[obs->sat-1][0];
            yp=p[obs->sat-1][1];
            
            if ((x-xp)*(x-xp)+(y-yp)*(y-yp)>=xs*xs) {
                int siz=PlotStyle<2?(satsys(obs->sat,NULL)==SYS_QZS?3:1)*MarkSize:1;
                GraphS->DrawMark(x,y,0,PlotStyle<2?MColor[color]:CColor[3],siz,0);
                p[obs->sat-1][0]=x;
                p[obs->sat-1][1]=y;
            }
            if (xp==0.0&&yp==0.0) {
                p0[obs->sat-1][0]=x;
                p0[obs->sat-1][1]=y;
            }
        }
    }
    if ((PlotStyle==0||PlotStyle==2)&&!BtnShowTrack->Down) {
        
        for (i=0;i<MAXSAT;i++) {
            if (p0[i][0]!=0.0||p0[i][1]!=0.0) {
                satno2id(i+1,id); s=id;
                GraphS->DrawText(p0[i][0],p0[i][1],s,CColor[2],1,0,0);
            }
        }
    }
    if (!level) return;
    
    if (ShowSlip&&PlotStyle<=2) {
        
        for (i=0;i<Obs.n;i++) {
            obs=&Obs.data[i];
            if (SatMask[obs->sat-1]||El[i]<=0.0) continue;
            
            slip[0]=obs->LLI[0];
            slip[1]=obs->LLI[1];
            slip[2]=0;
            if (obs->L[0]!=0.0&&obs->L[1]!=0.0) {
                gf=CLIGHT*(obs->L[0]/FREQ1-obs->L[1]/FREQ2);
                if (fabs(gfp[obs->sat-1]-gf)>THRESLIP) slip[2]=1;
                gfp[obs->sat-1]=gf;
            }
            x=r*sin(Az[i])*(1.0-2.0*El[i]/PI);
            y=r*cos(Az[i])*(1.0-2.0*El[i]/PI);
            
            dt=timediff(obs->time,t[obs->sat-1]);
            dx=x-p[obs->sat-1][0];
            dy=y-p[obs->sat-1][1];
            t[obs->sat-1]=obs->time;
            p[obs->sat-1][0]=x;
            p[obs->sat-1][1]=y;
            if (dt<-300.0||dt>300.0) continue;
            if (El[i]<ElMask*D2R) continue;
            if (ElMaskP&&El[i]<ElMaskData[(int)(Az[i]*R2D+0.5)]) continue;
            if ((ShowSlip==2&&(type==0||type==1)&&slip[0])||
                (ShowSlip==2&&(type==0||type==2)&&slip[1])||
                (ShowSlip==1&&type<=2&&slip[2])) {
                GraphS->DrawMark(x,y,4,MColor[5],MarkSize*3+2,atan2(dy,dx)*R2D+90);
            }
        }
    }
    if (ElMaskP) {
        double x[361],y[361];
        for (i=0;i<=360;i++) {
            x[i]=r*sin(i*D2R)*(1.0-2.0*ElMaskData[i]/PI);
            y[i]=r*cos(i*D2R)*(1.0-2.0*ElMaskData[i]/PI);
        }
        GraphS->DrawPoly(x,y,361,CColor[2],0);
    }
    if (BtnShowTrack->Down&&0<=ind&&ind<NObs) {
        
        for (i=IndexObs[ind];i<Obs.n&&i<IndexObs[ind+1];i++) {
            obs=&Obs.data[i];
            if (SatMask[obs->sat-1]) continue;
            if (El[i]<=0.0) continue;
            if ((color=ObsColor(obs,Az[i],El[i]))<0) continue;
            
            x=r*sin(Az[i])*(1.0-2.0*El[i]/PI);
            y=r*cos(Az[i])*(1.0-2.0*El[i]/PI);
            
            satno2id(obs->sat,id);
            GraphS->DrawMark(x,y,0,MColor[color],Disp->Font->Size*2+5,0);
            GraphS->DrawMark(x,y,1,color==0?MColor[0]:CColor[2],Disp->Font->Size*2+5,0);
            GraphS->DrawText(x,y,s=id,CColor[0],0,0,0);
        }
    }
    if (ShowStats&&BtnShowTrack->Down&&0<=ind&&ind<NObs) { // statistics
        
        GraphS->GetPos(p1,p2);
        p1.x=p2.x-10;
        p1.y+=8;
        s.sprintf("SAT : %-*s %-*s %-*s",NFREQ*2,"OBS",NFREQ*3,"SNR",NFREQ,"LLI");
        GraphS->DrawText(p1,s,CColor[2],2,2,0);
        p1.y+=3;
        
        for (i=IndexObs[ind];i<Obs.n&&i<IndexObs[ind+1];i++) {
            obs=&Obs.data[i];
            if (SatMask[obs->sat-1]) continue;
            if (El[i]<=0.0) continue;
            if (HideLowSat&&El[i]<ElMask*D2R) continue;
            if (HideLowSat&&ElMaskP&&El[i]<ElMaskData[(int)(Az[i]*R2D+0.5)]) continue;
            
            satno2id(obs->sat,id);
            s.sprintf("%-3s : ",id);
            for (j=0;j<NFREQ;j++) s+=obs->L[j]==0.0?"_":"L";
            for (j=0;j<NFREQ;j++) s+=obs->P[j]==0.0?"_":"P";
            s+=" ";
            for (j=0;j<NFREQ;j++) s+=ss.sprintf("%02d ",obs->SNR[j]);
            for (j=0;j<NFREQ;j++) s+=ss.sprintf("%d"   ,obs->LLI[j]);
            
            color=ObsColor(obs,Az[i],El[i]);
            p1.y+=hh;
            GraphS->DrawText(p1,s,MColor[color<0?0:color],2,2,0);
        }
    }
    if (Nav.n<=0&&Nav.ng<=0) {
        GraphS->GetPos(p1,p2);
        p2.x-=10;
        p2.y-=3;
        GraphS->DrawText(p2,"No Navigation Data",CColor[2],2,1,0);
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawDop(int level)
{
    AnsiString label;
    TPoint p1,p2;
    gtime_t time;
    double xp,xc,yc,xl[2],yl[2],azel[MAXSAT*2],*dop,*x,*y;
    int i,j,*ns,prn,n=0;
    int ind=ObsScroll->Position,doptype=DopType->ItemIndex;
    
    trace(3,"DrawDop: level=%d\n",level);
    
    GraphR->XLPos=TimeLabel?6:1; 
    GraphR->YLPos=1;
    GraphR->Week=Week;
    GraphR->GetLim(xl,yl);
    yl[0]=0.0; yl[1]=MaxDop;
    GraphR->SetLim(xl,yl);
    GraphR->SetTick(0.0,0.0);
    
    if (0<=ind&&ind<NObs&&BtnShowTrack->Down&&BtnFixHoriz->Down) {
        double xl[2],yl[2],off;
        GraphR->GetLim(xl,yl);
        off=Xcent*(xl[1]-xl[0])/2.0;
        xp=TimePos(Obs.data[IndexObs[ind]].time);
        GraphR->GetCent(xc,yc);
        GraphR->SetCent(xp-off,yc);
    }
    GraphR->DrawAxis(1,1);
    GraphR->GetPos(p1,p2);
    p1.x=Disp->Font->Size;
    p1.y=(p1.y+p2.y)/2;
    if (doptype==0) {
        label.sprintf("# OF SATELLITES / DOP (EL>=%.0f%s)",ElMask,CHARDEG);
    }
    else if (doptype==1) {
        label.sprintf("# OF SATELLITES (EL>=%.0f%s)",ElMask,CHARDEG);
    }
    else {
        label.sprintf("DOP (EL>=%.0f%s)",ElMask,CHARDEG);
    }
    GraphR->DrawText(p1,label,CColor[2],0,0,90);
    
    if (!BtnSol1->Down) return;
    
    x  =new double[NObs];
    y  =new double[NObs];
    dop=new double[NObs*4];
    ns =new int   [NObs];
    
    for (i=0;i<NObs;i++) {
        ns[n]=0;
        for (j=IndexObs[i];j<Obs.n&&j<IndexObs[i+1];j++) {
            if (SatMask[Obs.data[j].sat-1]) continue;
            if (El[j]<ElMask*D2R) continue;
            if (ElMaskP&&El[j]<ElMaskData[(int)(Az[j]*R2D+0.5)]) continue;
            azel[  ns[n]*2]=Az[j];
            azel[1+ns[n]*2]=El[j];
            ns[n]++;
        }
        dops(ns[n],azel,ElMask*D2R,dop+n*4);
        x[n++]=TimePos(Obs.data[IndexObs[i]].time);
    }
    for (i=0;i<4;i++) {
        if (doptype!=0&&doptype!=i+2) continue;
        
        for (j=0;j<n;j++) y[j]=dop[i+j*4];
        
        if (!(PlotStyle%2)) {
            DrawPolyS(GraphR,x,y,n,CColor[3],0);
        }
        if (level&&PlotStyle<2) {
            for (j=0;j<n;j++) {
                if (y[j]==0.0) continue;
                GraphR->DrawMark(x[j],y[j],0,MColor[i+2],MarkSize,0);
            }
        }
    }
    if (doptype==0||doptype==1) {
        for (i=0;i<n;i++) y[i]=ns[i];
        
        if (!(PlotStyle%2)) {
            DrawPolyS(GraphR,x,y,n,CColor[3],1);
        }
        if (level&&PlotStyle<2) {
            for (i=0;i<n;i++) {
                if (y[i]==0.0) continue;
                GraphR->DrawMark(x[i],y[i],0,MColor[1],MarkSize,0);
            }
        }
    }
    DrawDopStat(dop,ns,n);
    
    if (BtnShowTrack->Down&&0<=ind&&ind<NObs) {
        GraphR->GetLim(xl,yl);
        xl[0]=xl[1]=TimePos(Obs.data[IndexObs[ind]].time);
        
        GraphR->DrawPoly(xl,yl,2,CColor[2],0);
        
        ns[0]=0;
        for (i=IndexObs[ind];i<Obs.n&&i<IndexObs[ind+1];i++) {
            if (SatMask[Obs.data[i].sat-1]) continue;
            if (El[i]<ElMask*D2R) continue;
            if (ElMaskP&&El[i]<ElMaskData[(int)(Az[i]*R2D+0.5)]) continue;
            azel[  ns[0]*2]=Az[i];
            azel[1+ns[0]*2]=El[i];
            ns[0]++;
        }
        dops(ns[0],azel,ElMask*D2R,dop);
        
        for (i=0;i<4;i++) {
            if (doptype!=0&&doptype!=i+2) continue;
            GraphR->DrawMark(xl[0],dop[i],0,MColor[i+2],MarkSize*2+2,0);
        }
        if (doptype==0||doptype==1) {
            GraphR->DrawMark(xl[0],ns[0],0,MColor[1],MarkSize*2+2,0);
        }
        GraphR->DrawMark(xl[0],yl[1]-1E-6,0,CColor[2],5,0);
        if (!BtnFixHoriz->Down) {
            GraphR->DrawMark(xl[0],yl[1]-1E-6,1,CColor[2],9,0);
        }
    }
    if (Nav.n<=0&&Nav.ng<=0&&(doptype==0||doptype>=2)) {
        GraphR->GetPos(p1,p2);
        p2.x-=10;
        p2.y-=3;
        GraphS->DrawText(p2,"No Navigation Data",CColor[2],2,1,0);
    }
    delete [] x;
    delete [] y;
    delete [] dop;
    delete [] ns;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawDopStat(double *dop, int *ns, int n)
{
    AnsiString s0[MAXOBS+2],s1[MAXOBS+2],s2[MAXOBS+2];
    TPoint p1,p2,p3,p4;
    double ave[4]={0};
    int i,j,m=0;
    int ndop[4]={0},nsat[MAXOBS]={0},fonth=(int)(Disp->Font->Size*1.5);
    
    trace(3,"DrawDopStat: n=%d\n",n);
    
    if (!ShowStats) return;
    
    for (i=0;i<n;i++) nsat[ns[i]]++;
    
    for (i=0;i<4;i++) {
        for (j=0;j<n;j++) {
            if (dop[i+j*4]<=0.0||dop[i+j*4]>MaxDop) continue;
            ave[i]+=dop[i+j*4];
            ndop[i]++;
        }
        if (ndop[i]>0) ave[i]/=ndop[i];
    }
    if (DopType->ItemIndex==0||DopType->ItemIndex>=2) {
        s2[m++].sprintf("AVE= GDOP:%4.1f PDOP:%4.1f HDOP:%4.1f VDOP:%4.1f",
                        ave[0],ave[1],ave[2],ave[3]);
        s2[m++].sprintf("NDOP=%d(%4.1f%%) %d(%4.1f%%) %d(%4.1f%%) %d(%4.1f%%)",
                        ndop[0],n>0?ndop[0]*100.0/n:0.0,
                        ndop[1],n>0?ndop[1]*100.0/n:0.0,
                        ndop[2],n>0?ndop[2]*100.0/n:0.0,
                        ndop[3],n>0?ndop[3]*100.0/n:0.0);
    }
    if (DopType->ItemIndex<=1) {
        
        for (i=0,j=0;i<MAXOBS;i++) {
            if (nsat[i]<=0) continue;
            s0[m].sprintf("%s%2d:",j++==0?"NSAT= ":"",i);
            s1[m].sprintf("%7d",nsat[i]);
            s2[m++].sprintf("(%4.1f%%)",nsat[i]*100.0/n);
        }
    }
    GraphR->GetPos(p1,p2);
    p1.x=p2.x-10;
    p1.y+=8;
    p2=p1; p2.x-=fonth*4;
    p3=p2; p3.x-=fonth*8;
    
    for (i=0;i<m;i++,p1.y+=fonth,p2.y+=fonth,p3.y+=fonth) {
        GraphR->DrawText(p3,s0[i],CColor[2],2,2,0);
        GraphR->DrawText(p2,s1[i],CColor[2],2,2,0);
        GraphR->DrawText(p1,s2[i],CColor[2],2,2,0);
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawRes(int level, int frq)
{
    AnsiString label[]={
        "Pseudorange Residuals (m)",
        "Carrier-Phase Residuals (m)",
        "Elevation Angle (deg) / Signal Strength (dBHz)"
    };
    AnsiString satid,str;
    TSpeedButton *btn[]={BtnOn1,BtnOn2,BtnOn3};
    TPoint p1,p2;
    double xc,yc,xl[2],yl[2],res[2],sum[2]={0},sum2[2]={0};
    int i,j,ind=SolScroll->Position,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"DrawRes: level=%d frq=%d\n",level,frq);
    
    SatList1->Visible=!sel;
    SatList2->Visible=sel;
    satid=sel?SatList2->Text:SatList1->Text;
    
    if (0<=ind&&ind<SolData[sel].n&&BtnShowTrack->Down&&BtnFixHoriz->Down) {
        
        gtime_t t=SolData[sel].data[ind].time;
        
        for (i=0;i<3;i++) {
            if (BtnFixHoriz->Down) {
                double xl[2],yl[2],off;
                GraphG[i]->GetLim(xl,yl);
                off=Xcent*(xl[1]-xl[0])/2.0;
                GraphG[i]->GetCent(xc,yc);
                GraphG[i]->GetCent(xc,yc);
                GraphG[i]->SetCent(TimePos(t)-off,yc);
            }
            else {
                GraphG[i]->GetRight(xc,yc);
                GraphG[i]->SetRight(TimePos(t),yc);
            }
        }
    }
    j=-1;
    for (i=0;i<3;i++) if (btn[i]->Down) j=i;
    for (i=0;i<3;i++) {
        if (!btn[i]->Down) continue;
        GraphG[i]->XLPos=TimeLabel?(i==j?6:5):(i==j?1:0);
        GraphG[i]->Week=Week;
        GraphG[i]->DrawAxis(ShowLabel,ShowLabel);
        GraphG[i]->GetPos(p1,p2);
        p1.x+=5;
        p1.y+=3;
        GraphG[i]->DrawText(p1,label[i],CColor[2],1,2,0);
    }
    double *x,*y[4];
    int n=SolStat[sel].n;
    int m,ns[2]={0},*q,*s;
    
    if (n<=0||sel==0&&!BtnSol1->Down||sel==1&&!BtnSol2->Down) return;
    
    q   =new int[n];
    s   =new int[n];
    x   =new double[n],
    y[0]=new double[n];
    y[1]=new double[n];
    y[2]=new double[n];
    y[3]=new double[n];
    
    for (int sat=1;sat<=MAXSAT;sat++) {
        char id[32];
        satno2id(sat,id);
        if (satid!="ALL"&&satid!=id) continue;
        m=0;
        for (int i=0;i<n;i++) {
            solstat_t *p=SolStat[sel].data+i;
            if (p->sat!=sat||p->frq!=frq) continue;
            if (p->resp==0.0&&p->resc==0.0) continue;
            x[m]=TimePos(p->time);
            y[0][m]=p->resp;
            y[1][m]=p->resc;
            y[2][m]=p->el*R2D;
            y[3][m]=p->snr;
            if      (!(p->flag>>5))  q[m]=0; // invalid
            else if ((p->flag&7)==1) q[m]=2; // float
            else if ((p->flag&7)<=3) q[m]=1; // fixed
            else                     q[m]=6; // ppp
            s[m++]=(p->flag>>3)&0x3;         // slip
            
            if (p->resp!=0.0) {
                sum [0]+=p->resp;
                sum2[0]+=p->resp*p->resp;
                ns[0]++;
            }
            if (p->resc!=0.0) {
                sum [1]+=p->resc;
                sum2[1]+=p->resc*p->resc;
                ns[1]++;
            }
        }
        for (int i=0;i<3;i++) {
            if (!btn[i]->Down) continue;
            if (!level||!(PlotStyle%2)) {
                DrawPolyS(GraphG[i],x,y[i],m,CColor[3],0);
                if (i==2) DrawPolyS(GraphG[i],x,y[3],m,CColor[3],0);
            }
            if (level&&PlotStyle<2) {
                TColor color;
                for (int j=0;j<m;j++) {
                    color=i<2?MColor[q[j]]:MColor[1];
                    GraphG[i]->DrawMark(x[j],y[i][j],0,color,MarkSize,0);
                    if (i==2) GraphG[i]->DrawMark(x[j],y[3][j],0,MColor[4],MarkSize,0);
                }
            }
            if (level&&i==1) { /* slip */
                for (int j=0;j<m;j++) {
                    if (!s[j]) continue;
                    TColor color=s[j]&1?MColor[5]:MColor[0];
                    GraphG[i]->DrawMark(x[j],y[i][j],4,color,MarkSize*3,90);
                }
            }
        }
    }
    delete [] x;
    delete [] q;
    delete [] s;
    delete [] y[0];
    delete [] y[1];
    delete [] y[2];
    delete [] y[3];
    
    if (ShowStats) {
        for (int i=0;i<2;i++) {
            if (!btn[i]->Down) continue;
            double ave,std,rms;
            ave=ns[i]<=0?0.0:sum[i]/ns[i];
            std=ns[i]<=1?0.0:SQRT((sum2[i]-2.0*sum[i]*ave+ns[i]*ave*ave)/(ns[i]-1));
            rms=ns[i]<=0?0.0:SQRT(sum2[i]/ns[i]);
            GraphG[i]->GetPos(p1,p2);
            p1.x=p2.x-5;
            p1.y+=3;
            str.sprintf("AVE=%.3fm STD=%.3fm RMS=%.3fm",ave,std,rms);
            GraphG[i]->DrawText(p1,str,CColor[2],2,2,0);
        }
    }
    if (BtnShowTrack->Down&&0<=ind&&ind<SolData[sel].n&&(BtnSol1->Down||BtnSol2->Down)) {
        for (int i=0,j=0;i<3;i++) {
            if (!btn[i]->Down) continue;
            gtime_t t=SolData[sel].data[ind].time;
            GraphG[i]->GetLim(xl,yl);
            xl[0]=xl[1]=TimePos(t);
            GraphG[i]->DrawPoly(xl,yl,2,ind==0?CColor[1]:CColor[2],0);
            if (j++==0) {
                GraphG[i]->DrawMark(xl[0],yl[1]-1E-6,0,CColor[2],5,0);
                GraphG[i]->DrawMark(xl[0],yl[1]-1E-6,1,CColor[2],9,0);
            }
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DrawPolyS(TGraph *graph, double *x, double *y, int n,
    TColor color, int style)
{
    int i,j;
    
    for (i=0;i<n;i=j) { // draw poly with break
        for (j=i+1;j<n;j++) if (fabs(x[j]-x[j-1])>TBRK) break;
        graph->DrawPoly(x+i,y+i,j-i,color,style);
    }
}
//---------------------------------------------------------------------------
// class : TPlot common routines
//---------------------------------------------------------------------------
TIMEPOS * __fastcall TPlot::SolToPos(solbuf_t *sol, int index, int qflag, int type)
{
    TIMEPOS *pos,*vel,*acc;
    gtime_t ts={0};
    sol_t *data;
    double tint,xyz[3],xyzs[4];
    int i;
    
    trace(3,"SolToPos: n=%d\n",sol->n);
    
    pos=new TIMEPOS(index<0?sol->n:3,1);
    
    if (index>=0) {
        if (type==1&&index>sol->n-2) index=sol->n-2;
        if (type==2&&index>sol->n-3) index=sol->n-3;
    }
    for (i=index<0?0:index;data=getsol(sol,i);i++) {
        
        tint=TimeEna[2]?TimeInt:0.0;
        
        if (index<0&&!screent(data->time,ts,ts,tint)) continue;
        if (qflag&&data->stat!=qflag) continue;
        
        PosToXyz(data->time,data->rr,data->type,xyz);
        CovToXyz(data->rr,data->qr,data->type,xyzs);
        
        pos->t  [pos->n]=data->time;
        pos->x  [pos->n]=xyz [0];
        pos->y  [pos->n]=xyz [1];
        pos->z  [pos->n]=xyz [2];
        pos->xs [pos->n]=xyzs[0]; // var x^2
        pos->ys [pos->n]=xyzs[1]; // var y^2
        pos->zs [pos->n]=xyzs[2]; // var z^2
        pos->xys[pos->n]=xyzs[3]; // cov xy
        pos->q  [pos->n]=data->stat;
        pos->n++;
        
        if (index>=0&&pos->n>=3) break;
    }
    if (type!=1&&type!=2) return pos; // position
    
    vel=pos->tdiff();
    delete pos;
    if (type==1) return vel; // velocity
    
    acc=vel->tdiff();
    delete vel;
    return acc; // acceleration
}
//---------------------------------------------------------------------------
TIMEPOS * __fastcall TPlot::SolToNsat(solbuf_t *sol, int index, int qflag)
{
    TIMEPOS *ns;
    sol_t *data;
    int i;
    
    trace(3,"SolToNsat: n=%d\n",sol->n);
    
    ns=new TIMEPOS(index<0?sol->n:3,1);
    
    for (i=index<0?0:index;data=getsol(sol,i);i++) {
        
        if (qflag&&data->stat!=qflag) continue;
        
        ns->t[ns->n]=data->time;
        ns->x[ns->n]=data->ns;
        ns->y[ns->n]=data->age;
        ns->z[ns->n]=data->ratio;
        ns->q[ns->n]=data->stat;
        ns->n++;
        
        if (index>=0&&i>=2) break;
    }
    return ns;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::PosToXyz(gtime_t time, const double *rr, int type,
                                double *xyz)
{
    double opos[3],pos[3],r[3],enu[3];
    int i;
    
    trace(4,"SolToXyz:\n");
    
    if (type==0) { // xyz
        for (i=0;i<3;i++) {
            opos[i]=OPos[i];
            if (time.time==0.0||OEpoch.time==0.0) continue;
            opos[i]+=OVel[i]*timediff(time,OEpoch);
        }
        for (i=0;i<3;i++) r[i]=rr[i]-opos[i];
        ecef2pos(opos,pos);
        ecef2enu(pos,r,enu);
        xyz[0]=enu[0];
        xyz[1]=enu[1];
        xyz[2]=enu[2];
    }
    else { // enu
        xyz[0]=rr[0];
        xyz[1]=rr[1];
        xyz[2]=rr[2];
    }
}
//---------------------------------------------------------------------------
void __fastcall TPlot::CovToXyz(const double *rr, const float *qr, int type,
                                double *xyzs)
{
    double pos[3],P[9],Q[9];
    
    trace(4,"CovToXyz:\n");
    
    if (type==0) { // xyz
        ecef2pos(rr,pos);
        P[0]=qr[0];
        P[4]=qr[1];
        P[8]=qr[2];
        P[1]=P[3]=qr[3];
        P[5]=P[7]=qr[4];
        P[2]=P[6]=qr[5];
        covenu(pos,P,Q);
        xyzs[0]=Q[0];
        xyzs[1]=Q[4];
        xyzs[2]=Q[8];
        xyzs[3]=Q[1];
    }
    else { // enu
        xyzs[0]=qr[0];
        xyzs[1]=qr[1];
        xyzs[2]=qr[2];
        xyzs[3]=qr[3];
    }
}

//---------------------------------------------------------------------------
void __fastcall TPlot::CalcStats(const double *x, int n,
    double ref, double &ave, double &std, double &rms)
{
    double sum=0.0,sumsq=0.0;
    int i;
    
    trace(3,"CalcStats: n=%d\n",n);
    
    if (n<=0) {
        ave=std=rms=0.0;
        return;
    }
    ave=std=rms=0.0;
    
    for (i=0;i<n;i++) {
        sum  +=x[i];
        sumsq+=x[i]*x[i];
    }
    ave=sum/n;
    std=n>1?SQRT((sumsq-2.0*sum*ave+ave*ave*n)/(n-1)):0.0;
    rms=SQRT((sumsq-2.0*sum*ref+ref*ref*n)/n);
}
//---------------------------------------------------------------------------
int __fastcall TPlot::ObsColor(const obsd_t *obs, double az, double el)
{
    double snr,sns[]={45.0,40.0,35.0,30.0,25.0};
    int color=1,type=ObsType->ItemIndex,freq;
    
    trace(4,"ObsColor\n");
    
    if (HideLowSat) {
        if (el<ElMask*D2R) return -1;
        if (ElMaskP&&el<ElMaskData[(int)(az*R2D+0.5)]) return -1;
    }
    if (type==0) {
        if      (obs->L[0]!=0.0&&obs->L[1]!=0.0) color=1;
        else if (obs->L[0]!=0.0) color=2;
        else if (obs->L[1]!=0.0) color=3;
        else if (obs->P[0]!=0.0&&obs->P[1]) color=4;
        else if (obs->P[0]!=0.0) color=5;
        else if (obs->P[1]!=0.0) color=7;
    }
    else {
        freq=(type-1)%NFREQ;
        if (type-1< NFREQ&&obs->L[freq]==0.0) return -1;
        if (type-1>=NFREQ&&obs->P[freq]==0.0) return -1;
        snr=obs->SNR[freq];
        for (color=1;color<6;color++) if (snr>sns[color-1]) break;
        if (color==6) color=7;
    }
    if (el<ElMask*D2R) return 0;
    if (ElMaskP&&el<ElMaskData[(int)(az*R2D+0.5)]) return 0;
    
    return color;
}
//---------------------------------------------------------------------------
int __fastcall TPlot::SearchPos(int x, int y)
{
    sol_t *data;
    TPoint p(x,y);
    double xp,yp,xs,ys,r,xyz[3];
    int i,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"SearchPos: x=%d y=%d\n",x,y);
    
    if (!BtnShowTrack->Down||(!BtnSol1->Down&&!BtnSol2->Down)) return -1;
    
    GraphT->ToPos(p,xp,yp);
    GraphT->GetScale(xs,ys);
    r=(MarkSize/2+2)*xs;
    
    for (i=0;data=getsol(SolData+sel,i);i++) {
        if (QFlag->ItemIndex&&data->stat!=QFlag->ItemIndex) continue;
        
        PosToXyz(data->time,data->rr,data->type,xyz);
        
        if (SQR(xp-xyz[0])+SQR(yp-xyz[1])<=SQR(r)) return i;
    }
    return -1;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::TimeSpan(gtime_t *ts, gtime_t *te, double *tint)
{
    gtime_t t0={0};
    
    trace(3,"TimeSpan\n");
    
    *ts=*te=t0; *tint=0.0;
    if (TimeEna[0]) *ts=TimeStart;
    if (TimeEna[1]) *te=TimeEnd;
    if (TimeEna[2]) *tint=TimeInt;
}
//---------------------------------------------------------------------------
double __fastcall TPlot::TimePos(gtime_t time)
{
    double tow;
    int week;
    
    if (TimeLabel<=1) { // www/ssss or gpst
        tow=time2gpst(time,&week);
    }
    else if (TimeLabel==2) { // utc
        tow=time2gpst(gpst2utc(time),&week);
    }
    else { // jst
        tow=time2gpst(timeadd(gpst2utc(time),9*3600.0),&week);
    }
    return tow+(week-Week)*86400.0*7;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TPlot::TimeStr(gtime_t time, int n, int tsys)
{
    AnsiString s;
    char tstr[64],*label="";
    double tow;
    int week;
    
    if (TimeLabel==0) { // www/ssss
        tow=time2gpst(time,&week);
        sprintf(tstr,"%4d/%*.*fs",week,(n>0?6:5)+n,n,tow);
    }
    else if (TimeLabel==1) { // gpst
        time2str(time,tstr,n);
        label=" GPST";
    }
    else if (TimeLabel==2) { // utc
        time2str(gpst2utc(time),tstr,n);
        label=" UTC";
    }
    else { // jst
        time2str(timeadd(gpst2utc(time),9*3600.0),tstr,n);
        label=" JST";
    }
    return s.sprintf("%s%s",tstr,label);
}
//---------------------------------------------------------------------------
void __fastcall TPlot::ShowMsg(AnsiString msg)
{
    Message1->Caption=msg;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::ShowLegend(AnsiString *msgs)
{
    TLabel *ql[]={QL1,QL2,QL3,QL4,QL5,QL6,QL7};
    int i;
    
    trace(3,"ShowLegend\n");
    
    for (i=0;i<7;i++) {
        if (!msgs||msgs[i]=="") {
            ql[i]->Caption=" "; ql[i]->Width=1;
        }
        else {
            ql[i]->Caption=msgs[i];
            ql[i]->Font->Color=MColor[i+1];
        }
    }
}
//---------------------------------------------------------------------------
int __fastcall TPlot::ExecCmd(AnsiString cmd)
{
    PROCESS_INFORMATION info;
    STARTUPINFO si={0};
    si.cb=sizeof(si);
    char *p=cmd.c_str();
    
    if (!CreateProcess(NULL,p,NULL,NULL,false,CREATE_NO_WINDOW,NULL,NULL,&si,
                       &info)) return 0;
    CloseHandle(info.hProcess);
    CloseHandle(info.hThread);
    return 1;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::LoadOpt(void)
{
    TIniFile *ini=new TIniFile("rtkplot.ini");
    AnsiString s,s1;
    int i;
    
    trace(3,"LoadOpt\n");
    
    TimeLabel    =ini->ReadInteger("plot","timelabel",     1);
    AutoScale    =ini->ReadInteger("plot","autoscale",     1);
    ShowStats    =ini->ReadInteger("plot","showstats",     0);
    ShowLabel    =ini->ReadInteger("plot","showlabel",     1);
    ShowGLabel   =ini->ReadInteger("plot","showglabel",    0);
    ShowCompass  =ini->ReadInteger("plot","showcompass",   0);
    ShowScale    =ini->ReadInteger("plot","showscale",     1);
    ShowArrow    =ini->ReadInteger("plot","showarrow",     0);
    ShowSlip     =ini->ReadInteger("plot","showslip",      0);
    ShowHalfC    =ini->ReadInteger("plot","showhalfc",     0);
    ShowErr      =ini->ReadInteger("plot","showerr",       0);
    ShowEph      =ini->ReadInteger("plot","showeph",       0);
    PlotStyle    =ini->ReadInteger("plot","plotstyle",     0);
    MarkSize     =ini->ReadInteger("plot","marksize",      2);
    NavSys       =ini->ReadInteger("plot","navsys",  SYS_GPS);
    AnimCycle    =ini->ReadInteger("plot","animcycle",    10);
    RefCycle     =ini->ReadInteger("plot","refcycle",    100);
    HideLowSat   =ini->ReadInteger("plot","hidelowsat",    0);
    ElMaskP      =ini->ReadInteger("plot","elmaskp",       0);
    ExSats       =ini->ReadString ("plot","exsats",       "");
    RtBuffSize   =ini->ReadInteger("plot","rtbuffsize",10800);
    RtStream     =ini->ReadInteger("plot","rtstream",      0);
    RtFormat     =ini->ReadInteger("plot","rtformat",      0);
    RtTimeForm   =ini->ReadInteger("plot","rttimeform",    0);
    RtDegForm    =ini->ReadInteger("plot","rtdegform",     0);
    RtFieldSep   =ini->ReadString ("plot","rtfieldsep",   "");
    RtTimeOutTime=ini->ReadInteger("plot","rttimeouttime", 0);
    RtReConnTime =ini->ReadInteger("plot","rtreconntime",10000);
    
    MColor[0]=(TColor)ini->ReadInteger("plot","mcolor0",(int)clSilver );
    MColor[1]=(TColor)ini->ReadInteger("plot","mcolor1",(int)clGreen  );
    MColor[2]=(TColor)ini->ReadInteger("plot","mcolor2",     0x00AAFF );
    MColor[3]=(TColor)ini->ReadInteger("plot","mcolor3",(int)clFuchsia);
    MColor[4]=(TColor)ini->ReadInteger("plot","mcolor4",(int)clBlue   );
    MColor[5]=(TColor)ini->ReadInteger("plot","mcolor5",(int)clRed    );
    MColor[6]=(TColor)ini->ReadInteger("plot","mcolor6",(int)clTeal   );
    MColor[7]=clGray;
    CColor[0]=(TColor)ini->ReadInteger("plot","color1", (int)clWhite  );
    CColor[1]=(TColor)ini->ReadInteger("plot","color2", (int)clSilver );
    CColor[2]=(TColor)ini->ReadInteger("plot","color3", (int)clBlack  );
    CColor[3]=(TColor)ini->ReadInteger("plot","color4", (int)clSilver );
    
    RefDialog->StaPosFile=ini->ReadString ("plot","staposfile","");
    RefDialog->Format    =ini->ReadInteger("plot","staposformat",0);
    
    ElMask    =ini->ReadFloat  ("plot","elmask", 0.0);
    MaxDop    =ini->ReadFloat  ("plot","maxdop",30.0);
    YRange    =ini->ReadFloat  ("plot","yrange", 5.0);
    Origin    =ini->ReadInteger("plot","orgin",    2);
    OOPos[0]  =ini->ReadFloat  ("plot","oopos1",   0);
    OOPos[1]  =ini->ReadFloat  ("plot","oopos2",   0);
    OOPos[2]  =ini->ReadFloat  ("plot","oopos3",   0);
    QcCmd     =ini->ReadString ("plot","qccmd","teqc +qc +sym +l -rep -plot");
    
    Font->Charset=ANSI_CHARSET;
    Font->Name=ini->ReadString ("plot","fontname","Tahoma");
    Font->Size=ini->ReadInteger("plot","fontsize",8);
    
    for (i=0;i<2;i++) {
        StrCmds    [i]=ini->ReadString ("str",s.sprintf("strcmd_%d",     i),"");
        StrCmdEna  [i]=ini->ReadInteger("str",s.sprintf("strcmdena_%d",  i), 0);
    }
    for (i=0;i<3;i++) {
        StrPaths   [i]=ini->ReadString ("str",s.sprintf("strpath_%d",    i),"");
    }
    for (i=0;i<10;i++) {
        StrHistory [i]=ini->ReadString ("str",s.sprintf("strhistry_%d",  i),"");
        StrMntpHist[i]=ini->ReadString ("str",s.sprintf("strmntphist_%d",i),"");
    }
    NMapPnt   =ini->ReadInteger("plot","nmappnt",0);
    for (i=0;i<NMapPnt;i++) {
        PntName[i]=ini->ReadString("plot",s.sprintf("pntname%d",i+1),"");
        s1=ini->ReadString("plot",s.sprintf("pntpos%d",i+1),"0,0,0");
        PntPos[i][0]=PntPos[i][1]=PntPos[i][2]=0.0;
        sscanf(s1.c_str(),"%lf,%lf,%lf",PntPos[i],PntPos[i]+1,PntPos[i]+2);
    }
    TTextViewer::Color1=(TColor)ini->ReadInteger("viewer","color1",(int)clBlack);
    TTextViewer::Color2=(TColor)ini->ReadInteger("viewer","color2",(int)clWhite);
    TTextViewer::FontD=new TFont;
    TTextViewer::FontD->Name=ini->ReadString ("viewer","fontname","Courier New");
    TTextViewer::FontD->Size=ini->ReadInteger("viewer","fontsize",9);
    
    delete ini;
}
//---------------------------------------------------------------------------
void __fastcall TPlot::SaveOpt(void)
{
    TIniFile *ini=new TIniFile("rtkplot.ini");
    AnsiString s,s1;
    int i;
    
    trace(3,"SaveOpt\n");
    
    ini->WriteInteger("plot","timelabel",    TimeLabel    );
    ini->WriteInteger("plot","autoscale",    AutoScale    );
    ini->WriteInteger("plot","showstats",    ShowStats    );
    ini->WriteInteger("plot","showlabel",    ShowLabel    );
    ini->WriteInteger("plot","showglabel",   ShowGLabel   );
    ini->WriteInteger("plot","showcompass",  ShowCompass  );
    ini->WriteInteger("plot","showscale",    ShowScale    );
    ini->WriteInteger("plot","showarrow",    ShowArrow    );
    ini->WriteInteger("plot","showslip",     ShowSlip     );
    ini->WriteInteger("plot","showhalfc",    ShowHalfC    );
    ini->WriteInteger("plot","showerr",      ShowErr      );
    ini->WriteInteger("plot","showeph",      ShowEph      );
    ini->WriteInteger("plot","plotstyle",    PlotStyle    );
    ini->WriteInteger("plot","marksize",     MarkSize     );
    ini->WriteInteger("plot","navsys",       NavSys       );
    ini->WriteInteger("plot","animcycle",    AnimCycle    );
    ini->WriteInteger("plot","refcycle",     RefCycle     );
    ini->WriteInteger("plot","hidelowsat",   HideLowSat   );
    ini->WriteInteger("plot","elmaskp",      ElMaskP      );
    ini->WriteString ("plot","exsats",       ExSats       );
    ini->WriteInteger("plot","rtbuffsize",   RtBuffSize   );
    ini->WriteInteger("plot","rtstream",     RtStream     );
    ini->WriteInteger("plot","rtformat",     RtFormat     );
    ini->WriteInteger("plot","rttimeform",   RtTimeForm   );
    ini->WriteInteger("plot","rtdegform",    RtDegForm    );
    ini->WriteString ("plot","rtfieldsep",   RtFieldSep   );
    ini->WriteInteger("plot","rttimeouttime",RtTimeOutTime);
    ini->WriteInteger("plot","rtreconntime", RtReConnTime );
    
    ini->WriteInteger("plot","mcolor0",     (int)MColor[0]);
    ini->WriteInteger("plot","mcolor1",     (int)MColor[1]);
    ini->WriteInteger("plot","mcolor2",     (int)MColor[2]);
    ini->WriteInteger("plot","mcolor3",     (int)MColor[3]);
    ini->WriteInteger("plot","mcolor4",     (int)MColor[4]);
    ini->WriteInteger("plot","mcolor5",     (int)MColor[5]);
    ini->WriteInteger("plot","mcolor6",     (int)MColor[6]);
    ini->WriteInteger("plot","color1",      (int)CColor[0]);
    ini->WriteInteger("plot","color2",      (int)CColor[1]);
    ini->WriteInteger("plot","color3",      (int)CColor[2]);
    ini->WriteInteger("plot","color4",      (int)CColor[3]);
    
    ini->WriteString ("plot","staposfile",   RefDialog->StaPosFile);
    ini->WriteInteger("plot","staposformat", RefDialog->Format);
    
    ini->WriteFloat  ("plot","elmask",       ElMask        );
    ini->WriteFloat  ("plot","maxdop",       MaxDop        );
    ini->WriteFloat  ("plot","yrange",       YRange        );
    ini->WriteInteger("plot","orgin",        Origin        );
    ini->WriteFloat  ("plot","oopos1",       OOPos[0]      );
    ini->WriteFloat  ("plot","oopos2",       OOPos[1]      );
    ini->WriteFloat  ("plot","oopos3",       OOPos[2]      );
    ini->WriteString ("plot","qccmd",        QcCmd         );
    
    ini->WriteString ("plot","fontname",     Font->Name    );
    ini->WriteInteger("plot","fontsize",     Font->Size    );
    
    for (i=0;i<2;i++) {
        ini->WriteString ("str",s.sprintf("strcmd_%d",     i),StrCmds    [i]);
        ini->WriteInteger("str",s.sprintf("strcmdena_%d",  i),StrCmdEna  [i]);
    }
    for (i=0;i<3;i++) {
        ini->WriteString ("str",s.sprintf("strpath_%d",    i),StrPaths   [i]);
    }
    for (i=0;i<10;i++) {
        ini->WriteString ("str",s.sprintf("strhistry_%d",  i),StrHistory [i]);
        ini->WriteString ("str",s.sprintf("strmntphist_%d",i),StrMntpHist[i]);
    }
    ini->WriteInteger("plot","nmappnt",NMapPnt);
    for (i=0;i<NMapPnt;i++) {
        ini->WriteString("plot",s.sprintf("pntname%d",i+1),PntName[i]);
        ini->WriteString("plot",s.sprintf("pntpos%d",i+1),
            s1.sprintf("%.4f,%.4f,%.4f",PntPos[i][0],PntPos[i][1],PntPos[i][2]));
    }
    ini->WriteInteger("viewer","color1",(int)TTextViewer::Color1  );
    ini->WriteInteger("viewer","color2",(int)TTextViewer::Color2  );
    ini->WriteString ("viewer","fontname",TTextViewer::FontD->Name);
    ini->WriteInteger("viewer","fontsize",TTextViewer::FontD->Size);
    
    delete ini;
}
//---------------------------------------------------------------------------

