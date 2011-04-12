object MainWindow: TMainWindow
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'RTKCONV'
  ClientHeight = 276
  ClientWidth = 416
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object BtnConvert: TButton
    Left = 258
    Top = 252
    Width = 77
    Height = 23
    Caption = '&Convert'
    TabOrder = 0
    OnClick = BtnConvertClick
  end
  object BtnExit: TButton
    Left = 337
    Top = 252
    Width = 77
    Height = 23
    Caption = '&Exit'
    TabOrder = 2
    OnClick = BtnExitClick
  end
  object BtnOptions: TButton
    Left = 84
    Top = 252
    Width = 77
    Height = 23
    Caption = '&Options...'
    TabOrder = 1
    OnClick = BtnOptionsClick
  end
  object Panel3: TPanel
    Left = 2
    Top = 226
    Width = 413
    Height = 26
    BevelInner = bvRaised
    BevelOuter = bvLowered
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clGray
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 5
    object Message: TLabel
      Left = 4
      Top = 6
      Width = 401
      Height = 13
      Alignment = taCenter
      AutoSize = False
    end
    object BtnAbout: TSpeedButton
      Left = 394
      Top = 4
      Width = 15
      Height = 18
      Caption = '?'
      Flat = True
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clGray
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      OnClick = BtnAboutClick
    end
  end
  object Panel2: TPanel
    Left = 2
    Top = 46
    Width = 413
    Height = 179
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 4
    object LabelInFile: TLabel
      Left = 8
      Top = 4
      Width = 128
      Height = 13
      Caption = 'RTCM, RCV RAW or RINEX'
    end
    object LabelOutFile: TLabel
      Left = 6
      Top = 52
      Width = 183
      Height = 13
      Caption = 'RINEX OBS/NAV/GNAV/HNAV and LOG'
    end
    object BtnArrow: TSpeedButton
      Left = 200
      Top = 42
      Width = 24
      Height = 24
      AllowAllUp = True
      Flat = True
      Glyph.Data = {
        36030000424D3603000000000000360000002800000010000000100000000100
        1800000000000003000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0C0C0C0C0C0FFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0C0C000
        0000000000C0C0C0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFC0C0C0000000FFFFFFFFFFFF000000C0C0C0FFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0C0C0000000FFFFFFFF
        FFFFFFFFFFFFFFFF000000C0C0C0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFC0C0C0000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000C0C0
        C0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0C0C0000000FFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFF000000C0C0C0FFFFFFFFFFFFFFFFFFC0C0C0
        000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF000000C0C0C0FFFFFFC0C0C0000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000C0C0C0000000000000
        000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0000000000
        00000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFF000000000000000000000000000000000000000000000000FFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF}
      OnClick = BtnConvertClick
    end
    object LabelFormat: TLabel
      Left = 278
      Top = 46
      Width = 34
      Height = 13
      Caption = 'Format'
    end
    object BtnOutFileView1: TSpeedButton
      Left = 374
      Top = 68
      Width = 17
      Height = 20
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      OnClick = BtnOutFileView1Click
    end
    object BtnOutFileView2: TSpeedButton
      Left = 374
      Top = 90
      Width = 17
      Height = 20
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      OnClick = BtnOutFileView2Click
    end
    object BtnOutFileView4: TSpeedButton
      Left = 374
      Top = 134
      Width = 17
      Height = 20
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      OnClick = BtnOutFileView4Click
    end
    object BtnOutFileView3: TSpeedButton
      Left = 374
      Top = 112
      Width = 17
      Height = 20
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      OnClick = BtnOutFileView3Click
    end
    object BtnOutFileView5: TSpeedButton
      Left = 374
      Top = 156
      Width = 17
      Height = 20
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      OnClick = BtnOutFileView5Click
    end
    object OutFileEna4: TCheckBox
      Left = 4
      Top = 136
      Width = 20
      Height = 17
      TabOrder = 6
      OnClick = OutFileEnaClick
    end
    object OutFileEna2: TCheckBox
      Left = 4
      Top = 92
      Width = 20
      Height = 17
      Checked = True
      State = cbChecked
      TabOrder = 4
      OnClick = OutFileEnaClick
    end
    object OutFileEna1: TCheckBox
      Left = 4
      Top = 70
      Width = 20
      Height = 17
      Checked = True
      State = cbChecked
      TabOrder = 3
      OnClick = OutFileEnaClick
    end
    object InFile: TEdit
      Left = 4
      Top = 20
      Width = 387
      Height = 21
      TabOrder = 0
      OnChange = InFileChange
    end
    object BtnInFile: TButton
      Left = 392
      Top = 20
      Width = 17
      Height = 20
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnClick = BtnInFileClick
    end
    object OutFile1: TEdit
      Left = 18
      Top = 68
      Width = 355
      Height = 21
      TabOrder = 7
    end
    object OutFile2: TEdit
      Left = 18
      Top = 90
      Width = 355
      Height = 21
      TabOrder = 8
    end
    object OutFile4: TEdit
      Left = 18
      Top = 134
      Width = 355
      Height = 21
      TabOrder = 10
    end
    object Format: TComboBox
      Left = 318
      Top = 42
      Width = 93
      Height = 21
      Style = csDropDownList
      DropDownCount = 20
      ItemHeight = 13
      TabOrder = 2
    end
    object BtnOutFile4: TButton
      Left = 392
      Top = 134
      Width = 17
      Height = 20
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 14
      OnClick = BtnOutFile4Click
    end
    object BtnOutFile2: TButton
      Left = 392
      Top = 90
      Width = 17
      Height = 20
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 12
      OnClick = BtnOutFile2Click
    end
    object BtnOutFile1: TButton
      Left = 392
      Top = 68
      Width = 17
      Height = 20
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 11
      OnClick = BtnOutFile1Click
    end
    object OutFileEna3: TCheckBox
      Left = 4
      Top = 114
      Width = 20
      Height = 17
      TabOrder = 5
      OnClick = OutFileEnaClick
    end
    object OutFile3: TEdit
      Left = 18
      Top = 112
      Width = 355
      Height = 21
      TabOrder = 9
    end
    object BtnOutFile3: TButton
      Left = 392
      Top = 112
      Width = 17
      Height = 20
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 13
      OnClick = BtnOutFile3Click
    end
    object OutFileEna5: TCheckBox
      Left = 4
      Top = 158
      Width = 20
      Height = 17
      TabOrder = 15
      OnClick = OutFileEnaClick
    end
    object OutFile5: TEdit
      Left = 18
      Top = 156
      Width = 355
      Height = 21
      TabOrder = 16
    end
    object BtnOutFile5: TButton
      Left = 392
      Top = 156
      Width = 17
      Height = 20
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 17
      OnClick = BtnOutFile5Click
    end
  end
  object Panel1: TPanel
    Left = 2
    Top = 2
    Width = 413
    Height = 43
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 3
    object LabelTimeInt: TLabel
      Left = 398
      Top = 22
      Width = 5
      Height = 13
      Caption = 's'
    end
    object BtnTime1: TSpeedButton
      Left = 128
      Top = 2
      Width = 17
      Height = 15
      Caption = '?'
      Flat = True
      OnClick = BtnTime1Click
    end
    object BtnTime2: TSpeedButton
      Left = 294
      Top = 2
      Width = 17
      Height = 15
      Caption = '?'
      Flat = True
      OnClick = BtnTime2Click
    end
    object TimeStartF: TCheckBox
      Left = 4
      Top = 2
      Width = 109
      Height = 17
      Caption = 'Time Start (GPST)'
      TabOrder = 0
      OnClick = TimeStartFClick
    end
    object TimeY1: TEdit
      Left = 4
      Top = 18
      Width = 65
      Height = 21
      Enabled = False
      TabOrder = 1
      Text = '2000/01/01'
    end
    object TimeY1UD: TUpDown
      Left = 68
      Top = 19
      Width = 17
      Height = 20
      Enabled = False
      Min = -32000
      Max = 32000
      TabOrder = 2
      Wrap = True
      OnChangingEx = TimeY1UDChangingEx
    end
    object TimeH1: TEdit
      Left = 90
      Top = 18
      Width = 55
      Height = 21
      Enabled = False
      TabOrder = 3
      Text = '00:00:00'
    end
    object TimeH1UD: TUpDown
      Left = 144
      Top = 19
      Width = 17
      Height = 20
      Enabled = False
      Min = -32000
      Max = 32000
      TabOrder = 4
      Wrap = True
      OnChangingEx = TimeH1UDChangingEx
    end
    object TimeEndF: TCheckBox
      Left = 172
      Top = 2
      Width = 103
      Height = 17
      Caption = 'Time End (GPST)'
      TabOrder = 5
      OnClick = TimeEndFClick
    end
    object TimeY2: TEdit
      Left = 172
      Top = 18
      Width = 65
      Height = 21
      Enabled = False
      TabOrder = 6
      Text = '2010/12/31'
    end
    object TimeY2UD: TUpDown
      Left = 236
      Top = 19
      Width = 17
      Height = 20
      Enabled = False
      Min = -32000
      Max = 32000
      TabOrder = 7
      OnChangingEx = TimeY2UDChangingEx
    end
    object TimeH2: TEdit
      Left = 258
      Top = 18
      Width = 55
      Height = 21
      Enabled = False
      TabOrder = 8
      Text = '23:59:59'
    end
    object TimeH2UD: TUpDown
      Left = 310
      Top = 19
      Width = 17
      Height = 20
      Enabled = False
      Min = -32000
      Max = 32000
      TabOrder = 9
      Wrap = True
      OnChangingEx = TimeH2UDChangingEx
    end
    object TimeIntF: TCheckBox
      Left = 338
      Top = 2
      Width = 61
      Height = 17
      Caption = 'Interval'
      TabOrder = 10
      OnClick = TimeIntFClick
    end
    object TimeInt: TComboBox
      Left = 338
      Top = 18
      Width = 59
      Height = 21
      DropDownCount = 20
      ItemHeight = 13
      TabOrder = 11
      Text = '0'
      Items.Strings = (
        '0'
        '0.05'
        '0.1'
        '0.2'
        '0.25'
        '0.5'
        '1'
        '5'
        '10'
        '15'
        '30'
        '60')
    end
  end
  object BtnPlot: TButton
    Left = 4
    Top = 252
    Width = 77
    Height = 23
    Caption = '&Plot...'
    TabOrder = 6
    OnClick = BtnPlotClick
  end
  object OpenDialog: TOpenDialog
    Filter = 
      'All (*.*)|*.*|NovAtel Log (*.gps)|*.gps|u-blox Log (*.ubx)|*.ubx' +
      '|SuperStar II Log (*.log)|*.log|Crescent Log (*.bin)|*.bin'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    OptionsEx = [ofExNoPlacesBar]
    Left = 214
    Top = 246
  end
  object OpenDialog2: TOpenDialog
    Filter = 
      'All (*.*)|*.*|RINEX Obs File (*.obs,*.*o)|*.obs;*.*o|RINEX Nav F' +
      'ile (*.nav,*.*n)|*.nav;*.*n|RINEX GNav File (*.gnav,*.*g)|*.gnav' +
      ';*.*g|SBAS Log File (*.sbs)|*.sbs'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    OptionsEx = [ofExNoPlacesBar]
    Left = 184
    Top = 246
  end
end
