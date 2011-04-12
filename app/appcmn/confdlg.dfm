object ConfDialog: TConfDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Confirmation'
  ClientHeight = 87
  ClientWidth = 288
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 267
    Height = 13
    Alignment = taCenter
    AutoSize = False
    Caption = 'File exists. Overwrite it?'
  end
  object Label2: TLabel
    Left = 8
    Top = 24
    Width = 267
    Height = 37
    Alignment = taCenter
    AutoSize = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clGray
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    WordWrap = True
  end
  object BtnOverwrite: TButton
    Left = 66
    Top = 64
    Width = 75
    Height = 21
    Caption = '&Overwrite'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ModalResult = 1
    ParentFont = False
    TabOrder = 1
  end
  object BtnCancel: TButton
    Left = 144
    Top = 64
    Width = 75
    Height = 21
    Caption = '&Cancel'
    Default = True
    ModalResult = 2
    TabOrder = 0
  end
end
