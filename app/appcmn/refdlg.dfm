object RefDialog: TRefDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'Stations'
  ClientHeight = 445
  ClientWidth = 433
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object StaList: TStringGrid
    Left = 0
    Top = 0
    Width = 433
    Height = 420
    Margins.Top = 1
    Margins.Bottom = 1
    Align = alClient
    ColCount = 7
    DefaultRowHeight = 16
    FixedCols = 0
    RowCount = 2
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    GridLineWidth = 0
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goColSizing, goRowSelect]
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 0
    OnDblClick = StaListDblClick
    OnMouseDown = StaListMouseDown
    ColWidths = (
      31
      78
      83
      64
      39
      70
      69)
  end
  object Panel1: TPanel
    Left = 0
    Top = 420
    Width = 433
    Height = 25
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 1
    object BtnLoad: TButton
      Left = 4
      Top = 2
      Width = 67
      Height = 21
      Caption = '&Load'
      TabOrder = 0
      OnClick = BtnLoadClick
    end
    object Panel2: TPanel
      Left = 294
      Top = 0
      Width = 139
      Height = 25
      Align = alRight
      BevelOuter = bvNone
      TabOrder = 1
      object BtnOK: TButton
        Left = 0
        Top = 2
        Width = 67
        Height = 21
        Caption = '&OK'
        ModalResult = 1
        TabOrder = 0
        OnClick = BtnOKClick
      end
      object BtnCancel: TButton
        Left = 68
        Top = 2
        Width = 67
        Height = 21
        Caption = '&Cancel'
        ModalResult = 2
        TabOrder = 1
      end
    end
    object FormatS: TComboBox
      Left = 72
      Top = 2
      Width = 83
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 2
      Text = 'Lat/Lon/Hgt'
      OnChange = FormatSChange
      Items.Strings = (
        'Lat/Lon/Hgt'
        'SINEX')
    end
  end
  object OpenDialog: TOpenDialog
    Filter = 'All (*.*)|*.*'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    OptionsEx = [ofExNoPlacesBar]
    Left = 260
    Top = 414
  end
end
