
#define MyAppName "QHYCCD EZCAP_QT"
#define MyAppVersion GetDateTimeString('yy.mm.dd.hh', '', '') 
;0.1.51.19     

[Files]
Source: {source}\*.*; DestDir: {app}; Flags: recursesubdirs;
Source: sendmail.dll; Flags: dontcopy solidbreak;

[Run]
;Filename: "{app}\EZCAP\devcon64.exe"; Parameters: "install driver\VCam_WDM.inf VCam_WDM"; WorkingDir: "{app}\EZCAP\"; Flags: runhidden; StatusMsg: "Installing Webcam Interface Device Driver"; Check: IsWin64
;Filename: "{app}\EZCAP\devcon.exe";   Parameters: "install driver\VCam_WDM.inf VCam_WDM"; WorkingDir: "{app}\EZCAP\"; Flags: runhidden; StatusMsg: "Installing Webcam Interface Device Driver";  Check: not IsWin64
;Filename: "regsvr32.exe"; Parameters: "VCamFilter.ax"; WorkingDir: "{app}\EZCAP"; Description: "Install Video Filter"
;Filename: "regsvr32.exe"; Parameters: "VCamCOM.dll";   WorkingDir: "{app}\EZCAP"; Description: "Install Video Filter"

[Setup]
AppId={{BCF29B8E-B29E-4939-B8F2-54C0A20390DB}
PrivilegesRequired=admin
ShowLanguageDialog=no
AppName={#MyAppName}
AppVerName={#MyAppName} Ver{#MyAppVersion}
AppVersion={#MyAppVersion}
AppCopyright=(c) QHYCCD
VersionInfoDescription=EZCAP_QT
VersionInfoVersion={#MyAppVersion}
VersionInfoTextVersion=08
DefaultDirName={pf32}\QHYCCD\EZCAP_QT
DefaultGroupName = QHYCCD EZCAP_QT
OutputDir=Output
OutputBaseFilename= EZCAP_QT_V{#MyAppVersion}_Setup


[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "cs"; MessagesFile: "compiler:Languages\ChineseSimp.isl"


[Icons]
Name: "{group}\EZCAP_QT";                                Filename: "{app}\EZCAP_Qt\EZCAP.exe"
Name: "{group}\Uninstall EZCAP_QT";                      Filename: "{uninstallexe}"
Name: "{commondesktop}\EZCAP_QT";                        Filename: "{app}\EZCAP_Qt\EZCAP.exe"; WorkingDir: "{app}\EZCAP_Qt"

[UninstallRun]
;Filename: "{app}\EZCAP\devcon64.exe"; Parameters: "remove VCam_WDM"; WorkingDir: "{app}\EZCAP\"; Flags: 64bit runhidden; StatusMsg: "Uninstalling Webcam Interface Device Driver";  Check: IsWin64
;Filename: "{app}\EZCAP\devcon.exe";   Parameters: "remove VCam_WDM"; WorkingDir: "{app}\EZCAP\"; Flags: 32bit runhidden; StatusMsg: "Uninstalling Webcam Interface Device Driver";  Check: not IsWin64
;Filename: "regsvr32.exe"; Parameters: "VCamFilter.ax /u"; WorkingDir: "{app}\EZCAP"; 
;Filename: "regsvr32.exe"; Parameters: "VCamCOM.dll /u";   WorkingDir: "{app}\EZCAP";



[ISFormDesigner]
WizardForm=FF0A005457495A415244464F524D003010C109000054504630F10B5457697A617264466F726D0A57697A617264466F726D0C436C69656E744865696768740368010B436C69656E74576964746803F1010C4578706C696369744C65667402000B4578706C69636974546F7002000D4578706C69636974576964746803F9010E4578706C696369744865696768740383010D506978656C73506572496E636802600A54657874486569676874020D00F10C544E65774E6F7465626F6F6B0D4F757465724E6F7465626F6F6B00F110544E65774E6F7465626F6F6B506167650B57656C636F6D6550616765084E65787450616765070D4953437573746F6D50616765310D4578706C69636974576964746803F1010E4578706C696369744865696768740339010000F110544E65774E6F7465626F6F6B5061676509496E6E6572506167650D4578706C69636974576964746803F1010E4578706C6963697448656967687403390100F10C544E65774E6F7465626F6F6B0D496E6E65724E6F7465626F6F6B00F110544E65774E6F7465626F6F6B506167650B4C6963656E7365506167650C50726576696F757350616765070D4953437573746F6D50616765310D4578706C69636974576964746803A1010E4578706C6963697448656967687403ED00000010544E65774E6F7465626F6F6B506167650D4953437573746F6D50616765310743617074696F6E1206000000AE90F64ED15301904B6DD58B0B4465736372697074696F6E120C000000938F6551F8767351C25370657353EF53D1530190AE90F64E0C50726576696F757350616765070B57656C636F6D6550616765084E65787450616765070B4C6963656E7365506167650006544C6162656C064C6162656C31044C656674020803546F700208055769647468023C06486569676874021A0743617074696F6E1417000000E982AEE4BBB6E69C8DE58AA1E599A80D0A28534D5450290B5472616E73706172656E7408000006544C6162656C064C6162656C32044C656674022C03546F700220055769647468021806486569676874020D0743617074696F6E1202000000EF7AE3530B5472616E73706172656E7408000006544C6162656C064C6162656C33044C65667403C80003546F70020805576964746803900006486569676874020D084175746F53697A65080743617074696F6E1414000000E4BE8BE5A6823A20736D74702E3136332E636F6D0B5472616E73706172656E7408000006544C6162656C064C6162656C34044C65667403C80003546F700220055769647468027806486569676874020D084175746F53697A65080743617074696F6E140A000000E4BE8BE5A6823A2032350B5472616E73706172656E7408000006544C6162656C064C6162656C35044C656674022C03546F700238055769647468021806486569676874020D0743617074696F6E1202000000105EF7530B5472616E73706172656E7408000006544C6162656C064C6162656C37044C656674022C03546F700250055769647468021806486569676874020D0743617074696F6E1202000000C65B01780B5472616E73706172656E7408000006544C6162656C064C6162656C36044C656674021403546F700268055769647468023006486569676874020D0743617074696F6E1204000000A5633665305740570B5472616E73706172656E7408000006544C6162656C064C6162656C38044C65667403C80003546F700268055769647468027806486569676874020D084175746F53697A65080743617074696F6E120A000000D98FCC916B586551F95BB965AE90B17B305740570B5472616E73706172656E7408000006544C6162656C064C6162656C39044C656674021403546F70038000055769647468023006486569676874020D0743617074696F6E1204000000D1530190305740570B5472616E73706172656E7408000006544C6162656C074C6162656C3130044C65667403C80003546F7003800005576964746803900006486569676874020D084175746F53697A65080743617074696F6E120C000000D98FCC916B586551604FEA81F15D8476AE90B17B305740570B5472616E73706172656E7408000006544C6162656C074C6162656C3131044C656674022C03546F70039800055769647468021806486569676874020D0743617074696F6E12020000003B4E98980B5472616E73706172656E7408000006544C6162656C074C6162656C3132044C65667403C80003546F70039800055769647468023006486569676874020D084175746F53697A65080743617074696F6E1204000000AE90F64E3B4E98980B5472616E73706172656E7408000006544C6162656C074C6162656C3133044C656674022C03546F7003B000055769647468021806486569676874020D0743617074696F6E12020000008551B95B0B5472616E73706172656E7408000006544C6162656C074C6162656C3134044C65667403680103546F7003B000055769647468023006486569676874020D084175746F53697A65080743617074696F6E1204000000AE90F64E8551B95B0B5472616E73706172656E7408000008544E65774564697407656474486F7374044C656674024803546F7002080557696474680279064865696768740215085461624F7264657202000454657874060C736D74702E3136332E636F6D000008544E65774564697407656474506F7274044C656674024803546F7002200557696474680279064865696768740215085461624F726465720201045465787406023235000008544E6577456469740B656474557365726E616D65044C656674024803546F7002380557696474680279064865696768740215085461624F726465720202000008544E6577456469740B65647450617373776F7264044C656674024803546F7002500557696474680279064865696768740215085461624F726465720203000008544E6577456469740F656474454D61696C41646472657373044C656674024803546F7002680557696474680279064865696768740215085461624F726465720204000008544E6577456469740A65647441646472657373044C656674024803546F700380000557696474680279064865696768740215085461624F726465720205000008544E6577456469740A6564745375626A656374044C656674024803546F700398000557696474680279064865696768740215085461624F726465720206000008544E65774564697407656474426F6479044C656674024803546F7003B000055769647468031901064865696768740215085461624F72646572020700000A544E6577427574746F6E0B62746E53656E644D61696C044C65667403380103546F7003D000055769647468024B0648656967687402190743617074696F6E1204000000D1530190AE90F64E085461624F726465720208074F6E436C69636B071062746E53656E644D61696C436C69636B00000000000000


[code] 
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var infoStr: String;
begin 
  { 根据当前语言不同，显示中文/英文}   if ActiveLanguage() = 'cs' then 
      infoStr := '是否清除用户配置信息？'
  else
      infoStr := 'Whether you want to delete the user configuration information？';

  if CurUninstallStep = usUninstall then
    if MsgBox(infoStr, mbConfirmation, MB_YESNO) = IDYES then
      //删除 {app} 文件夹及其中所有文件                                                                                
      DelTree(ExpandConstant('{app}'), True, True, True);
end;



// Code to enable the installer to uninstall previous versions of itself when a new version is installed
procedure CurStepChanged(CurStep: TSetupStep);
var
  ResultCode: Integer;
  UninstallExe: String;
  UninstallRegistry: String;
  removePreVer: String;
begin
  { 根据当前语言不同，显示中文/英文} 
  if ActiveLanguage() = 'cs' then 
      removePreVer := '检测到已安装过该软件，开始卸载旧版本.'
  else
      removePreVer := 'Setup will now remove the previous version.';

  if (CurStep = ssInstall) then // Install step has started
	begin
      // Create the correct registry location name, which is based on the AppId
      UninstallRegistry := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#SetupSetting("AppId")}' + '_is1');
      // Check whether an extry exists
      if RegQueryStringValue(HKLM, UninstallRegistry, 'UninstallString', UninstallExe) then
        begin // Entry exists and previous version is installed so run its uninstaller quietly after informing the user
          MsgBox(removePreVer, mbInformation, MB_OK);
          Exec(RemoveQuotes(UninstallExe), ' /SILENT', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ResultCode);
          sleep(1000);    //Give enough time for the install screen to be repainted before continuing
        end
  end;
end;


// ------用户信息采集功能------------------


function QSendMail(QHost, QUsername, QPassword, QEMailAddresses, QAddress, QSubject, QBody: String; QPort: Integer): Boolean;
external 'QSendMail@files:sendmail.dll stdcall';

{ RedesignWizardFormBegin } // 不要删除这一行代码。
// 不要修改这一段代码，它是自动生成的。
var
  ISCustomPage1: TWizardPage;
  ISCustomPageID: Integer;
  Label1: TLabel;
  Label2: TLabel;
  Label3: TLabel;
  Label4: TLabel;
  Label5: TLabel;
  Label7: TLabel;
  Label6: TLabel;
  edtNation: TNewEdit;
  edtAgent: TNewEdit;
  edtCameraType: TNewEdit;
  edtName: TNewEdit;
  edtEMailBoxAddress: TNewEdit;
  edtSuggestions: TNewEdit;
  btnSendMail: TNewButton;

  strTitle: String;
  strSubtitle: String;
  strNation: String;
  strAgent: String;
  strCamType: String;
  strName: String;
  strEmailBox: String;
  strFeedback: String;
  strBtnSend: String;
  strSuccessMsg: String;
  strFailureMsg: String;
  strHintInfo: String;

  {弹出提示信息}
procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = ISCustomPageID then   
    MsgBox(strHintInfo, mbInformation, MB_OK);  
end;


//跳过信息采集
function ShouldSkipPage(PageID: Integer): Boolean;
begin
  Result := False;
  if PageID = ISCustomPageID then begin
      Result := True;
    end;
end;


procedure btnSendMailClick(Sender: TObject); forward;

procedure RedesignWizardForm;
begin
  
  { 根据当前语言不同，显示中文/英文} 
  if ActiveLanguage() = 'cs' then
    begin
      strTitle := '用户体验改善计划';
      strSubtitle := '惊喜：如果您认真填写此页面内容并发送邮件成功，将获得6个月的额外保修期。感谢您的支持！';
      strNation := '国家';
      strAgent := '代理商';
      strCamType := '相机型号';
      strName := '姓名';
      strEmailBox := '邮箱地址';
      strFeedback := '反馈建议';
      strBtnSend := '发送邮件';
      strSuccessMsg := '邮件发送成功！';
      strFailureMsg := '邮件发送失败！';
      strHintInfo := '认真填写表格可获得6个月的额外保修期';
    end
  else
    begin
      strTitle := 'UE Improvement Plan';
      strSubtitle := 'Surprise: if you fill in this page content and send mail success, you will receive additional warranty period of 6 months. Thank you for your support!';
      strNation := 'Nation';
      strAgent := 'Agent';
      strCamType := 'Camera Type';
      strName := 'Name';
      strEmailBox := 'Email Address';
      strFeedback := 'Suggestions';
      strBtnSend := 'Send Email';
      strSuccessMsg := 'Send E-mail Success!';
      strFailureMsg := 'Send E-mail Failure!';
      strHintInfo := 'Fill out the form carefully will get additional warranty period of 6 months';
    end;


  { 创建自定义向导页面 }
  ISCustomPage1 := CreateCustomPage(wpWelcome, strTitle, strSubtitle);
  ISCustomPageID := ISCustomPage1.ID;

  { Label1 }
  Label1 := TLabel.Create(WizardForm);
  with Label1 do
  begin
    Parent := ISCustomPage1.Surface;
    Caption := strNation;//'国家';
    Transparent := False;
    Left := ScaleX(2);
    Top := ScaleY(8);
    Width := ScaleX(66);
    Height := ScaleY(13);
    Alignment := taRightJustify;
  end;

  { Label2 }
  Label2 := TLabel.Create(WizardForm);
  with Label2 do
  begin
    Parent := ISCustomPage1.Surface;
    AutoSize := False;
    Caption := strAgent;//'代理商';
    Transparent := False;
    Left := ScaleX(2);
    Top := ScaleY(36);
    Width := ScaleX(66);
    Height := ScaleY(13);
    Alignment := taRightJustify;
  end;

  { Label3 }
  Label3 := TLabel.Create(WizardForm);
  with Label3 do
  begin
    Parent := ISCustomPage1.Surface;
    Caption := strCamType;//'相机型号';
    Transparent := False;
    Left := ScaleX(2);
    Top := ScaleY(64);
    Width := ScaleX(66);
    Height := ScaleY(13);
    Alignment := taRightJustify;
  end;

  { Label4 }
  Label4 := TLabel.Create(WizardForm);
  with Label4 do
  begin
    Parent := ISCustomPage1.Surface;
    AutoSize := False;
    Caption := strName;//'姓名';
    Transparent := False;
    Left := ScaleX(2);
    Top := ScaleY(92);
    Width := ScaleX(66);
    Height := ScaleY(13);
    Alignment := taRightJustify;
  end;

  { Label5 }
  Label5 := TLabel.Create(WizardForm);
  with Label5 do
  begin
    Parent := ISCustomPage1.Surface;
    Caption := strEmailBox;//'邮箱地址';
    Transparent := False;
    Left := ScaleX(2);
    Top := ScaleY(120);
    Width := ScaleX(66);
    Height := ScaleY(13);
    Alignment := taRightJustify;
  end;

  { Label6 }
  Label6 := TLabel.Create(WizardForm);
  with Label6 do
  begin
    Parent := ISCustomPage1.Surface;
    AutoSize := False;
    Caption := strFeedback;//'反馈建议';
    Transparent := False;
    Left := ScaleX(2);
    Top := ScaleY(148);
    Width := ScaleX(66);
    Height := ScaleY(13);
    Alignment := taRightJustify;
  end;

  { edtNation }
  edtNation := TNewEdit.Create(WizardForm);
  with edtNation do
  begin
    Parent := ISCustomPage1.Surface;
    Left := ScaleX(72);
    Top := ScaleY(8);
    Width := ScaleX(121);
    Height := ScaleY(21);
  end;

  { edtAgent }
  edtAgent := TNewEdit.Create(WizardForm);
  with edtAgent do
  begin
    Parent := ISCustomPage1.Surface;
    Left := ScaleX(72);
    Top := ScaleY(36);
    Width := ScaleX(121);
    Height := ScaleY(21);
  end;

  { edtCameraType }
  edtCameraType := TNewEdit.Create(WizardForm);
  with edtCameraType do
  begin
    Parent := ISCustomPage1.Surface;
    Left := ScaleX(72);
    Top := ScaleY(64);
    Width := ScaleX(121);
    Height := ScaleY(21);
  end;

  { edtName }
  edtName := TNewEdit.Create(WizardForm);
  with edtName do
  begin
    Parent := ISCustomPage1.Surface;
    Left := ScaleX(72);
    Top := ScaleY(92);
    Width := ScaleX(121);
    Height := ScaleY(21);
  end;

  { edtEmailBoxAddress }
  edtEmailBoxAddress := TNewEdit.Create(WizardForm);
  with edtEmailBoxAddress do
  begin
    Parent := ISCustomPage1.Surface;
    Left := ScaleX(72);
    Top := ScaleY(120);
    Width := ScaleX(121);
    Height := ScaleY(21);
  end;

  { edtSuggestions }
  edtSuggestions := TNewEdit.Create(WizardForm);
  with edtSuggestions do
  begin
    Parent := ISCustomPage1.Surface;
    Left := ScaleX(72);
    Top := ScaleY(148);
    Width := ScaleX(312);
    Height := ScaleY(21);
  end;

  { btnSendMail }
  btnSendMail := TNewButton.Create(WizardForm);
  with btnSendMail do
  begin
    Parent := ISCustomPage1.Surface;
    Left := ScaleX(312);
    Top := ScaleY(176);
    Width := ScaleX(75);
    Height := ScaleY(25);
    Caption := strBtnSend;//'发送邮件';
    OnClick := @btnSendMailClick;
  end;


  edtNation.TabOrder := 0;
  edtAgent.TabOrder := 1;
  edtCameraType.TabOrder := 2;
  edtName.TabOrder := 3;
  edtEmailBoxAddress.TabOrder := 4;
  edtSuggestions.TabOrder := 5;
  btnSendMail.TabOrder := 6;

{ ReservationBegin }
  // 这一部分是提供给你的，你可以在这里输入一些补充代码。

{ ReservationEnd }
end;
// 不要修改这一段代码，它是自动生成的。
{ RedesignWizardFormEnd } // 不要删除这一行代码。

procedure btnSendMailClick(Sender: TObject);
var
edtHost: String;
edtPort: Integer;
edtUsername: String;
edtPassword: String;
edtEmailAddress: String;
edtAddress: String;
edtSubject: String;
edtContents: String;
begin
    edtHost := 'smtp.aliyun.com';
    edtPort := 25;
    edtUsername := 'qhyccdUser@aliyun.com';
    edtPassword := 'qhyccda1005';
    edtEmailAddress := 'userinfo@qhyccd.com';//收件人地址
    edtAddress := 'qhyccdUser@aliyun.com';//发件人地址
    edtSubject := '用户信息收集';
    edtContents := '国家:'+edtNation.Text+' ' + '代理商:'+edtAgent.Text+' ' + '相机型号:'+edtCameraType.Text+' ' +
                   '姓名:'+edtName.Text+' ' + '邮箱:'+edtEmailBoxAddress.Text+' ' + '反馈建议:'+edtSuggestions.Text;
    if QSendMail(edtHost, edtUsername, edtPassword, edtEMailAddress, edtAddress, edtSubject, edtContents, edtPort) then
      MsgBox(strSuccessMsg, mbInformation, MB_OK)
    else
      MsgBox(strFailureMsg, mbError, MB_OK);
end;

procedure InitializeWizard();
begin
  RedesignWizardForm;
end;
