//---------------------------------------------------------------------------
#ifndef mondlgH
#define mondlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TMonitorDialog : public TForm
{
__published:
	TStringGrid *Tbl;
	TPanel *Panel1;
	TPanel *Panel2;
	TButton *BtnClose;
	TTimer *Timer1;
	TLabel *Label;
	TComboBox *Type;
	TPaintBox *Console;
	TTimer *Timer2;
	TSpeedButton *BtnAsc;
	TSpeedButton *BtnHex;
	TScrollBar *Scroll;
	TSpeedButton *BtnPause;
	TSpeedButton *BtnDown;
	TSpeedButton *BtnClear;
	TComboBox *SelEph;
	TComboBox *SelStr;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall BtnCloseClick(TObject *Sender);
	void __fastcall TypeChange(TObject *Sender);
	void __fastcall Timer2Timer(TObject *Sender);
	void __fastcall ConsolePaint(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall ScrollChange(TObject *Sender);
	void __fastcall BtnDownClick(TObject *Sender);
	void __fastcall BtnClearClick(TObject *Sender);
private:
	int TypeF,ScrollPos,FontScale;
	TStringList *ConBuff;
	void __fastcall ClearTable(void);
	void __fastcall SetRtk(void);
	void __fastcall SetSat(void);
	void __fastcall SetEst(void);
	void __fastcall SetCov(void);
	void __fastcall SetObs(void);
	void __fastcall SetNav(void);
	void __fastcall SetGnav(void);
	void __fastcall SetStr(void);
	void __fastcall SetSbsMsg(void);
	void __fastcall SetSbsLong(void);
	void __fastcall SetSbsIono(void);
	void __fastcall SetSbsFast(void);
	void __fastcall SetSbsNav(void);
	void __fastcall SetRtcm(void);
	void __fastcall SetRtcmDgps(void);
	void __fastcall SetRtcmSsr(void);
	void __fastcall ShowRtk(void);
	void __fastcall ShowSat(void);
	void __fastcall ShowEst(void);
	void __fastcall ShowCov(void);
	void __fastcall ShowObs(void);
	void __fastcall ShowNav(void);
	void __fastcall ShowGnav(void);
	void __fastcall ShowStr(void);
	void __fastcall ShowSbsMsg(void);
	void __fastcall ShowSbsLong(void);
	void __fastcall ShowSbsIono(void);
	void __fastcall ShowSbsFast(void);
	void __fastcall ShowSbsNav(void);
	void __fastcall ShowRtcm(void);
	void __fastcall ShowRtcmDgps(void);
	void __fastcall ShowRtcmSsr(void);
	void __fastcall AddConsole(unsigned char *msg, int len, int mode);
	void __fastcall ViewConsole(void);
public:
	__fastcall TMonitorDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
#endif
