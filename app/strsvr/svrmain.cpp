//---------------------------------------------------------------------------
#include <vcl.h>
#include <vcl\inifiles.hpp>
#include <mmsystem.h>
#include <stdio.h>
#pragma hdrstop

#include "rtklib.h"
#include "svroptdlg.h"
#include "serioptdlg.h"
#include "fileoptdlg.h"
#include "tcpoptdlg.h"
#include "ftpoptdlg.h"
#include "confdlg.h"
#include "cmdoptdlg.h"
#include "aboutdlg.h"
#include "refdlg.h"
#include "console.h"
#include "svrmain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

#define PRGNAME		"STRSVR"		// program name
#define TRACEFILE	"strsvr.trace"	// debug trace file
#define CLORANGE    (TColor)0x00AAFF

static strsvr_t strsvr;

//---------------------------------------------------------------------------
static void num2cnum(int num, char *str)
{
	char buff[256],*p=buff,*q=str;
	int i,n;
	n=sprintf(buff,"%d",num);
	for (i=0;i<n;i++) {
		*q++=*p++;
		if ((n-i-1)%3==0&&i<n-1) *q++=',';
	}
	*q='\0';
}
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
	AnsiString s;
	Caption=s.sprintf("STRSVR ver.%s",VER_RTKLIB);
	strsvrinit(&strsvr,3);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
	TIniFile *ini=new TIniFile("strsvr.ini");
	AnsiString s;
	int optdef[]={10000,10000,1000,32768,10,0};
	Input->ItemIndex=ini->ReadInteger("set","input",0);
	Output1->ItemIndex=ini->ReadInteger("set","output1",0);
	Output2->ItemIndex=ini->ReadInteger("set","output2",0);
	Output3->ItemIndex=ini->ReadInteger("set","output3",0);
	TraceLevel=ini->ReadInteger("set","tracelevel",0);
	NmeaReq=ini->ReadInteger("set","nmeareq",0);
	for (int i=0;i<6;i++) {
		SvrOpt[i]=ini->ReadInteger("set",s.sprintf("svropt_%d",i),optdef[i]);
	}
	for (int i=0;i<3;i++) {
		NmeaPos[i]=ini->ReadFloat("set",s.sprintf("nmeapos_%d",i),0.0);
	}
	for (int i=0;i<2;i++) {
		CmdEna[i]=ini->ReadInteger("set",s.sprintf("cmdena_%d",i),1);
	}
	for (int i=0;i<4;i++) for (int j=0;j<4;j++) {
		Paths[i][j]=ini->ReadString("path",s.sprintf("path_%d_%d",i,j),"");
	}
	for (int i=0;i<2;i++) {
		Cmds[i]=ini->ReadString("serial",s.sprintf("cmd_%d",i),"");
		for (char *p=Cmds[i].c_str();*p;p++) {
			if ((p=strstr(p,"@@"))) strncpy(p,"\r\n",2); else break;
		}
	}
	for (int i=0;i<MAXHIST;i++) {
		TcpHistory[i]=ini->ReadString("tcpopt",s.sprintf("history%d",i),"");
	}
	for (int i=0;i<MAXHIST;i++) {
		TcpMntpHist[i]=ini->ReadString("tcpopt",s.sprintf("mntphist%d",i),"");
	}
	StaPosFile=ini->ReadString("stapos","staposfile","");
	ExeDirectory=ini->ReadString("dirs","exedirectory","");
	LocalDirectory=ini->ReadString("dirs","localdirectory","");
	delete ini;
	
	UpdateEnable();
	SetTrayIcon(0);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	TIniFile *ini=new TIniFile("strsvr.ini");
	AnsiString s;
	ini->WriteInteger("set","input",Input->ItemIndex);
	ini->WriteInteger("set","output1",Output1->ItemIndex);
	ini->WriteInteger("set","output2",Output2->ItemIndex);
	ini->WriteInteger("set","output3",Output3->ItemIndex);
	ini->WriteInteger("set","tracelevel",TraceLevel);
	ini->WriteInteger("set","nmeareq",NmeaReq);
	for (int i=0;i<6;i++) {
		ini->WriteInteger("set",s.sprintf("svropt_%d",i),SvrOpt[i]);
	}
	for (int i=0;i<3;i++) {
		ini->WriteFloat("set",s.sprintf("nmeapos_%d",i),NmeaPos[i]);
	}
	for (int i=0;i<2;i++) {
		ini->WriteInteger("set",s.sprintf("cmdena_%d",i),CmdEna[i]);
	}
	for (int i=0;i<4;i++) for (int j=0;j<4;j++) {
		ini->WriteString("path",s.sprintf("path_%d_%d",i,j),Paths[i][j]);
	}
	for (int i=0;i<2;i++) {
		for (char *p=Cmds[i].c_str();*p;p++) {
			if ((p=strstr(p,"\r\n"))) strncpy(p,"@@",2); else break;
		}
		ini->WriteString("serial",s.sprintf("cmd_%d",i),Cmds[i]);
	}
	for (int i=0;i<MAXHIST;i++) {
		ini->WriteString("tcpopt",s.sprintf("history%d",i),TcpOptDialog->History[i]);
	}
	for (int i=0;i<MAXHIST;i++) {
		ini->WriteString("tcpopt",s.sprintf("mntphist%d",i),TcpOptDialog->MntpHist[i]);
	}
	ini->WriteString("stapos","staposfile",StaPosFile);
	ini->WriteString("dirs","exedirectory",ExeDirectory);
	ini->WriteString("dirs","localdirectory",LocalDirectory);
	delete ini;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnExitClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnStartClick(TObject *Sender)
{
	SvrStart();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnStopClick(TObject *Sender)
{
	SvrStop();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnOptClick(TObject *Sender)
{
	for (int i=0;i<6;i++) SvrOptDialog->SvrOpt[i]=SvrOpt[i];
	for (int i=0;i<3;i++) SvrOptDialog->NmeaPos[i]=NmeaPos[i];
	SvrOptDialog->TraceLevel=TraceLevel;
	SvrOptDialog->NmeaReq=NmeaReq;
	SvrOptDialog->StaPosFile=StaPosFile;
	SvrOptDialog->ExeDirectory=ExeDirectory;
	SvrOptDialog->LocalDirectory=LocalDirectory;
	if (SvrOptDialog->ShowModal()!=mrOk) return;
	for (int i=0;i<6;i++) SvrOpt[i]=SvrOptDialog->SvrOpt[i];
	for (int i=0;i<3;i++) NmeaPos[i]=SvrOptDialog->NmeaPos[i];
	TraceLevel=SvrOptDialog->TraceLevel;
	NmeaReq=SvrOptDialog->NmeaReq;
	StaPosFile=SvrOptDialog->StaPosFile;
	ExeDirectory=SvrOptDialog->ExeDirectory;
	LocalDirectory=SvrOptDialog->LocalDirectory;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnCmdClick(TObject *Sender)
{
	CmdOptDialog->Cmds[0]=Cmds[0];
	CmdOptDialog->Cmds[1]=Cmds[1];
	CmdOptDialog->CmdEna[0]=CmdEna[0];
	CmdOptDialog->CmdEna[1]=CmdEna[1];
	if (CmdOptDialog->ShowModal()!=mrOk) return;
	Cmds[0]  =CmdOptDialog->Cmds[0];
	Cmds[1]  =CmdOptDialog->Cmds[1];
	CmdEna[0]=CmdOptDialog->CmdEna[0];
	CmdEna[1]=CmdOptDialog->CmdEna[1];
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnInputClick(TObject *Sender)
{
	switch (Input->ItemIndex) {
		case 0: SerialOpt(0,0); break;
		case 1: TcpOpt(0,1); break;
		case 2: TcpOpt(0,0); break;
		case 3: TcpOpt(0,3); break;
		case 4: FileOpt(0,0); break;
		case 5: FtpOpt(0,0); break;
		case 6: FtpOpt(0,1); break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnOutput1Click(TObject *Sender)
{
	switch (Output1->ItemIndex) {
		case 1: SerialOpt(1,0); break;
		case 2: TcpOpt(1,1); break;
		case 3: TcpOpt(1,0); break;
		case 4: TcpOpt(1,2); break;
		case 5: FileOpt(1,1); break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnOutput2Click(TObject *Sender)
{
	switch (Output2->ItemIndex) {
		case 1: SerialOpt(2,0); break;
		case 2: TcpOpt(2,1); break;
		case 3: TcpOpt(2,0); break;
		case 4: TcpOpt(2,2); break;
		case 5: FileOpt(2,1); break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnOutput3Click(TObject *Sender)
{
	switch (Output3->ItemIndex) {
		case 1: SerialOpt(3,0); break;
		case 2: TcpOpt(3,1); break;
		case 3: TcpOpt(3,0); break;
		case 4: TcpOpt(3,2); break;
		case 5: FileOpt(3,1); break; 
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnAboutClick(TObject *Sender)
{
	AboutDialog->About=PRGNAME;
	AboutDialog->IconIndex=6;
	AboutDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnTaskIconClick(TObject *Sender)
{
	Visible=false;
	TrayIcon->Visible=true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TrayIconDblClick(TObject *Sender)
{
	Visible=true;
	TrayIcon->Visible=false;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TrayIconMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if (Shift.Contains(ssRight)) PopupMenu->Popup(X,Y);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuExpandClick(TObject *Sender)
{
	Visible=true;
	TrayIcon->Visible=false;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuStartClick(TObject *Sender)
{
	SvrStart();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuStopClick(TObject *Sender)
{
	SvrStop();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuExitClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnStrMonClick(TObject *Sender)
{
	Console->Caption="Input Monitor";
	Console->Show();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::InputChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Output1Change(TObject *Sender)
{
	UpdateEnable();	
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Output2Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Output3Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Timer1Timer(TObject *Sender)
{
	TColor color[]={clRed,clBtnFace,CLORANGE,clGreen,clLime};
	TPanel *e0[]={IndInput,IndOutput1,IndOutput2,IndOutput3};
	TLabel *e1[]={InputByte,Output1Byte,Output2Byte,Output3Byte};
	TLabel *e2[]={InputBps,Output1Bps,Output2Bps,Output3Bps};
	AnsiString s;
	gtime_t time=utc2gpst(timeget());
	int stat[4]={0},byte[4]={0},bps[4]={0};
	char msg[MAXSTRMSG*4]="",s1[256],s2[256];
	double ctime,t[4];
	
	strsvrstat(&strsvr,stat,byte,bps,msg);
	for (int i=0;i<4;i++) {
		num2cnum(byte[i],s1);
		num2cnum(bps[i],s2);
		e0[i]->Color=color[stat[i]+1];
		e1[i]->Caption=s1;
		e2[i]->Caption=s2;
	}
	Progress->Position=!stat[0]?0:(int)(fmod(byte[0]/500.0,101.0)+0.5);
	
	time2str(time,s1,0);
	Time->Caption=s.sprintf("%s GPST",s1);
	
	if (Panel1->Enabled) {
		ctime=timediff(EndTime,StartTime);
	}
	else {
		ctime=timediff(time,StartTime);
	}
	ctime=floor(ctime);
	t[0]=floor(ctime/86400.0); ctime-=t[0]*86400.0;
	t[1]=floor(ctime/3600.0 ); ctime-=t[1]*3600.0;
	t[2]=floor(ctime/60.0   ); ctime-=t[2]*60.0;
	t[3]=ctime;
	ConTime->Caption=s.sprintf("%.0fd %02.0f:%02.0f:%02.0f",t[0],t[1],t[2],t[3]);
	
	num2cnum(byte[0],s1); num2cnum(bps[0],s2);
	TrayIcon->Hint=s.sprintf("%s bytes %s bps",s1,s2);
	SetTrayIcon(stat[0]<=0?0:(stat[0]==3?2:1));
	
	Message->Caption=msg;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SvrStart(void)
{
	int itype[]={
		STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP
	};
	int otype[]={
		STR_NONE,STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE
	};
	int ip[]={0,1,1,1,2,3,3},strs[4]={0},opt[6]={0};
	char *paths[4],*cmd=NULL,filepath[1024],*p;
	FILE *fp;
	
	if (TraceLevel>0) {
		traceopen(TRACEFILE);
		tracelevel(TraceLevel);
	}
	strs[0]=itype[Input->ItemIndex];
	strs[1]=otype[Output1->ItemIndex];
	strs[2]=otype[Output2->ItemIndex];
	strs[3]=otype[Output3->ItemIndex];
	
	paths[0]=Paths[0][ip[Input->ItemIndex]].c_str();
	paths[1]=!Output1->ItemIndex?"":Paths[1][ip[Output1->ItemIndex-1]].c_str();
	paths[2]=!Output2->ItemIndex?"":Paths[2][ip[Output2->ItemIndex-1]].c_str();
	paths[3]=!Output3->ItemIndex?"":Paths[3][ip[Output3->ItemIndex-1]].c_str();
	
	if (CmdEna[0]) cmd=MainForm->Cmds[0].c_str();
	
	for (int i=0;i<5;i++) {
		opt[i]=SvrOpt[i];
	}
	opt[5]=NmeaReq?SvrOpt[5]:0;
	
	for (int i=1;i<4;i++) {
		if (strs[i]!=STR_FILE) continue;
		strcpy(filepath,paths[i]);
		if (strstr(filepath,"::A")) continue;
		if ((p=strstr(filepath,"::"))) *p='\0';
		if (!(fp=fopen(filepath,"r"))) continue;
		fclose(fp);
		ConfDialog->Label2->Caption=filepath;
		if (ConfDialog->ShowModal()!=mrOk) return;
	}
	strsetdir(LocalDirectory.c_str());
	
	if (!strsvrstart(&strsvr,opt,strs,paths,cmd,NmeaPos)) return;
	
	StartTime=utc2gpst(timeget());
	Panel1->Enabled=false;
	BtnStart->Enabled=false;
	BtnStop->Enabled=true;
	BtnOpt->Enabled=false;
	BtnExit->Enabled=false;
	BtnInput->Enabled=false;
	BtnOutput1->Enabled=false;
	BtnOutput2->Enabled=false;
	BtnOutput3->Enabled=false;
	MenuStart->Enabled=false;
	MenuStop->Enabled=true;
	MenuExit->Enabled=false;
	SetTrayIcon(1);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SvrStop(void)
{
	char *cmd=NULL;
	
	if (CmdEna[1]) cmd=Cmds[1].c_str();
	
	strsvrstop(&strsvr,cmd);
	
	EndTime=utc2gpst(timeget());
	Panel1->Enabled=true;
	BtnStart->Enabled=true;
	BtnStop->Enabled=false;
	BtnOpt->Enabled=true;
	BtnExit->Enabled=true;
	BtnInput->Enabled=true;
	BtnOutput1->Enabled=true;
	BtnOutput2->Enabled=true;
	BtnOutput3->Enabled=true;
	MenuStart->Enabled=true;
	MenuStop->Enabled=false;
	MenuExit->Enabled=true;
	SetTrayIcon(0);
	
	if (TraceLevel>0) traceclose();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Timer2Timer(TObject *Sender)
{
	unsigned char *msg;
	int len;
	
	lock(&strsvr.lock);
	
	len=strsvr.npb;
	if (len>0&&(msg=(unsigned char *)malloc(len))) {
		memcpy(msg,strsvr.pbuf,len);
		strsvr.npb=0;
	}
	unlock(&strsvr.lock);
	
	if (len<=0||!msg) return;
	
	Console->AddMsg(msg,len);
	
	free(msg);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SerialOpt(int index, int opt)
{
	SerialOptDialog->Path=Paths[index][0];
	SerialOptDialog->Opt=opt;
	if (SerialOptDialog->ShowModal()!=mrOk) return;
	Paths[index][0]=SerialOptDialog->Path;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TcpOpt(int index, int opt)
{
	TcpOptDialog->Path=Paths[index][1];
	TcpOptDialog->Opt=opt;
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->History[i]=TcpHistory[i];
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->MntpHist[i]=TcpMntpHist[i];
	if (TcpOptDialog->ShowModal()!=mrOk) return;
	Paths[index][1]=TcpOptDialog->Path;
	for (int i=0;i<MAXHIST;i++) TcpHistory[i]=TcpOptDialog->History[i];
	for (int i=0;i<MAXHIST;i++) TcpMntpHist[i]=TcpOptDialog->MntpHist[i];
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FileOpt(int index, int opt)
{
	FileOptDialog->Path=Paths[index][2];
	FileOptDialog->Opt=opt;
	if (FileOptDialog->ShowModal()!=mrOk) return;
	Paths[index][2]=FileOptDialog->Path;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FtpOpt(int index, int opt)
{
	FtpOptDialog->Path=Paths[index][3];
	FtpOptDialog->Opt=opt;
	if (FtpOptDialog->ShowModal()!=mrOk) return;
	Paths[index][3]=FtpOptDialog->Path;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::UpdateEnable(void)
{
	BtnCmd->Enabled=Input->ItemIndex==0;
	BtnOutput1->Enabled=Output1->ItemIndex>0;
	BtnOutput2->Enabled=Output2->ItemIndex>0;
	BtnOutput3->Enabled=Output3->ItemIndex>0;
	Output1Byte->Font->Color=Output1->ItemIndex>0?clBlack:clGray;
	Output2Byte->Font->Color=Output2->ItemIndex>0?clBlack:clGray;
	Output3Byte->Font->Color=Output3->ItemIndex>0?clBlack:clGray;
	Output1Bps->Font->Color=Output1->ItemIndex>0?clBlack:clGray;
	Output2Bps->Font->Color=Output2->ItemIndex>0?clBlack:clGray;
	Output3Bps->Font->Color=Output3->ItemIndex>0?clBlack:clGray;
	LabelOutput1->Font->Color=Output1->ItemIndex>0?clBlack:clGray;
	LabelOutput2->Font->Color=Output2->ItemIndex>0?clBlack:clGray;
	LabelOutput3->Font->Color=Output3->ItemIndex>0?clBlack:clGray;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SetTrayIcon(int index)
{
	TIcon *icon=new TIcon;
	ImageList->GetIcon(index,icon);
	TrayIcon->Icon=icon;
	delete icon;
}
//---------------------------------------------------------------------------

