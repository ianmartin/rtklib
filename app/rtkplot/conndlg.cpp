//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "serioptdlg.h"
#include "fileoptdlg.h"
#include "tcpoptdlg.h"
#include "cmdoptdlg.h"
#include "conndlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConnectDialog *ConnectDialog;
//---------------------------------------------------------------------------
__fastcall TConnectDialog::TConnectDialog(TComponent* Owner)
	: TForm(Owner)
{
	Stream=Format=CmdEna[0]=CmdEna[1]=0;
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::FormShow(TObject *Sender)
{
	AnsiString s;
	int str[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE};
	for (int i=0;i<5;i++) {
		if (str[i]==Stream) SelStream->ItemIndex=i;
	}
	SolFormat->ItemIndex=Format;
	TimeFormS->ItemIndex=TimeForm;
	DegFormS ->ItemIndex=DegForm;
	FieldSepS->Text     =FieldSep;
	TimeOutTimeE->Text=s.sprintf("%d",TimeOutTime);
	ReConnTimeE ->Text=s.sprintf("%d",ReConnTime);
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::BtnOkClick(TObject *Sender)
{
	int str[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE};
	Stream=str[SelStream->ItemIndex];
	Format=SolFormat->ItemIndex;
	TimeForm=TimeFormS->ItemIndex;
	DegForm =DegFormS ->ItemIndex;
	FieldSep=FieldSepS->Text;
	TimeOutTime=TimeOutTimeE->Text.ToInt();
	ReConnTime =ReConnTimeE ->Text.ToInt();
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::BtnOptClick(TObject *Sender)
{
	switch (SelStream->ItemIndex) {
		case 0: SerialOpt(0); break;
		case 1: TcpOpt (1);   break;
		case 2: TcpOpt (0);   break;
		case 3: TcpOpt (3);   break;
		case 4: FileOpt(0);   break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::BtnCmdClick(TObject *Sender)
{
	CmdOptDialog->Cmds  [0]=Cmds  [0];
	CmdOptDialog->Cmds  [1]=Cmds  [1];
	CmdOptDialog->CmdEna[0]=CmdEna[0];
	CmdOptDialog->CmdEna[1]=CmdEna[1];
	if (CmdOptDialog->ShowModal()!=mrOk) return;
	Cmds  [0]=CmdOptDialog->Cmds  [0];
	Cmds  [1]=CmdOptDialog->Cmds  [1];
	CmdEna[0]=CmdOptDialog->CmdEna[0];
	CmdEna[1]=CmdOptDialog->CmdEna[1];
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::SelStreamChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::SolFormatChange(TObject *Sender)
{
	UpdateEnable();	
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::SerialOpt(int opt)
{
	SerialOptDialog->Path=Paths[0];
	SerialOptDialog->Opt=opt;
	if (SerialOptDialog->ShowModal()!=mrOk) return;
	Paths[0]=SerialOptDialog->Path;
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::TcpOpt(int opt)
{
	TcpOptDialog->Path=Paths[1];
	TcpOptDialog->Opt=opt;
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->History [i]=TcpHistory [i];
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->MntpHist[i]=TcpMntpHist[i];
	if (TcpOptDialog->ShowModal()!=mrOk) return;
	Paths[1]=TcpOptDialog->Path;
	for (int i=0;i<MAXHIST;i++) TcpHistory [i]=TcpOptDialog->History [i];
	for (int i=0;i<MAXHIST;i++) TcpMntpHist[i]=TcpOptDialog->MntpHist[i];
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::FileOpt(int opt)
{
	FileOptDialog->Path=Paths[2];
	FileOptDialog->Opt=opt;
	if (FileOptDialog->ShowModal()!=mrOk) return;
	Paths[2]=FileOptDialog->Path;
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::UpdateEnable(void)
{
	BtnCmd      ->Enabled=SelStream->ItemIndex==0;
	TimeFormS   ->Enabled=SolFormat->ItemIndex!=3;
	DegFormS    ->Enabled=SolFormat->ItemIndex==0;
	FieldSepS   ->Enabled=SolFormat->ItemIndex!=3;
	Label5      ->Enabled=SolFormat->ItemIndex!=3;
	Label6      ->Enabled=SolFormat->ItemIndex==0;
	Label7      ->Enabled=SolFormat->ItemIndex!=3;
	Label8      ->Enabled=1<=SelStream->ItemIndex&&SelStream->ItemIndex<=3;
	TimeOutTimeE->Enabled=1<=SelStream->ItemIndex&&SelStream->ItemIndex<=3;
	ReConnTimeE ->Enabled=1<=SelStream->ItemIndex&&SelStream->ItemIndex<=3;
}
//---------------------------------------------------------------------------
