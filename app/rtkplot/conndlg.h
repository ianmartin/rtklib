//---------------------------------------------------------------------------

#ifndef conndlgH
#define conndlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

#define MAXHIST		10

//---------------------------------------------------------------------------
class TConnectDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TComboBox *SelStream;
	TButton *BtnOpt;
	TLabel *Label1;
	TLabel *Label2;
	TComboBox *SolFormat;
	TButton *BtnCmd;
	TLabel *Label3;
	TLabel *Label4;
	TComboBox *TimeFormS;
	TComboBox *DegFormS;
	TLabel *Label5;
	TLabel *Label6;
	TEdit *FieldSepS;
	TLabel *Label7;
	TEdit *TimeOutTimeE;
	TEdit *ReConnTimeE;
	TLabel *Label8;
	void __fastcall BtnOptClick(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnCmdClick(TObject *Sender);
	void __fastcall SelStreamChange(TObject *Sender);
	void __fastcall SolFormatChange(TObject *Sender);
private:
	void __fastcall SerialOpt(int opt);
	void __fastcall TcpOpt(int opt);
	void __fastcall FileOpt(int opt);
	void __fastcall UpdateEnable(void);
public:
	int Stream,Format,CmdEna[2],TimeForm,DegForm,TimeOutTime,ReConnTime;
	AnsiString Path,Paths[4],TcpHistory[MAXHIST],TcpMntpHist[MAXHIST];
	AnsiString Cmds[2],FieldSep;
	__fastcall TConnectDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TConnectDialog *ConnectDialog;
//---------------------------------------------------------------------------
#endif
