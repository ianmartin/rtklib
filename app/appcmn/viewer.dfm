object TextViewer: TTextViewer
  Left = 0
  Top = 0
  Caption = 'TEXT VIEWER'
  ClientHeight = 415
  ClientWidth = 624
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 624
    Height = 23
    Align = alTop
    TabOrder = 0
    object Panel2: TPanel
      Left = 435
      Top = 1
      Width = 188
      Height = 21
      Align = alRight
      BevelOuter = bvNone
      TabOrder = 0
      object BtnReload: TSpeedButton
        Left = 0
        Top = 2
        Width = 19
        Height = 18
        Hint = 'Reload'
        Flat = True
        Glyph.Data = {
          DE000000424DDE0000000000000076000000280000000D0000000D0000000100
          0400000000006800000000000000000000001000000000000000000000000000
          8000008000000080800080000000800080008080000080808000C0C0C0000000
          FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00FFFFFFFFFFFF
          F000FFFFFFFFFFFFF000FFFF00007F0FF000FFF0FFFF000FF000FF7FFFF7000F
          F000FFFFFFFFF00FF000FF0FFFFFFF0FF000FF00FFFFFFFFF000FF0007FFFF7F
          F000FF000FFFF0FFF000FF0F70000FFFF000FFFFFFFFFFFFF000FFFFFFFFFFFF
          F000}
        OnClick = BtnReloadClick
      end
      object BtnClose: TButton
        Left = 132
        Top = 1
        Width = 55
        Height = 19
        Align = alCustom
        Caption = '&Close'
        TabOrder = 0
        OnClick = BtnCloseClick
      end
      object BtnRead: TButton
        Left = 20
        Top = 1
        Width = 55
        Height = 19
        Caption = '&Read...'
        TabOrder = 1
        OnClick = BtnReadClick
      end
      object BtnOpt: TButton
        Left = 76
        Top = 1
        Width = 55
        Height = 19
        Caption = '&Option...'
        TabOrder = 2
        OnClick = BtnOptClick
      end
    end
  end
  object Text: TRichEdit
    Left = 0
    Top = 23
    Width = 624
    Height = 392
    Align = alClient
    BevelInner = bvNone
    BevelOuter = bvNone
    BorderStyle = bsNone
    Lines.Strings = (
      'No Text')
    ReadOnly = True
    ScrollBars = ssBoth
    TabOrder = 1
    WordWrap = False
  end
  object OpenDialog: TOpenDialog
    Filter = 'All (*.*)|*.*|Text (*.txt)|*.txt'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    OptionsEx = [ofExNoPlacesBar]
    Left = 584
    Top = 368
  end
  object SaveDialog: TSaveDialog
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    OptionsEx = [ofExNoPlacesBar]
    Left = 556
    Top = 368
  end
end