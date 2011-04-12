//---------------------------------------------------------------------------

#ifndef dlmainH
#define dlmainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE 管理のコンポーネント
	TPanel *Panel1;
	TLabel *Label1;
	TComboBox *DataTypeS;
	TListBox *DataListS;
	TPanel *Panel2;
	TEdit *AddressE;
	TLabel *Label3;
	TLabel *Label2;
	TEdit *YMD1;
	TEdit *YMD2;
	TUpDown *UDYMD1;
	TUpDown *UDYMD2;
	TEdit *HM2;
	TEdit *HM1;
	TUpDown *UDHM1;
	TUpDown *UDHM2;
	TLabel *Label4;
	TEdit *LoginUserE;
	TLabel *Label5;
	TEdit *PasswdE;
	TLabel *Label6;
	TEdit *LocalDirE;
	TButton *BtnLocalDir;
	TPanel *Panel3;
	TButton *BtnStations;
	TButton *BtnLog;
	TButton *BtnDownload;
	TButton *BtnExit;
	TPanel *Panel4;
	TSpeedButton *BtnTime1;
	TSpeedButton *BtnTime2;
	TLabel *Label7;
	TLabel *Label8;
	TLabel *Label9;
	TMemo *Memo1;
	TGroupBox *GroupBox1;
	TCheckBox *UnzipC;
	TCheckBox *SkipC;
	TCheckBox *AbortC;
	TCheckBox *ProxyC;
	TEdit *ProxyE;
	TButton *Button1;
	void __fastcall BtnExitClick(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
