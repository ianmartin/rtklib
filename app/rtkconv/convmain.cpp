//---------------------------------------------------------------------------
// rtkconv : rinex translator for ap
//
//          Copyright (C) 2007-2010 by T.TAKASU, All rights reserved.
//
// version : $Revision: 1.1 $ $Date: 2008/07/17 22:16:10 $
// history : 2008/07/14  1.0 new
//           2010/07/18  1.1 rtklib 2.4.0
//---------------------------------------------------------------------------
#include <vcl.h>
#include <vcl\inifiles.hpp>
#pragma hdrstop

#include "convmain.h"
#include "timedlg.h"
#include "confdlg.h"
#include "aboutdlg.h"
#include "startdlg.h"
#include "convopt.h"
#include "viewer.h"
#include "rtklib.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TMainWindow *MainWindow;

#define PRGNAME     "RTKCONV"             // program name
#define TRACEFILE   "rtkconv.trace"       // debug trace file

static int abortf=0;
//---------------------------------------------------------------------------
extern "C" {
extern int showmsg(char *format,...)
{
    static int i=0;
    va_list arg;
    char buff[1024];
    va_start(arg,format); vsprintf(buff,format,arg); va_end(arg);
    MainWindow->Message->Caption=buff;
    if (++i%100==0) Application->ProcessMessages();
    return abortf;
}
}
//---------------------------------------------------------------------------
__fastcall TMainWindow::TMainWindow(TComponent* Owner)
    : TForm(Owner)
{
    int i;
    DoubleBuffered=true;
    Format->Items->Clear();
    Format->Items->Add("Auto");
    for (i=0;i<=MAXRCVFMT;i++) {
        Format->Items->Add(formatstrs[i]);
    }
    Format->Items->Add(formatstrs[STRFMT_RINEX]);
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::FormCreate(TObject *Sender)
{
    ::DragAcceptFiles(Handle,true);
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::FormShow(TObject *Sender)
{
    TIniFile *ini=new TIniFile("rtkconv.ini");
    AnsiString s;
    
    TimeStartF ->Checked=ini->ReadInteger("set","timestartf",  0);
    TimeEndF   ->Checked=ini->ReadInteger("set","timeendf",    0);
    TimeIntF   ->Checked=ini->ReadInteger("set","timeintf",    0);
    TimeY1     ->Text   =ini->ReadString ("set","timey1",     "2000/01/01");
    TimeH1     ->Text   =ini->ReadString ("set","timeh1",     "00:00:00"  );
    TimeY2     ->Text   =ini->ReadString ("set","timey2",     "2000/01/01");
    TimeH2     ->Text   =ini->ReadString ("set","timeh2",     "00:00:00"  );
    TimeInt    ->Text   =ini->ReadString ("set","timeint",    "1");
    InFile     ->Text   =ini->ReadString ("set","infile",     "");
    OutFile1   ->Text   =ini->ReadString ("set","outfile1",   "");
    OutFile2   ->Text   =ini->ReadString ("set","outfile2",   "");
    OutFile3   ->Text   =ini->ReadString ("set","outfile3",   "");
    OutFile4   ->Text   =ini->ReadString ("set","outfile4",   "");
    OutFile5   ->Text   =ini->ReadString ("set","outfile5",   "");
    OutFileEna1->Checked=ini->ReadInteger("set","outfileena1", 1);
    OutFileEna2->Checked=ini->ReadInteger("set","outfileena2", 1);
    OutFileEna3->Checked=ini->ReadInteger("set","outfileena3", 1);
    OutFileEna4->Checked=ini->ReadInteger("set","outfileena4", 1);
    OutFileEna5->Checked=ini->ReadInteger("set","outfileena5", 1);
    Format   ->ItemIndex=ini->ReadInteger("set","format",      0);
    
    RnxVer              =ini->ReadInteger("opt","rnxver",      0);
    RunBy               =ini->ReadString ("opt","runby",      "");
    Marker              =ini->ReadString ("opt","marker",     "");
    MarkerNo            =ini->ReadString ("opt","markerno",   "");
    MarkerType          =ini->ReadString ("opt","markertype", "");
    Name[0]             =ini->ReadString ("opt","name0",      "");
    Name[1]             =ini->ReadString ("opt","name1",      "");
    Rec[0]              =ini->ReadString ("opt","rec0",       "");
    Rec[1]              =ini->ReadString ("opt","rec1",       "");
    Rec[2]              =ini->ReadString ("opt","rec2",       "");
    Ant[0]              =ini->ReadString ("opt","ant0",       "");
    Ant[1]              =ini->ReadString ("opt","ant1",       "");
    Ant[2]              =ini->ReadString ("opt","ant2",       "");
    AppPos[0]           =ini->ReadFloat  ("opt","apppos0",   0.0);
    AppPos[1]           =ini->ReadFloat  ("opt","apppos1",   0.0);
    AppPos[2]           =ini->ReadFloat  ("opt","apppos2",   0.0);
    AntDel[0]           =ini->ReadFloat  ("opt","antdel0",   0.0);
    AntDel[1]           =ini->ReadFloat  ("opt","antdel1",   0.0);
    AntDel[2]           =ini->ReadFloat  ("opt","antdel2",   0.0);
    Comment[0]          =ini->ReadString ("opt","comment0",   "");
    Comment[1]          =ini->ReadString ("opt","comment1",   "");
    RcvOption           =ini->ReadString ("opt","rcvoption",  "");
    NavSys              =ini->ReadInteger("opt","navsys",    0x3);
    ObsType             =ini->ReadInteger("opt","obstype",   0xF);
    FreqType            =ini->ReadInteger("opt","freqtype",  0x3);
    ExSats              =ini->ReadString ("opt","exsats",     "");
    TraceLevel          =ini->ReadInteger("opt","tracelevel",  0);
    
    TTextViewer::Color1=(TColor)ini->ReadInteger("viewer","color1",(int)clBlack);
    TTextViewer::Color2=(TColor)ini->ReadInteger("viewer","color2",(int)clWhite);
    TTextViewer::FontD=new TFont;
    TTextViewer::FontD->Name=ini->ReadString ("viewer","fontname","Courier New");
    TTextViewer::FontD->Size=ini->ReadInteger("viewer","fontsize",9);
    delete ini;
    
    UpdateEnable();
    Caption=s.sprintf("%s ver.%s",PRGNAME,VER_RTKLIB);
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::FormClose(TObject *Sender, TCloseAction &Action)
{
    TIniFile *ini=new TIniFile("rtkconv.ini");
    
    ini->WriteInteger("set","timestartf", TimeStartF ->Checked);
    ini->WriteInteger("set","timeendf",   TimeEndF   ->Checked);
    ini->WriteInteger("set","timeintf",   TimeIntF   ->Checked);
    ini->WriteString ("set","timey1",     TimeY1     ->Text);
    ini->WriteString ("set","timeh1",     TimeH1     ->Text);
    ini->WriteString ("set","timey2",     TimeY2     ->Text);
    ini->WriteString ("set","timeh2",     TimeH2     ->Text);
    ini->WriteString ("set","timeint",    TimeInt    ->Text);
    ini->WriteString ("set","infile",     InFile     ->Text);
    ini->WriteString ("set","outfile1",   OutFile1   ->Text);
    ini->WriteString ("set","outfile2",   OutFile2   ->Text);
    ini->WriteString ("set","outfile3",   OutFile3   ->Text);
    ini->WriteString ("set","outfile4",   OutFile4   ->Text);
    ini->WriteString ("set","outfile5",   OutFile5   ->Text);
    ini->WriteInteger("set","outfileena1",OutFileEna1->Checked);
    ini->WriteInteger("set","outfileena2",OutFileEna2->Checked);
    ini->WriteInteger("set","outfileena3",OutFileEna3->Checked);
    ini->WriteInteger("set","outfileena4",OutFileEna4->Checked);
    ini->WriteInteger("set","outfileena5",OutFileEna5->Checked);
    ini->WriteInteger("set","format",     Format     ->ItemIndex);
    
    ini->WriteInteger("opt","rnxver",     RnxVer);
    ini->WriteString ("opt","runby",      RunBy);
    ini->WriteString ("opt","marker",     Marker);
    ini->WriteString ("opt","markerno",   MarkerNo);
    ini->WriteString ("opt","markertype", MarkerType);
    ini->WriteString ("opt","name0",      Name[0]);
    ini->WriteString ("opt","name1",      Name[1]);
    ini->WriteString ("opt","rec0",       Rec[0]);
    ini->WriteString ("opt","rec1",       Rec[1]);
    ini->WriteString ("opt","rec2",       Rec[2]);
    ini->WriteString ("opt","ant0",       Ant[0]);
    ini->WriteString ("opt","ant1",       Ant[1]);
    ini->WriteString ("opt","ant2",       Ant[2]);
    ini->WriteFloat  ("opt","apppos0",    AppPos[0]);
    ini->WriteFloat  ("opt","apppos1",    AppPos[1]);
    ini->WriteFloat  ("opt","apppos2",    AppPos[2]);
    ini->WriteFloat  ("opt","antdel0",    AntDel[0]);
    ini->WriteFloat  ("opt","antdel1",    AntDel[1]);
    ini->WriteFloat  ("opt","antdel2",    AntDel[2]);
    ini->WriteString ("opt","comment0",   Comment[0]);
    ini->WriteString ("opt","comment1",   Comment[1]);
    ini->WriteString ("opt","rcvoption",  RcvOption);
    ini->WriteInteger("opt","navsys",     NavSys);
    ini->WriteInteger("opt","obstype",    ObsType);
    ini->WriteInteger("opt","freqtype",   FreqType);
    ini->WriteString ("opt","exsats",     ExSats);
    ini->WriteInteger("opt","tracelevel", TraceLevel);
    
    ini->WriteInteger("viewer","color1",  (int)TTextViewer::Color1);
    ini->WriteInteger("viewer","color2",  (int)TTextViewer::Color2);
    ini->WriteString ("viewer","fontname",TTextViewer::FontD->Name);
    ini->WriteInteger("viewer","fontsize",TTextViewer::FontD->Size);
    delete ini;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::SetOutFiles(AnsiString infile)
{
    int flag=0;
    char *ifile,ofile[1024],*p;
    ifile=infile.c_str();
    strcpy(ofile,ifile);
    for (p=ofile;*p;p++) {
        if (*p!='*'&&*p!='?') continue;
        *p='0'; flag=1;
    }
    if (flag) strcat(ofile,"_");
    if (!(p=strrchr(ofile,'.'))) p=ofile+strlen(ofile);
    strcpy(p,".obs" ); if (!strcmp(ofile,ifile)) strcat(ofile,"_"); OutFile1->Text=ofile;
    strcpy(p,".nav" ); if (!strcmp(ofile,ifile)) strcat(ofile,"_"); OutFile2->Text=ofile;
    strcpy(p,".gnav"); if (!strcmp(ofile,ifile)) strcat(ofile,"_"); OutFile3->Text=ofile;
    strcpy(p,".hnav"); if (!strcmp(ofile,ifile)) strcat(ofile,"_"); OutFile4->Text=ofile;
    strcpy(p,".sbs" ); if (!strcmp(ofile,ifile)) strcat(ofile,"_"); OutFile5->Text=ofile;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::DropFiles(TWMDropFiles msg)
{
    char *p,str[1024];
    
    if (DragQueryFile((HDROP)msg.Drop,0xFFFFFFFF,NULL,0)<=0) return;
    DragQueryFile((HDROP)msg.Drop,0,str,sizeof(str));
    InFile->Text=str;
    SetOutFiles(str);
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnPlotClick(TObject *Sender)
{
    AnsiString file1=OutFile1->Text,file2=OutFile2->Text,file3=OutFile3->Text;
    AnsiString cmd="rtkplot -r \""+file1+"\" \""+file2+"\" \""+file3+"\"";
    if (!ExecCmd(cmd)) Message->Caption="error : rtkplot execution";
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnOptionsClick(TObject *Sender)
{
    if (ConvOptDialog->ShowModal()!=mrOk) return;
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnConvertClick(TObject *Sender)
{
    ConvertFile();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnExitClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnTime1Click(TObject *Sender)
{
    gtime_t ts={0},te={0};
    double tint=0.0;
    GetTime(&ts,&te,&tint);
    TimeDialog->Time=ts;
    TimeDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnTime2Click(TObject *Sender)
{
    gtime_t ts={0},te={0};
    double tint=0.0;
    GetTime(&ts,&te,&tint);
    TimeDialog->Time=te;
    TimeDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnInFileClick(TObject *Sender)
{
    OpenDialog->Title="Input Receiver Log File";
    OpenDialog->FileName="";
    if (!OpenDialog->Execute()) return;
    InFile->Text=OpenDialog->FileName;
    SetOutFiles(OpenDialog->FileName);
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnOutFile1Click(TObject *Sender)
{
    OpenDialog2->Title="Output RINEX OBS File";
    OpenDialog2->FileName="";
    if (!OpenDialog2->Execute()) return;
    OutFile1->Text=OpenDialog2->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnOutFile2Click(TObject *Sender)
{
    OpenDialog2->Title="Output RINEX NAV File";
    OpenDialog2->FileName="";
    if (!OpenDialog2->Execute()) return;
    OutFile2->Text=OpenDialog2->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnOutFile3Click(TObject *Sender)
{
    OpenDialog2->Title="Output RINEX GNAV File";
    OpenDialog2->FileName="";
    if (!OpenDialog2->Execute()) return;
    OutFile2->Text=OpenDialog2->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnOutFile4Click(TObject *Sender)
{
    OpenDialog2->Title="Output SBAS Log File";
    OpenDialog2->FileName="";
    if (!OpenDialog2->Execute()) return;
    OutFile3->Text=OpenDialog2->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnOutFile5Click(TObject *Sender)
{
    OpenDialog2->Title="Output SBAS Log File";
    OpenDialog2->FileName="";
    if (!OpenDialog2->Execute()) return;
    OutFile3->Text=OpenDialog2->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnOutFileView1Click(TObject *Sender)
{
    TTextViewer *viewer=new TTextViewer(Application);
    viewer->Show();
    viewer->Read(OutFile1->Text);
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnOutFileView2Click(TObject *Sender)
{
    TTextViewer *viewer=new TTextViewer(Application);
    viewer->Show();
    viewer->Read(OutFile2->Text);
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnOutFileView3Click(TObject *Sender)
{
    TTextViewer *viewer=new TTextViewer(Application);
    viewer->Show();
    viewer->Read(OutFile3->Text);
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnOutFileView4Click(TObject *Sender)
{
    TTextViewer *viewer=new TTextViewer(Application);
    viewer->Show();
    viewer->Read(OutFile4->Text);
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnOutFileView5Click(TObject *Sender)
{
    TTextViewer *viewer=new TTextViewer(Application);
    viewer->Show();
    viewer->Read(OutFile5->Text);
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnAboutClick(TObject *Sender)
{
    AboutDialog->About=PRGNAME;
    AboutDialog->IconIndex=3;
    AboutDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::TimeStartFClick(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::TimeEndFClick(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::TimeIntFClick(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::OutFileEnaClick(TObject *Sender)
{
	UpdateEnable();	
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::InFileChange(TObject *Sender)
{
	SetOutFiles(InFile->Text);
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::GetTime(gtime_t *ts, gtime_t *te, double *tint)
{
    double eps[]={2000,1,1,0,0,0},epe[]={2000,1,1,0,0,0};
    
    if (TimeStartF->Checked) {
        sscanf(TimeY1->Text.c_str(),"%lf/%lf/%lf",eps,eps+1,eps+2);
        sscanf(TimeH1->Text.c_str(),"%lf:%lf:%lf",eps+3,eps+4,eps+5);
        *ts=epoch2time(eps);
    }
    if (TimeEndF->Checked) {
        sscanf(TimeY2->Text.c_str(),"%lf/%lf/%lf",epe,epe+1,epe+2);
        sscanf(TimeH2->Text.c_str(),"%lf:%lf:%lf",epe+3,epe+4,epe+5);
        *te=epoch2time(epe);
    }
    if (TimeIntF->Checked) {
        sscanf(TimeInt->Text.c_str(),"%lf",tint);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::TimeY1UDChangingEx(TObject *Sender,
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
void __fastcall TMainWindow::TimeH1UDChangingEx(TObject *Sender,
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
void __fastcall TMainWindow::TimeY2UDChangingEx(TObject *Sender,
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
void __fastcall TMainWindow::TimeH2UDChangingEx(TObject *Sender,
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
int __fastcall TMainWindow::ExecCmd(AnsiString cmd)
{
    PROCESS_INFORMATION info;
    STARTUPINFO si={0};
    si.cb=sizeof(si);
    char *p=cmd.c_str();
    if (!CreateProcess(NULL,p,NULL,NULL,false,0,NULL,NULL,&si,&info)) return 0;
    CloseHandle(info.hProcess);
    CloseHandle(info.hThread);
    return 1;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::UpdateEnable(void)
{
    TimeY1         ->Enabled=TimeStartF ->Checked;
    TimeH1         ->Enabled=TimeStartF ->Checked;
    TimeY1UD       ->Enabled=TimeStartF ->Checked;
    TimeH1UD       ->Enabled=TimeStartF ->Checked;
    BtnTime1       ->Enabled=TimeStartF ->Checked;
    TimeY2         ->Enabled=TimeEndF   ->Checked;
    TimeH2         ->Enabled=TimeEndF   ->Checked;
    TimeY2UD       ->Enabled=TimeEndF   ->Checked;
    TimeH2UD       ->Enabled=TimeEndF   ->Checked;
    BtnTime2       ->Enabled=TimeEndF   ->Checked;
    TimeInt        ->Enabled=TimeIntF   ->Checked;
    OutFileEna3    ->Enabled=!RnxVer;
    OutFileEna4    ->Enabled=!RnxVer;
    OutFile1       ->Enabled=OutFileEna1->Checked;
    OutFile2       ->Enabled=OutFileEna2->Checked;
    OutFile3       ->Enabled=OutFileEna3->Checked&&!RnxVer;
    OutFile4       ->Enabled=OutFileEna4->Checked&&!RnxVer;
    OutFile5       ->Enabled=OutFileEna5->Checked;
    BtnOutFile1    ->Enabled=OutFileEna1->Checked;
    BtnOutFile2    ->Enabled=OutFileEna2->Checked;
    BtnOutFile3    ->Enabled=OutFileEna3->Checked&&!RnxVer;
    BtnOutFile4    ->Enabled=OutFileEna4->Checked&&!RnxVer;
    BtnOutFile5    ->Enabled=OutFileEna5->Checked;
    BtnOutFileView1->Enabled=OutFileEna1->Checked;
    BtnOutFileView2->Enabled=OutFileEna2->Checked;
    BtnOutFileView3->Enabled=OutFileEna3->Checked&&!RnxVer;
    BtnOutFileView4->Enabled=OutFileEna4->Checked&&!RnxVer;
    BtnOutFileView5->Enabled=OutFileEna5->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::ConvertFile(void)
{
    rnxopt_t rnxopt={0};
    int i,format,sat;
    char file[1024],*ofile[5],ofile_[5][1024]={""},msg[256],*p;
    char tracefile[1024],buff[256],tstr[32];
    FILE *fp;
    
    if (BtnConvert->Caption=="Abort") {
        abortf=1;
        return;
    }
    for (i=0;i<5;i++) ofile[i]=ofile_[i];
    
    strcpy(file,InFile->Text.c_str());
    if (!(p=strrchr(file,'.'))) p=file;
    if (Format->ItemIndex==0) { // auto
        if      (!strcmp(p,".rtcm2")) format=STRFMT_RTCM2;
        else if (!strcmp(p,".rtcm3")) format=STRFMT_RTCM3;
        else if (!strcmp(p,".gps"  )) format=STRFMT_OEM4;
        else if (!strcmp(p,".ubx"  )) format=STRFMT_UBX;
        else if (!strcmp(p,".log"  )) format=STRFMT_SS2;
        else if (!strcmp(p,".bin"  )) format=STRFMT_CRES;
        else if (!strcmp(p,".obs"  )) format=STRFMT_RINEX;
        else if (!strcmp(p+3,"o"   )) format=STRFMT_RINEX;
        else if (!strcmp(p+3,"O"   )) format=STRFMT_RINEX;
        else {
            showmsg("file format can not be recognized");
            return;
        }
    }
    else {
        for (i=0;formatstrs[i];i++) {
            if (Format->Text==formatstrs[i]) break;
        }
        if (formatstrs[i]) format=i; else return;
    }
    rnxopt.rnxver=RnxVer?3:2;
    
    if (format==STRFMT_RTCM2||format==STRFMT_RTCM3) {
        if (StartDialog->ShowModal()!=mrOk) return;
        rnxopt.trtcm=StartDialog->Time;
    }
    if (OutFile1->Enabled&&OutFileEna1->Checked) strcpy(ofile[0],OutFile1->Text.c_str());
    if (OutFile2->Enabled&&OutFileEna2->Checked) strcpy(ofile[1],OutFile2->Text.c_str());
    if (OutFile3->Enabled&&OutFileEna3->Checked) strcpy(ofile[2],OutFile3->Text.c_str());
    if (OutFile4->Enabled&&OutFileEna4->Checked) strcpy(ofile[3],OutFile4->Text.c_str());
    if (OutFile5->Enabled&&OutFileEna5->Checked) strcpy(ofile[4],OutFile5->Text.c_str());
    for (i=0;i<5;i++) {
        if (!*ofile[i]||!(fp=fopen(ofile[i],"r"))) continue;
        fclose(fp);
        ConfDialog->Label2->Caption=ofile[i];
        if (ConfDialog->ShowModal()!=mrOk) return;
    }
    GetTime(&rnxopt.ts,&rnxopt.te,&rnxopt.tint);
    sprintf(rnxopt.prog,"%s %s",PRGNAME,VER_RTKLIB);
    strncpy(rnxopt.runby,RunBy.c_str(),31);
    strncpy(rnxopt.marker,Marker.c_str(),63);
    strncpy(rnxopt.markerno,MarkerNo.c_str(),31);
    strncpy(rnxopt.markertype,MarkerType.c_str(),31);
    for (i=0;i<2;i++) strncpy(rnxopt.name[i],Name[i].c_str(),31);
    for (i=0;i<3;i++) strncpy(rnxopt.rec [i],Rec [i].c_str(),31);
    for (i=0;i<3;i++) strncpy(rnxopt.ant [i],Ant [i].c_str(),31);
    for (i=0;i<3;i++) rnxopt.apppos[i]=AppPos[i];
    for (i=0;i<3;i++) rnxopt.antdel[i]=AntDel[i];
    sprintf(rnxopt.comment[0],"log: %-53.53s",file);
    sprintf(rnxopt.comment[1],"format: %s",formatstrs[format]);
    for (i=0;i<2;i++) strncpy(rnxopt.comment[i+2],Comment[i].c_str(),63);
    strncpy(rnxopt.rcvopt,RcvOption.c_str(),255);
    rnxopt.navsys=NavSys;
    rnxopt.obstype=ObsType;
    rnxopt.freqtype=FreqType;
    
    strcpy(buff,ExSats.c_str());
    for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
        if (!(sat=satid2no(p))) continue;
        rnxopt.exsats[sat-1]=1;
    }
    abortf=0;
    BtnConvert  ->Caption="Abort";
    Panel1      ->Enabled=false;
    Panel2      ->Enabled=false;
    BtnPlot     ->Enabled=false;
    BtnOptions  ->Enabled=false;
    BtnExit     ->Enabled=false;
    BtnArrow    ->Enabled=false;
    Format      ->Enabled=false;
    LabelInFile ->Enabled=false;
    LabelOutFile->Enabled=false;
    LabelFormat ->Enabled=false;
    Message     ->Caption="";
    
    if (TraceLevel>0) {
        strcpy(tracefile,InFile->Text.c_str());
        if (!(p=strrchr(tracefile,'.'))) p=tracefile+strlen(tracefile);
        strcpy(p,".trace");
        traceopen(tracefile);
        tracelevel(TraceLevel);
    }
    (void)convrnx(format,&rnxopt,file,ofile);
    
    if (TraceLevel>0) {
        traceclose();
    }
    BtnConvert  ->Caption="&Convert";
    Panel1      ->Enabled=true;
    Panel2      ->Enabled=true;
    BtnPlot     ->Enabled=true;
    BtnOptions  ->Enabled=true;
    BtnExit     ->Enabled=true;
    BtnArrow    ->Enabled=true;
    Format      ->Enabled=true;
    LabelInFile ->Enabled=true;
    LabelOutFile->Enabled=true;
    LabelFormat ->Enabled=true;
    
    if (!TimeStartF->Checked&&rnxopt.tstart.time!=0) {
        time2str(rnxopt.tstart,tstr,0);
        tstr[10]='\0';
        TimeY1->Text=tstr;
        TimeH1->Text=tstr+11;
    }
    if (!TimeEndF->Checked&&rnxopt.tend.time!=0) {
        time2str(rnxopt.tend,tstr,0);
        tstr[10]='\0';
        TimeY2->Text=tstr;
        TimeH2->Text=tstr+11;
    }
}
//---------------------------------------------------------------------------

