//---------------------------------------------------------------------------
// rtkpost : post processing positioning ap
//
//          Copyright (C) 2007-2010 by T.TAKASU, All rights reserved.
//
// version : $Revision: 1.1 $ $Date: 2008/07/17 22:14:45 $
// history : 2008/07/14  1.0 new
//           2008/11/17  1.1 rtklib 2.1.1
//           2008/04/03  1.2 rtklib 2.3.1
//           2010/07/18  1.3 rtklib 2.4.0
//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "postmain.h"
#include "postopt.h"
#include "kmzconv.h"
#include "refdlg.h"
#include "timedlg.h"
#include "confdlg.h"
#include "aboutdlg.h"
#include "viewer.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TMainForm *MainForm;

#define PRGNAME     "RTKPOST"
#define MAXHIST     20
#define GOOGLE_EARTH "C:\\Program Files\\Google\\Google Earth\\googleearth.exe"

static const char version[]="$Revision: 1.1 $ $Date: 2008/07/17 22:14:45 $";

//---------------------------------------------------------------------------
extern "C" {
static gtime_t tstart_={0};
static gtime_t tend_  ={0};
static char rov_ [256]="";
static char base_[256]="";

//---------------------------------------------------------------------------
extern int showmsg(char *format, ...)
{
    va_list arg;
    char buff[1024];
    if (*format) {
        va_start(arg,format);
        vsprintf(buff,format,arg);
        va_end(arg);
        MainForm->ShowMsg(buff);
    }
    else Application->ProcessMessages();
    return !MainForm->BtnExec->Enabled;
}
//---------------------------------------------------------------------------
extern void settspan(gtime_t ts, gtime_t te)
{
    tstart_=ts;
    tend_  =te;
}
//---------------------------------------------------------------------------
extern void settime(gtime_t time)
{
    static int i=0;
    double tt;
    if (tend_.time!=0&&tstart_.time!=0&&(tt=timediff(tend_,tstart_))>0.0) {
        MainForm->Progress->Position=(int)(timediff(time,tstart_)/tt*100.0+0.5);
    }
    if (i++%23==0) Application->ProcessMessages();
}
}
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
    double val=0.0;
    sscanf(str.c_str(),"%lf",&val);
    return val;
}
//---------------------------------------------------------------------------
// TMainForm: event handling
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
    : TForm(Owner)
{
    int i;
    
    DynamicModel=IonoOpt=TropOpt=RovAntPcv=RefAntPcv=AmbRes=0;
    RovPosType=RefPosType=0;
    OutCntResetAmb=5; LockCntFixAmb=5; FixCntHoldAmb=10;
    MaxAgeDiff=30.0; RejectThres=30.0;
    MeasErr1=100.0; MeasErr2=0.004; MeasErr3=0.003; MeasErr4=1.0;
    SatClkStab=1E-11; ValidThresAR=3.0;
    RovAntE=RovAntN=RovAntU=RefAntE=RefAntN=RefAntU=0.0;
    for (i=0;i<3;i++) RovPos[i]=0.0;
    for (i=0;i<3;i++) RefPos[i]=0.0;
    
    DoubleBuffered=true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
    DragAcceptFiles(Handle,true);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
    AnsiString s;
    LoadOpt();
    UpdateEnable();
    Caption=s.sprintf("%s ver.%s",PRGNAME,VER_RTKLIB);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    SaveOpt();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::DropFiles(TWMDropFiles msg)
{
    POINT point={0};
    int top;
    char *p,file[1024];
    
    if (DragQueryFile((HDROP)msg.Drop,0xFFFFFFFF,NULL,0)<=0) return;
    DragQueryFile((HDROP)msg.Drop,0,file,sizeof(file));
    if (!DragQueryPoint((HDROP)msg.Drop,&point)) return;
    
    top=Panel1->Top+Panel4->Top;
    if (point.y<=top+InputFile1->Top+InputFile1->Height) {
        InputFile1->Text=file;
        if (!(p=strrchr(file,'.'))) p=file+strlen(file);
        strcpy(p,".pos");
        OutputFile->Text=file;
    }
    else if (point.y<=top+InputFile2->Top+InputFile2->Height) {
        InputFile2->Text=file;
    }
    else if (point.y<=top+InputFile3->Top+InputFile3->Height) {
        InputFile3->Text=file;
    }
    else if (point.y<=top+InputFile4->Top+InputFile4->Height) {
        InputFile4->Text=file;
    }
    else if (point.y<=top+InputFile5->Top+InputFile5->Height) {
        InputFile5->Text=file;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnPlotClick(TObject *Sender)
{
    AnsiString file=FilePath(OutputFile->Text);
    AnsiString cmd="rtkplot \""+file+"\"";
    if (!ExecCmd(cmd,1)) ShowMsg("error : rtkplot execution");
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnViewClick(TObject *Sender)
{
    ViewFile(FilePath(OutputFile->Text));
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnToKMLClick(TObject *Sender)
{
    ConvDialog->Show(); 
    ConvDialog->SetInput(FilePath(OutputFile->Text));
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnOptionClick(TObject *Sender)
{
    if (OptDialog->ShowModal()!=mrOk) return;
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnExecClick(TObject *Sender)
{
    char *p;
    
    if (BtnExec->Caption=="Abort") {
        BtnExec->Enabled=false;
        return;
    }
    if (InputFile1->Text=="") {
        showmsg("error : no rinex obs file (rover)");
        return;
    }
    if (InputFile2->Text==""&&PosMode>0) {
        showmsg("error : no rinex obs file (base station)");
        return;
    }
    if (OutputFile->Text=="") {
        showmsg("error : no output file");
        return;
    }
    if (p=strrchr(OutputFile->Text.c_str(),'.')) {
        if (!strcmp(p,".obs")||!strcmp(p,".OBS")||!strcmp(p,".nav")||
            !strcmp(p,".NAV")||!strcmp(p,".gnav")||!strcmp(p,".GNAV")||
            !strcmp(p,".gz")||!strcmp(p,".Z")||
            !strcmp(p+3,"o")||!strcmp(p+3,"O")||!strcmp(p+3,"d")||
            !strcmp(p+3,"D")||!strcmp(p+3,"n")||!strcmp(p+3,"N")||
            !strcmp(p+3,"g")||!strcmp(p+3,"G")) {
            showmsg("error : invalid extension of output file (%s)",p);
            return;
        }
    }
    showmsg("");
    BtnExec  ->Caption="Abort";
    BtnExit  ->Enabled=false;
    BtnView  ->Enabled=false;
    BtnToKML ->Enabled=false;
    BtnPlot  ->Enabled=false;
    BtnOption->Enabled=false;
    Panel1   ->Enabled=false;
    
    if (ExecProc()>=0) {
        AddHist(InputFile1);
        AddHist(InputFile2);
        AddHist(InputFile3);
        AddHist(InputFile4);
        AddHist(InputFile5);
        AddHist(OutputFile);
    }
    if (strstr(Message->Caption.c_str(),"processing")) {
        showmsg("done");
    }
    BtnExec  ->Caption="E&xecute";
    BtnExec  ->Enabled=true;
    BtnExit  ->Enabled=true;
    BtnView  ->Enabled=true;
    BtnToKML ->Enabled=true;
    BtnPlot  ->Enabled=true;
    BtnOption->Enabled=true;
    Panel1   ->Enabled=true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnStopClick(TObject *Sender)
{
    showmsg("abort");
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnExitClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnAboutClick(TObject *Sender)
{
    AnsiString prog=PRGNAME;
#ifdef MKL
    prog+="_MKL";
#endif
    AboutDialog->About=prog;
    AboutDialog->IconIndex=1;
    AboutDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnTime1Click(TObject *Sender)
{
    TimeDialog->Time=GetTime1();
    TimeDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnTime2Click(TObject *Sender)
{
    TimeDialog->Time=GetTime2();
    TimeDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnInputFile1Click(TObject *Sender)
{
    char file[1024],*p;
    
    OpenDialog->Title="RINEX OBS (Rover) File";
    OpenDialog->FileName="";
    OpenDialog->FilterIndex=0;
    if (!OpenDialog->Execute()) return;
    InputFile1->Text=OpenDialog->FileName;
    
    strcpy(file,InputFile1->Text.c_str());
    if (!(p=strrchr(file,'.'))) p=file+strlen(file);
    strcpy(p,".pos");
    for (p=file;*p;p++) if (*p=='*') *p='0';
    OutputFile->Text=file;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnInputFile2Click(TObject *Sender)
{
    OpenDialog->Title="RINEX OBS (Base Station) File";
    OpenDialog->FileName="";
    OpenDialog->FilterIndex=0;
    if (!OpenDialog->Execute()) return;
    InputFile2->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnInputFile3Click(TObject *Sender)
{
    OpenDialog->Title="RINEX NAV/GNAV, SP3/RINEX CLK or SBAS Log/EMS File";
    OpenDialog->FileName="";
    OpenDialog->FilterIndex=0;
    if (!OpenDialog->Execute()) return;
    InputFile3->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnInputFile4Click(TObject *Sender)
{
    OpenDialog->Title="RINEX NAV/GNAV, SP3/RINEX CLK or SBAS Log/EMS File";
    OpenDialog->FileName="";
    OpenDialog->FilterIndex=0;
    if (!OpenDialog->Execute()) return;
    InputFile4->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnInputFile5Click(TObject *Sender)
{
    OpenDialog->Title="RINEX NAV/GNAV, SP3/RINEX CLK or SBAS Log/EMS File";
    OpenDialog->FileName="";
    OpenDialog->FilterIndex=0;
    if (!OpenDialog->Execute()) return;
    InputFile5->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnOutputFileClick(TObject *Sender)
{
    SaveDialog->Title="Output File";
    OpenDialog->FileName="";
    if (!SaveDialog->Execute()) return;
    OutputFile->Text=SaveDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnInputView1Click(TObject *Sender)
{
    ViewFile(FilePath(InputFile1->Text));
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnInputView2Click(TObject *Sender)
{
    ViewFile(FilePath(InputFile2->Text));
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnInputView3Click(TObject *Sender)
{
    AnsiString file=FilePath(InputFile3->Text);
    char f[1024];
    
    if (file=="") {
        file=FilePath(InputFile1->Text);
        if (!ObsToNav(file.c_str(),f)) return;
        file=f;
    }
    ViewFile(file);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnInputView4Click(TObject *Sender)
{
    AnsiString file=FilePath(InputFile4->Text);
    char f[1024];
    
    if (file=="") {
        file=FilePath(InputFile1->Text);
        if (!ObsToGnav(file.c_str(),f)) return;
        file=f;
    }
    ViewFile(file);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnInputView5Click(TObject *Sender)
{
    ViewFile(FilePath(InputFile5->Text));
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnOutputView1Click(TObject *Sender)
{
    AnsiString file=FilePath(OutputFile->Text)+".stat";
    FILE *fp=fopen(file.c_str(),"r");
    if (fp) fclose(fp); else return;
    ViewFile(file);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnOutputView2Click(TObject *Sender)
{
    AnsiString file=FilePath(OutputFile->Text)+".trace";
    FILE *fp=fopen(file.c_str(),"r");
    if (fp) fclose(fp); else return;
    ViewFile(file);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnInputPlot1Click(TObject *Sender)
{
    AnsiString files[5],cmd;
    char navfile[1024],gnavfile[1024];
    
    files[0]=FilePath(InputFile1->Text);
    files[1]=FilePath(InputFile2->Text);
    files[2]=FilePath(InputFile3->Text);
    files[3]=FilePath(InputFile4->Text);
    
    if (files[2]=="") {
        if (ObsToNav(files[0].c_str(),navfile)) files[2]=navfile;
    }
    if (files[3]=="") {
        if (ObsToGnav(files[0].c_str(),gnavfile)) files[3]=gnavfile;
    }
    cmd="rtkplot -r \""+files[0]+"\" \""+files[2]+"\" \""+files[3]+"\"";
    
    if (!ExecCmd(cmd,1)) ShowMsg("error : rtkplot execution");
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnInputPlot2Click(TObject *Sender)
{
    AnsiString files[5],cmd;
    char navfile[1024],gnavfile[1024];
    
    files[0]=FilePath(InputFile1->Text);
    files[1]=FilePath(InputFile2->Text);
    files[2]=FilePath(InputFile3->Text);
    files[3]=FilePath(InputFile4->Text);
    
    if (files[2]=="") {
        if (ObsToNav(files[0].c_str(),navfile)) files[2]=navfile;
    }
    if (files[3]=="") {
        if (ObsToGnav(files[0].c_str(),gnavfile)) files[3]=gnavfile;
    }
    cmd="rtkplot -r \""+files[1]+"\" \""+files[2]+"\" \""+files[3]+"\"";
    
    if (!ExecCmd(cmd,1)) ShowMsg("error : rtkplot execution");
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::RefPosClick(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeStartClick(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeIntFClick(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeUnitFClick(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeY1UDChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
    AnsiString s;
    double ep[]={2000,1,1,0,0,0};
    int p=TimeY1->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeY1->Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    if (4<p&&p<8) {
        ep[1]+=ud;
        if (ep[1]<=0) {ep[0]--; ep[1]+=12;}
        else if (ep[1]>12) {ep[0]++; ep[1]-=12;}
    }
    else if (p>7||p==0) ep[2]+=ud; else ep[0]+=ud;
    time2epoch(epoch2time(ep),ep);
    TimeY1->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
    TimeY1->SelStart=p>7||p==0?10:(p>4?7:4);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeH1UDChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
    AnsiString s;
    int hms[3]={0},sec,p=TimeH1->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeH1->Text.c_str(),"%d:%d:%d",hms,hms+1,hms+2);
    if (p>5||p==0) hms[2]+=ud; else if (p>2) hms[1]+=ud; else hms[0]+=ud;
    sec=hms[0]*3600+hms[1]*60+hms[2];
    if (sec<0) sec+=86400; else if (sec>=86400) sec-=86400;
    TimeH1->Text=s.sprintf("%02d:%02d:%02d",sec/3600,(sec%3600)/60,sec%60);
    TimeH1->SelStart=p>5||p==0?8:(p>2?5:2);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeY2UDChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
    AnsiString s;
    double ep[]={2000,1,1,0,0,0};
    int p=TimeY2->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeY2->Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    if (4<p&&p<8) {
        ep[1]+=ud;
        if (ep[1]<=0) {ep[0]--; ep[1]+=12;}
        else if (ep[1]>12) {ep[0]++; ep[1]-=12;}
    }
    else if (p>7||p==0) ep[2]+=ud; else ep[0]+=ud;
    time2epoch(epoch2time(ep),ep);
    TimeY2->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
    TimeY2->SelStart=p>7||p==0?10:(p>4?7:4);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeH2UDChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
    AnsiString s;
    int hms[3]={0},sec,p=TimeH2->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeH2->Text.c_str(),"%d:%d:%d",hms,hms+1,hms+2);
    if (p>5||p==0) hms[2]+=ud; else if (p>2) hms[1]+=ud; else hms[0]+=ud;
    sec=hms[0]*3600+hms[1]*60+hms[2];
    if (sec<0) sec+=86400; else if (sec>=86400) sec-=86400;
    TimeH2->Text=s.sprintf("%02d:%02d:%02d",sec/3600,(sec%3600)/60,sec%60);
    TimeH2->SelStart=p>5||p==0?8:(p>2?5:2);
}

//---------------------------------------------------------------------------
void __fastcall TMainForm::InputFile1Change(TObject *Sender)
{
    char *p,file[1024];
    
    if (InputFile1->Text=="") return;
    strcpy(file,InputFile1->Text.c_str());
    if (!(p=strrchr(file,'.'))) p=file+strlen(file);
    strcpy(p,".pos");
    for (p=file;*p;p++) if (*p=='*') *p='0';
    OutputFile->Text=file;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::PosModeChange(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SolutionChange(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SolFormatChange(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
// TMainForm: command execution
//---------------------------------------------------------------------------
int __fastcall TMainForm::ExecProc(void)
{
    FILE *fp;
    prcopt_t prcopt=prcopt_default;
    solopt_t solopt=solopt_default;
    filopt_t filopt={""};
    gtime_t ts={0},te={0};
    double ti=0.0,tu=0.0;
    int i,n=0,stat;
    char infile_[5][1024]={""},*infile[5],outfile[1024];
    char *rov,*base,*p,*q,*r;
    
    // get processing options
    if (TimeStart->Checked) ts=GetTime1();
    if (TimeEnd  ->Checked) te=GetTime2();
    if (TimeIntF ->Checked) ti=str2dbl(TimeInt ->Text);
    if (TimeUnitF->Checked) tu=str2dbl(TimeUnit->Text)*3600.0;
    
    if (!GetOption(prcopt,solopt,filopt)) return 0;
    
    // set input/output files
    for (i=0;i<5;i++) infile[i]=infile_[i];
    
    strcpy(infile[n++],InputFile1->Text.c_str());
    
    if (PMODE_DGPS<=prcopt.mode&&prcopt.mode<=PMODE_FIXED) {
        strcpy(infile[n++],InputFile2->Text.c_str());
    }
    if (InputFile3->Text!="") {
        strcpy(infile[n++],InputFile3->Text.c_str());
    }
    else if ((prcopt.navsys&SYS_GPS)&&!ObsToNav(InputFile1->Text.c_str(),infile[n++])) {
        showmsg("error: no gps navigation data");
        return 0;
    }
    if (InputFile4->Text!="") {
        strcpy(infile[n++],InputFile4->Text.c_str());
    }
    else if ((prcopt.navsys&SYS_GLO)&&!ObsToGnav(InputFile1->Text.c_str(),infile[n++])) {
        showmsg("error: no glonass navigation data");
        return 0;
    }
    if (InputFile5->Text!="") {
        strcpy(infile[n++],InputFile5->Text.c_str());
    }
    strcpy(outfile,OutputFile->Text.c_str());
    
    // confirm overwrite
    if (!TimeStart->Checked||!TimeEnd->Checked) {
        if ((fp=fopen(outfile,"r"))) {
            fclose(fp);
            ConfDialog->Label2->Caption=outfile;
            if (ConfDialog->ShowModal()!=mrOk) return 0;
        }
    }
    // set rover and base station list
    rov =new char [strlen(RovList .c_str())];
    base=new char [strlen(BaseList.c_str())];
    
    for (p=RovList.c_str(),r=rov;*p;p=q+2) {
        
        if (!(q=strstr(p,"\r\n"))) {
            if (*p!='#') strcpy(r,p); break;
        }
        else if (*p!='#') {
            strncpy(r,p,q-p); r+=q-p;
            strcpy(r++," ");
        }
    }
    for (p=BaseList.c_str(),r=base;*p;p=q+2) {
        
        if (!(q=strstr(p,"\r\n"))) {
            if (*p!='#') strcpy(r,p); break;
        }
        else if (*p!='#') {
            strncpy(r,p,q-p); r+=q-p;
            strcpy(r++," ");
        }
    }
    // post processing positioning
    if ((stat=postpos(ts,te,ti,tu,&prcopt,&solopt,&filopt,infile,n,outfile,
                      rov,base))==1) {
        showmsg("aborted");
    }
    delete [] rov ;
    delete [] base;
    
    return stat;
}
//---------------------------------------------------------------------------
int __fastcall TMainForm::GetOption(prcopt_t &prcopt, solopt_t &solopt,
                                    filopt_t &filopt)
{
    char buff[1024],id[32],*p;
    int sat;
    
    // processing options
    prcopt.mode     =PosMode;
    prcopt.soltype  =Solution;
    prcopt.nf       =Freq+1;
    prcopt.navsys   =NavSys;
    prcopt.elmin    =ElMask*D2R;
    prcopt.snrmin   =SnrMask;
    prcopt.sateph   =SatEphem;
    prcopt.modear   =AmbRes;
    prcopt.glomodear=GloAmbRes;
    prcopt.maxout   =OutCntResetAmb;
    prcopt.minfix   =FixCntHoldAmb;
    prcopt.minlock  =LockCntFixAmb;
    prcopt.ionoopt  =IonoOpt;
    prcopt.tropopt  =TropOpt;
    prcopt.dynamics =DynamicModel;
    prcopt.tidecorr =TideCorr;
    prcopt.niter    =NumIter;
    prcopt.intpref  =IntpRefObs;
    prcopt.err[0]   =MeasErr1;
    prcopt.err[1]   =MeasErr2;
    prcopt.err[2]   =MeasErr3;
    prcopt.err[3]   =MeasErr4;
    prcopt.err[4]   =MeasErr5;
    prcopt.prn[0]   =PrNoise1;
    prcopt.prn[1]   =PrNoise2;
    prcopt.prn[2]   =PrNoise3;
    prcopt.prn[3]   =PrNoise4;
    prcopt.prn[4]   =PrNoise5;
    prcopt.sclkstab =SatClkStab;
    prcopt.thresar  =ValidThresAR;
    prcopt.elmaskar =ElMaskAR*D2R;
    prcopt.thresslip=SlipThres;
    prcopt.maxtdiff =MaxAgeDiff;
    prcopt.maxinno  =RejectThres;
    if (BaseLineConst) {
        prcopt.baseline[0]=BaseLine[0];
        prcopt.baseline[1]=BaseLine[1];
    }
    else {
        prcopt.baseline[0]=0.0;
        prcopt.baseline[1]=0.0;
    }
    if (PosMode!=PMODE_FIXED) {
        for (int i=0;i<3;i++) prcopt.ru[i]=0.0;
    }
    else if (RovPosType<=2) {
        for (int i=0;i<3;i++) prcopt.ru[i]=RovPos[i];
    }
    else prcopt.rovpos=RovPosType-2; /* 1:single,2:posfile,3:rinex */
    
    if (PosMode==PMODE_SINGLE||PosMode==PMODE_MOVEB) {
        for (int i=0;i<3;i++) prcopt.rb[i]=0.0;
    }
    else if (RefPosType<=2) {
        for (int i=0;i<3;i++) prcopt.rb[i]=RefPos[i];
    }
    else prcopt.refpos=RefPosType-2;
    
    if (RovAntPcv) {
        strcpy(prcopt.anttype[0],RovAnt.c_str());
        prcopt.antdel[0][0]=RovAntE;
        prcopt.antdel[0][1]=RovAntN;
        prcopt.antdel[0][2]=RovAntU;
    }
    if (RefAntPcv) {
        strcpy(prcopt.anttype[1],RefAnt.c_str());
        prcopt.antdel[1][0]=RefAntE;
        prcopt.antdel[1][1]=RefAntN;
        prcopt.antdel[1][2]=RefAntU;
    }
    if (ExSats!="") { // excluded satellites
        strcpy(buff,ExSats.c_str());
        for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
            if (!(sat=satid2no(p))) continue;
            prcopt.exsats[sat-1]=1;
        }
    }
    // solution options
    solopt.posf     =SolFormat;
    solopt.times    =TimeFormat==0?0:TimeFormat-1;
    solopt.timef    =TimeFormat==0?0:1;
    solopt.timeu    =TimeDecimal<=0?0:TimeDecimal;
    solopt.degf     =LatLonFormat;
    solopt.outhead  =OutputHead;
    solopt.outopt   =OutputOpt;
    solopt.datum    =OutputDatum;
    solopt.height   =OutputHeight;
    solopt.geoid    =OutputGeoid;
    solopt.solstatic=SolStatic;
    solopt.sstat    =DebugStatus;
    solopt.trace    =DebugTrace;
    strcpy(solopt.sep,FieldSep!=""?FieldSep.c_str():" ");
    sprintf(solopt.prog,"%s ver.%s",PRGNAME,VER_RTKLIB);
    
    // file options
    strcpy(filopt.satantp,SatPcvFile.c_str());
    strcpy(filopt.rcvantp,AntPcvFile.c_str());
    strcpy(filopt.stapos, StaPosFile.c_str());
    strcpy(filopt.geoid,  GeoidDataFile.c_str());
    strcpy(filopt.dcb,    DCBFile.c_str());
    
    return 1;
}
//---------------------------------------------------------------------------
// TMainForm: common functions
//---------------------------------------------------------------------------
int __fastcall TMainForm::ObsToNav(const char *obsfile, char *navfile)
{
    char *p;
    strcpy(navfile,obsfile);
    if (!(p=strrchr(navfile,'.'))) return 0;
    if      (strlen(p)==4&&*(p+3)=='o') *(p+3)='n';
    else if (strlen(p)==4&&*(p+3)=='d') *(p+3)='n';
    else if (strlen(p)==4&&*(p+3)=='O') *(p+3)='N';
    else if (!strcmp(p,".obs")) strcpy(p,".nav");
    else if (!strcmp(p,".OBS")) strcpy(p,".NAV");
    else if (!strcmp(p,".gz")||!strcmp(p,".Z")) {
        if      (*(p-1)=='o') *(p-1)='n';
        else if (*(p-1)=='d') *(p-1)='n';
        else if (*(p-1)=='O') *(p-1)='N';
        else return 0;
    }
    else return 0;
    return 1;
}
//---------------------------------------------------------------------------
int __fastcall TMainForm::ObsToGnav(const char *obsfile, char *gnavfile)
{
    char *p;
    strcpy(gnavfile,obsfile);
    if (!(p=strrchr(gnavfile,'.'))) return 0;
    if      (strlen(p)==4&&*(p+3)=='o') *(p+3)='g';
    else if (strlen(p)==4&&*(p+3)=='d') *(p+3)='g';
    else if (strlen(p)==4&&*(p+3)=='O') *(p+3)='G';
    else if (!strcmp(p,".obs")) strcpy(p,".gnav");
    else if (!strcmp(p,".OBS")) strcpy(p,".GNAV");
    else if (!strcmp(p,".gz")||!strcmp(p,".Z")) {
        if      (*(p-1)=='o') *(p-1)='g';
        else if (*(p-1)=='d') *(p-1)='g';
        else if (*(p-1)=='O') *(p-1)='G';
        else return 0;
    }
    else return 0;
    return 1;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TMainForm::FilePath(AnsiString file)
{
    AnsiString s;
    gtime_t ts={0};
    char rov[256]="",base[256]="",path[1024],*p,*q;
    
    if (TimeStart->Checked) ts=GetTime1();
    
    for (p=RovList.c_str();(q=strstr(p,"\r\n"));p=q+2) {
        if (*p&&*p!='#') break;
    }
    if (!q) strcpy(rov,p); else strncpy(rov,p,q-p);
    
    for (p=BaseList.c_str();(q=strstr(p,"\r\n"));p=q+2) {
        if (*p&&p[0]!='#') break;
    }
    if (!q) strcpy(base,p); else strncpy(base,p,q-p);
    
    reppath(file.c_str(),path,ts,rov,base);
    
    return (s=path);
}
//---------------------------------------------------------------------------
TStringList * __fastcall TMainForm::ReadList(TIniFile *ini, AnsiString cat,
    AnsiString key)
{
    TStringList *list=new TStringList;
    AnsiString s,item;
    int i;
    
    for (i=0;i<100;i++) {
        item=ini->ReadString(cat,s.sprintf("%s_%03d",key.c_str(),i),"");
        if (item!="") list->Add(item); else break;
    }
    return list;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::WriteList(TIniFile *ini, AnsiString cat,
    AnsiString key, TStrings *list)
{
    AnsiString s;
    int i;
    
    for (i=0;i<list->Count;i++) {
        ini->WriteString(cat,s.sprintf("%s_%03d",key.c_str(),i),list->Strings[i]);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::AddHist(TComboBox *combo)
{
    AnsiString hist=combo->Text;
    if (hist=="") return;
    TStrings *list=combo->Items;
    int i=list->IndexOf(hist);
    if (i>=0) list->Delete(i);
    list->Insert(0,hist);
    for (int i=list->Count-1;i>=MAXHIST;i--) list->Delete(i);
    combo->ItemIndex=0;
}
//---------------------------------------------------------------------------
int __fastcall TMainForm::ExecCmd(AnsiString cmd, int show)
{
    PROCESS_INFORMATION info;
    STARTUPINFO si={0};
    si.cb=sizeof(si);
    char *p=cmd.c_str();
    
    if (!CreateProcess(NULL,p,NULL,NULL,false,show?0:CREATE_NO_WINDOW,NULL,
                       NULL,&si,&info)) return 0;
    CloseHandle(info.hProcess);
    CloseHandle(info.hThread);
    return 1;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ViewFile(AnsiString file)
{
    TTextViewer *viewer;
    AnsiString f;
    char tmpfile[1024];
    int cstat;
    
    if (file=="") return;
    cstat=uncompress(file.c_str(),tmpfile);
    f=!cstat?file.c_str():tmpfile;
    
    viewer=new TTextViewer(Application);
    viewer->Caption=file;
    viewer->Show();
    viewer->Read(f);
    if (cstat==1) remove(tmpfile);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ShowMsg(char *msg)
{
    Message->Caption=msg;
    Message->Font->Color=strstr(msg,"error")?clRed:clGray;
}
//---------------------------------------------------------------------------
gtime_t _fastcall TMainForm::GetTime1(void)
{
    double ep[]={2000,1,1,0,0,0};
    
    sscanf(TimeY1->Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    sscanf(TimeH1->Text.c_str(),"%lf:%lf:%lf",ep+3,ep+4,ep+5);
    return epoch2time(ep);
}
//---------------------------------------------------------------------------
gtime_t _fastcall TMainForm::GetTime2(void)
{
    double ep[]={2000,1,1,0,0,0};
    
    sscanf(TimeY2->Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    sscanf(TimeH2->Text.c_str(),"%lf:%lf:%lf",ep+3,ep+4,ep+5);
    return epoch2time(ep);
}
//---------------------------------------------------------------------------
void _fastcall TMainForm::SetTime1(gtime_t time)
{
    AnsiString s;
    double ep[6];
    
    time2epoch(time,ep);
    TimeY1->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
    TimeH1->Text=s.sprintf("%02.0f:%02.0f:%02.0f",ep[3],ep[4],ep[5]);
    TimeY1->SelStart=10; TimeH1->SelStart=10;
}
//---------------------------------------------------------------------------
void _fastcall TMainForm::SetTime2(gtime_t time)
{
    AnsiString s;
    double ep[6];
    
    time2epoch(time,ep);
    TimeY2->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
    TimeH2->Text=s.sprintf("%02.0f:%02.0f:%02.0f",ep[3],ep[4],ep[5]);
    TimeY2->SelStart=10; TimeH2->SelStart=10;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::UpdateEnable(void)
{
    int moder=PMODE_DGPS<=PosMode&&PosMode<=PMODE_FIXED;
    
    InputFile2     ->Enabled=moder;
    BtnInputFile2  ->Enabled=moder;
    BtnInputPlot2  ->Enabled=moder;
    BtnInputView2  ->Enabled=moder;
    BtnOutputView1 ->Enabled=DebugStatus>0;
    BtnOutputView2 ->Enabled=DebugTrace >0;
    LabelInputFile3->Enabled=moder;
    TimeY1         ->Enabled=TimeStart->Checked;
    TimeH1         ->Enabled=TimeStart->Checked;
    TimeY1UD       ->Enabled=TimeStart->Checked;
    TimeH1UD       ->Enabled=TimeStart->Checked;
    BtnTime1       ->Enabled=TimeStart->Checked;
    TimeY2         ->Enabled=TimeEnd  ->Checked;
    TimeH2         ->Enabled=TimeEnd  ->Checked;
    TimeY2UD       ->Enabled=TimeEnd  ->Checked;
    TimeH2UD       ->Enabled=TimeEnd  ->Checked;
    BtnTime2       ->Enabled=TimeEnd  ->Checked;
    TimeInt        ->Enabled=TimeIntF ->Checked;
    LabelTimeInt   ->Enabled=TimeIntF ->Checked;
    TimeUnitF      ->Enabled=TimeStart->Checked&&TimeEnd  ->Checked;
    TimeUnit       ->Enabled=TimeUnitF->Enabled&&TimeUnitF->Checked;
    LabelTimeUnit  ->Enabled=TimeUnitF->Enabled&&TimeUnitF->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::LoadOpt(void)
{
    TIniFile *ini=new TIniFile("rtkpost.ini");
    AnsiString s;
    char *p;
    
    TimeStart->Checked =ini->ReadInteger("set","timestart",   0);
    TimeEnd->Checked   =ini->ReadInteger("set","timeend",     0);
    TimeY1->Text       =ini->ReadString ("set","timey1",      "2000/01/01");
    TimeY1->Text       =ini->ReadString ("set","timey1",      "2000/01/01");
    TimeH1->Text       =ini->ReadString ("set","timeh1",      "00:00:00");
    TimeY2->Text       =ini->ReadString ("set","timey2",      "2000/01/01");
    TimeH2->Text       =ini->ReadString ("set","timeh2",      "00:00:00");
    TimeIntF ->Checked =ini->ReadInteger("set","timeintf",    0);
    TimeInt->Text      =ini->ReadString ("set","timeint",     "0");
    TimeUnitF->Checked =ini->ReadInteger("set","timeunitf",   0);
    TimeUnit->Text     =ini->ReadString ("set","timeunit",    "24");
    InputFile1->Text   =ini->ReadString ("set","inputfile1",  "");
    InputFile2->Text   =ini->ReadString ("set","inputfile2",  "");
    InputFile3->Text   =ini->ReadString ("set","inputfile3",  "");
    InputFile4->Text   =ini->ReadString ("set","inputfile4",  "");
    InputFile5->Text   =ini->ReadString ("set","inputfile5",  "");
    OutputFile->Text   =ini->ReadString ("set","outputfile",  "");
    
    InputFile1->Items  =ReadList(ini,"hist","inputfile1");
    InputFile2->Items  =ReadList(ini,"hist","inputfile2");
    InputFile3->Items  =ReadList(ini,"hist","inputfile3");
    InputFile4->Items  =ReadList(ini,"hist","inputfile4");
    InputFile5->Items  =ReadList(ini,"hist","inputfile5");
    OutputFile->Items  =ReadList(ini,"hist","outputfile");
    
    PosMode            =ini->ReadInteger("opt","posmode",        0);
    Freq               =ini->ReadInteger("opt","freq",           1);
    Solution           =ini->ReadInteger("opt","solution",       0);
    ElMask             =ini->ReadFloat  ("opt","elmask",      15.0);
    SnrMask            =ini->ReadFloat  ("opt","snrmask",      0.0);
    IonoOpt            =ini->ReadInteger("opt","ionoopt",     IONOOPT_BRDC);
    TropOpt            =ini->ReadInteger("opt","tropopt",     TROPOPT_SAAS);
    RcvBiasEst         =ini->ReadInteger("opt","rcvbiasest",     0);
    DynamicModel       =ini->ReadInteger("opt","dynamicmodel",   0);
    TideCorr           =ini->ReadInteger("opt","tidecorr",       0);
    SatEphem           =ini->ReadInteger("opt","satephem",       0);
    ExSats             =ini->ReadString ("opt","exsats",        "");
    NavSys             =ini->ReadInteger("opt","navsys",   SYS_GPS);
    
    AmbRes             =ini->ReadInteger("opt","ambres",         1);
    GloAmbRes          =ini->ReadInteger("opt","gloambres",      1);
    ValidThresAR       =ini->ReadFloat  ("opt","validthresar", 3.0);
    LockCntFixAmb      =ini->ReadInteger("opt","lockcntfixamb",  0);
    FixCntHoldAmb      =ini->ReadInteger("opt","fixcntholdamb", 10);
    ElMaskAR           =ini->ReadFloat  ("opt","elmaskar",     0.0);
    OutCntResetAmb     =ini->ReadInteger("opt","outcntresetbias",5);
    SlipThres          =ini->ReadFloat  ("opt","slipthres",   0.05);
    MaxAgeDiff         =ini->ReadFloat  ("opt","maxagediff",  30.0);
    RejectThres        =ini->ReadFloat  ("opt","rejectthres", 30.0);
    NumIter            =ini->ReadInteger("opt","numiter",        1);
    CodeSmooth         =ini->ReadInteger("opt","codesmooth",     0);
    BaseLine[0]        =ini->ReadFloat  ("opt","baselinelen",  0.0);
    BaseLine[1]        =ini->ReadFloat  ("opt","baselinesig",  0.0);
    BaseLineConst      =ini->ReadInteger("opt","baselineconst",  0);
    
    SolFormat          =ini->ReadInteger("opt","solformat",      0);
    TimeFormat         =ini->ReadInteger("opt","timeformat",     1);
    TimeDecimal        =ini->ReadInteger("opt","timedecimal",    3);
    LatLonFormat       =ini->ReadInteger("opt","latlonformat",   0);
    FieldSep           =ini->ReadString ("opt","fieldsep",      "");
    OutputHead         =ini->ReadInteger("opt","outputhead",     1);
    OutputOpt          =ini->ReadInteger("opt","outputopt",      1);
    OutputDatum        =ini->ReadInteger("opt","outputdatum",    0);
    OutputHeight       =ini->ReadInteger("opt","outputheight",   0);
    OutputGeoid        =ini->ReadInteger("opt","outputgeoid",    0);
    SolStatic          =ini->ReadInteger("opt","solstatic",      0);
    DebugTrace         =ini->ReadInteger("opt","debugtrace",     0);
    DebugStatus        =ini->ReadInteger("opt","debugstatus",    0);
    
    MeasErr1           =ini->ReadFloat  ("opt","measerr1",   100.0);
    MeasErr2           =ini->ReadFloat  ("opt","measerr2",   0.003);
    MeasErr3           =ini->ReadFloat  ("opt","measerr3",   0.003);
    MeasErr4           =ini->ReadFloat  ("opt","measerr4",   0.000);
    MeasErr5           =ini->ReadFloat  ("opt","measerr5",  10.000);
    SatClkStab         =ini->ReadFloat  ("opt","satclkstab", 5E-12);
    PrNoise1           =ini->ReadFloat  ("opt","prnoise1",    1E-4);
    PrNoise2           =ini->ReadFloat  ("opt","prnoise2",    1E-3);
    PrNoise3           =ini->ReadFloat  ("opt","prnoise3",    1E-4);
    PrNoise4           =ini->ReadFloat  ("opt","prnoise4",    1E+1);
    PrNoise5           =ini->ReadFloat  ("opt","prnoise5",    1E+1);
    
    RovPosType         =ini->ReadInteger("opt","rovpostype",     0);
    RefPosType         =ini->ReadInteger("opt","refpostype",     0);
    RovPos[0]          =ini->ReadFloat  ("opt","rovpos1",      0.0);
    RovPos[1]          =ini->ReadFloat  ("opt","rovpos2",      0.0);
    RovPos[2]          =ini->ReadFloat  ("opt","rovpos3",      0.0);
    RefPos[0]          =ini->ReadFloat  ("opt","refpos1",      0.0);
    RefPos[1]          =ini->ReadFloat  ("opt","refpos2",      0.0);
    RefPos[2]          =ini->ReadFloat  ("opt","refpos3",      0.0);
    RovAntPcv          =ini->ReadInteger("opt","rovantpcv",      0);
    RefAntPcv          =ini->ReadInteger("opt","refantpcv",      0);
    RovAnt             =ini->ReadString ("opt","rovant",        "");
    RefAnt             =ini->ReadString ("opt","refant",        "");
    RovAntE            =ini->ReadFloat  ("opt","rovante",      0.0);
    RovAntN            =ini->ReadFloat  ("opt","rovantn",      0.0);
    RovAntU            =ini->ReadFloat  ("opt","rovantu",      0.0);
    RefAntE            =ini->ReadFloat  ("opt","refante",      0.0);
    RefAntN            =ini->ReadFloat  ("opt","refantn",      0.0);
    RefAntU            =ini->ReadFloat  ("opt","refantu",      0.0);
    
    AntPcvFile         =ini->ReadString ("opt","antpcvfile",    "");
    IntpRefObs         =ini->ReadInteger("opt","intprefobs",     0);
    NetRSCorr          =ini->ReadInteger("opt","netrscorr",      0);
    SatClkCorr         =ini->ReadInteger("opt","satclkcorr",     0);
    SbasCorr           =ini->ReadInteger("opt","sbascorr",       0);
    SbasCorr1          =ini->ReadInteger("opt","sbascorr1",      0);
    SbasCorr2          =ini->ReadInteger("opt","sbascorr2",      0);
    SbasCorr3          =ini->ReadInteger("opt","sbascorr3",      0);
    SbasCorr4          =ini->ReadInteger("opt","sbascorr4",      0);
    SbasCorrFile       =ini->ReadString ("opt","sbascorrfile",  "");
    PrecEphFile        =ini->ReadString ("opt","precephfile",   "");
    SatPcvFile         =ini->ReadString ("opt","satpcvfile",    "");
    StaPosFile         =ini->ReadString ("opt","staposfile",    "");
    GeoidDataFile      =ini->ReadString ("opt","geoiddatafile", "");
    DCBFile            =ini->ReadString ("opt","dcbfile",       "");
    GoogleEarthFile    =ini->ReadString ("opt","googleearthfile",GOOGLE_EARTH);
    
    RovList            =ini->ReadString ("opt","rovlist",       "");
    for (p=RovList.c_str();*p;p++) {
        if ((p=strstr(p,"@@"))) strncpy(p,"\r\n",2); else break;
    }
    BaseList           =ini->ReadString ("opt","baselist",      "");
    for (p=BaseList.c_str();*p;p++) {
        if ((p=strstr(p,"@@"))) strncpy(p,"\r\n",2); else break;
    }
    ConvDialog->TimeSpan  ->Checked  =ini->ReadInteger("conv","timespan",  0);
    ConvDialog->TimeIntF  ->Checked  =ini->ReadInteger("conv","timeintf",  0);
    ConvDialog->TimeY1    ->Text     =ini->ReadString ("conv","timey1","2000/01/01");
    ConvDialog->TimeH1    ->Text     =ini->ReadString ("conv","timeh1","00:00:00"  );
    ConvDialog->TimeY2    ->Text     =ini->ReadString ("conv","timey2","2000/01/01");
    ConvDialog->TimeH2    ->Text     =ini->ReadString ("conv","timeh2","00:00:00"  );
    ConvDialog->TimeInt   ->Text     =ini->ReadString ("conv","timeint", "0");
    ConvDialog->TrackColor->ItemIndex=ini->ReadInteger("conv","trackcolor",5);
    ConvDialog->PointColor->ItemIndex=ini->ReadInteger("conv","pointcolor",5);
    ConvDialog->OutputAlt ->ItemIndex=ini->ReadInteger("conv","outputalt", 0);
    ConvDialog->OutputTime->ItemIndex=ini->ReadInteger("conv","outputtime",0);
    ConvDialog->AddOffset ->Checked  =ini->ReadInteger("conv","addoffset", 0);
    ConvDialog->Offset1   ->Text     =ini->ReadString ("conv","offset1", "0");
    ConvDialog->Offset2   ->Text     =ini->ReadString ("conv","offset2", "0");
    ConvDialog->Offset3   ->Text     =ini->ReadString ("conv","offset3", "0");
    ConvDialog->Compress  ->Checked  =ini->ReadInteger("conv","compress",  0);
    
    TTextViewer::Color1=(TColor)ini->ReadInteger("viewer","color1",(int)clBlack);
    TTextViewer::Color2=(TColor)ini->ReadInteger("viewer","color2",(int)clWhite);
    TTextViewer::FontD=new TFont;
    TTextViewer::FontD->Name=ini->ReadString ("viewer","fontname","Courier New");
    TTextViewer::FontD->Size=ini->ReadInteger("viewer","fontsize",9);
    delete ini;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SaveOpt(void)
{
    TIniFile *ini=new TIniFile("rtkpost.ini");
    char *p;
    
    ini->WriteInteger("set","timestart",   TimeStart ->Checked?1:0);
    ini->WriteInteger("set","timeend",     TimeEnd   ->Checked?1:0);
    ini->WriteString ("set","timey1",      TimeY1    ->Text);
    ini->WriteString ("set","timeh1",      TimeH1    ->Text);
    ini->WriteString ("set","timey2",      TimeY2    ->Text);
    ini->WriteString ("set","timeh2",      TimeH2    ->Text);
    ini->WriteInteger("set","timeintf",    TimeIntF  ->Checked?1:0);
    ini->WriteString ("set","timeint",     TimeInt   ->Text);
    ini->WriteInteger("set","timeunitf",   TimeUnitF ->Checked?1:0);
    ini->WriteString ("set","timeunit",    TimeUnit  ->Text);
    ini->WriteString ("set","inputfile1",  InputFile1->Text);
    ini->WriteString ("set","inputfile2",  InputFile2->Text);
    ini->WriteString ("set","inputfile3",  InputFile3->Text);
    ini->WriteString ("set","inputfile4",  InputFile4->Text);
    ini->WriteString ("set","inputfile5",  InputFile5->Text);
    ini->WriteString ("set","outputfile",  OutputFile->Text);
    
    WriteList(ini,"hist","inputfile1",     InputFile1->Items);
    WriteList(ini,"hist","inputfile2",     InputFile2->Items);
    WriteList(ini,"hist","inputfile3",     InputFile3->Items);
    WriteList(ini,"hist","inputfile4",     InputFile4->Items);
    WriteList(ini,"hist","inputfile5",     InputFile5->Items);
    WriteList(ini,"hist","outputfile",     OutputFile->Items);
    
    ini->WriteInteger("opt","posmode",     PosMode     );
    ini->WriteInteger("opt","freq",        Freq        );
    ini->WriteInteger("opt","solution",    Solution    );
    ini->WriteFloat  ("opt","elmask",      ElMask      );
    ini->WriteFloat  ("opt","snrmask",     SnrMask     );
    ini->WriteInteger("opt","ionoopt",     IonoOpt     );
    ini->WriteInteger("opt","tropopt",     TropOpt     );
    ini->WriteInteger("opt","rcvbiasest",  RcvBiasEst  );
    ini->WriteInteger("opt","dynamicmodel",DynamicModel);
    ini->WriteInteger("opt","tidecorr",    TideCorr    );
    ini->WriteInteger("opt","satephem",    SatEphem    );
    ini->WriteString ("opt","exsats",      ExSats      );
    ini->WriteInteger("opt","navsys",      NavSys      );
    
    ini->WriteInteger("opt","ambres",      AmbRes      );
    ini->WriteInteger("opt","gloambres",   GloAmbRes   );
    ini->WriteFloat  ("opt","validthresar",ValidThresAR);
    ini->WriteInteger("opt","lockcntfixamb",LockCntFixAmb);
    ini->WriteInteger("opt","fixcntholdamb",FixCntHoldAmb);
    ini->WriteFloat  ("opt","elmaskar",    ElMaskAR    );
    ini->WriteInteger("opt","outcntresetbias",OutCntResetAmb);
    ini->WriteFloat  ("opt","slipthres",   SlipThres   );
    ini->WriteFloat  ("opt","maxagediff",  MaxAgeDiff  );
    ini->WriteFloat  ("opt","rejectthres", RejectThres );
    ini->WriteInteger("opt","numiter",     NumIter     );
    ini->WriteInteger("opt","codesmooth",  CodeSmooth  );
    ini->WriteFloat  ("opt","baselinelen", BaseLine[0] );
    ini->WriteFloat  ("opt","baselinesig", BaseLine[1] );
    ini->WriteInteger("opt","baselineconst",BaseLineConst);
    
    ini->WriteInteger("opt","solformat",   SolFormat   );
    ini->WriteInteger("opt","timeformat",  TimeFormat  );
    ini->WriteInteger("opt","timedecimal", TimeDecimal );
    ini->WriteInteger("opt","latlonformat",LatLonFormat);
    ini->WriteString ("opt","fieldsep",    FieldSep    );
    ini->WriteInteger("opt","outputhead",  OutputHead  );
    ini->WriteInteger("opt","outputopt",   OutputOpt   );
    ini->WriteInteger("opt","outputdatum", OutputDatum );
    ini->WriteInteger("opt","outputheight",OutputHeight);
    ini->WriteInteger("opt","outputgeoid", OutputGeoid );
    ini->WriteInteger("opt","solstatic",   SolStatic   );
    ini->WriteInteger("opt","debugtrace",  DebugTrace  );
    ini->WriteInteger("opt","debugstatus", DebugStatus );
    
    ini->WriteFloat  ("opt","measerr1",    MeasErr1    );
    ini->WriteFloat  ("opt","measerr2",    MeasErr2    );
    ini->WriteFloat  ("opt","measerr3",    MeasErr3    );
    ini->WriteFloat  ("opt","measerr4",    MeasErr4    );
    ini->WriteFloat  ("opt","measerr5",    MeasErr5    );
    ini->WriteFloat  ("opt","satclkstab",  SatClkStab  );
    ini->WriteFloat  ("opt","prnoise1",    PrNoise1    );
    ini->WriteFloat  ("opt","prnoise2",    PrNoise2    );
    ini->WriteFloat  ("opt","prnoise3",    PrNoise3    );
    ini->WriteFloat  ("opt","prnoise4",    PrNoise4    );
    ini->WriteFloat  ("opt","prnoise5",    PrNoise5    );
    
    ini->WriteInteger("opt","rovpostype",  RovPosType  );
    ini->WriteInteger("opt","refpostype",  RefPosType  );
    ini->WriteFloat  ("opt","rovpos1",     RovPos[0]   );
    ini->WriteFloat  ("opt","rovpos2",     RovPos[1]   );
    ini->WriteFloat  ("opt","rovpos3",     RovPos[2]   );
    ini->WriteFloat  ("opt","refpos1",     RefPos[0]   );
    ini->WriteFloat  ("opt","refpos2",     RefPos[1]   );
    ini->WriteFloat  ("opt","refpos3",     RefPos[2]   );
    ini->WriteInteger("opt","rovantpcv",   RovAntPcv   );
    ini->WriteInteger("opt","refantpcv",   RefAntPcv   );
    ini->WriteString ("opt","rovant",      RovAnt      );
    ini->WriteString ("opt","refant",      RefAnt      );
    ini->WriteFloat  ("opt","rovante",     RovAntE     );
    ini->WriteFloat  ("opt","rovantn",     RovAntN     );
    ini->WriteFloat  ("opt","rovantu",     RovAntU     );
    ini->WriteFloat  ("opt","refante",     RefAntE     );
    ini->WriteFloat  ("opt","refantn",     RefAntN     );
    ini->WriteFloat  ("opt","refantu",     RefAntU     );
    
    ini->WriteString ("opt","antpcvfile",  AntPcvFile  );
    ini->WriteInteger("opt","intprefobs",  IntpRefObs  );
    ini->WriteInteger("opt","netrscorr",   NetRSCorr   );
    ini->WriteInteger("opt","satclkcorr",  SatClkCorr  );
    ini->WriteInteger("opt","sbascorr",    SbasCorr    );
    ini->WriteInteger("opt","sbascorr1",   SbasCorr1   );
    ini->WriteInteger("opt","sbascorr2",   SbasCorr2   );
    ini->WriteInteger("opt","sbascorr3",   SbasCorr3   );
    ini->WriteInteger("opt","sbascorr4",   SbasCorr4   );
    ini->WriteString ("opt","sbascorrfile",SbasCorrFile);
    ini->WriteString ("opt","precephfile", PrecEphFile );
    ini->WriteString ("opt","satpcvfile",  SatPcvFile  );
    ini->WriteString ("opt","staposfile",  StaPosFile  );
    ini->WriteString ("opt","geoiddatafile",GeoidDataFile);
    ini->WriteString ("opt","dcbfile",     DCBFile     );
    ini->WriteString ("opt","googleearthfile",GoogleEarthFile);
    
    for (p=RovList.c_str();*p;p++) {
        if ((p=strstr(p,"\r\n"))) strncpy(p,"@@",2); else break;
    }
    ini->WriteString ("opt","rovlist",     RovList);
    
    for (p=BaseList.c_str();*p;p++) {
        if ((p=strstr(p,"\r\n"))) strncpy(p,"@@",2); else break;
    }
    ini->WriteString ("opt","baselist",    BaseList);
    
    ini->WriteInteger("conv","timespan",   ConvDialog->TimeSpan  ->Checked  );
    ini->WriteString ("conv","timey1",     ConvDialog->TimeY1    ->Text     );
    ini->WriteString ("conv","timeh1",     ConvDialog->TimeH1    ->Text     );
    ini->WriteString ("conv","timey2",     ConvDialog->TimeY2    ->Text     );
    ini->WriteString ("conv","timeh2",     ConvDialog->TimeH2    ->Text     );
    ini->WriteInteger("conv","timeintf",   ConvDialog->TimeIntF  ->Checked  );
    ini->WriteString ("conv","timeint",    ConvDialog->TimeInt   ->Text     );
    ini->WriteInteger("conv","trackcolor", ConvDialog->TrackColor->ItemIndex);
    ini->WriteInteger("conv","pointcolor", ConvDialog->PointColor->ItemIndex);
    ini->WriteInteger("conv","outputalt",  ConvDialog->OutputAlt ->ItemIndex);
    ini->WriteInteger("conv","outputtime", ConvDialog->OutputTime->ItemIndex);
    ini->WriteInteger("conv","addoffset",  ConvDialog->AddOffset ->Checked  );
    ini->WriteString ("conv","offset1",    ConvDialog->Offset1   ->Text     );
    ini->WriteString ("conv","offset2",    ConvDialog->Offset2   ->Text     );
    ini->WriteString ("conv","offset3",    ConvDialog->Offset3   ->Text     );
    ini->WriteInteger("conv","compress",   ConvDialog->Compress  ->Checked  );
    
    ini->WriteInteger("viewer","color1",(int)TTextViewer::Color1  );
    ini->WriteInteger("viewer","color2",(int)TTextViewer::Color2  );
    ini->WriteString ("viewer","fontname",TTextViewer::FontD->Name);
    ini->WriteInteger("viewer","fontsize",TTextViewer::FontD->Size);
    delete ini;
}
//---------------------------------------------------------------------------
