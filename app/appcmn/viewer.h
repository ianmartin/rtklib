//---------------------------------------------------------------------------
#ifndef viewerH
#define viewerH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TTextViewer : public TForm
{
__published:
	TPanel *Panel1;
	TButton *BtnClose;
	TPanel *Panel2;
	TButton *BtnRead;
	TOpenDialog *OpenDialog;
	TButton *BtnOpt;
	TSpeedButton *BtnReload;
	TRichEdit *Text;
	TSaveDialog *SaveDialog;
	void __fastcall BtnCloseClick(TObject *Sender);
	void __fastcall BtnReadClick(TObject *Sender);
	void __fastcall BtnOptClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnReloadClick(TObject *Sender);
private:
	TStringList *TextBuff;
	AnsiString File;
	int SearchLine;
	void __fastcall UpdateText(void);
	TStringList *__fastcall TextFilter(TStringList *text);
public:
	int Option;
	static TColor Color1,Color2;
	static TFont *FontD;
	__fastcall TTextViewer(TComponent* Owner);
	void __fastcall Read(AnsiString file);
	void __fastcall Save(AnsiString file);
};
//---------------------------------------------------------------------------
extern PACKAGE TTextViewer *TextViewer;
//---------------------------------------------------------------------------
#endif
