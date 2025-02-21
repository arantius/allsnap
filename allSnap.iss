;--allSnap Install

[Setup]
OutputBaseFilename=allSnap Setup 1.6B1
AppName=allSnap
AppVerName=allSnap version 1.6 Beta 1
AppMutex=IVAN_HECKMAN_ALLSNAP_MUTEX,IVAN_HECKMAN_ALLSNAP_MUTEXB
AppCopyright=Copyright © 2002 Ivan Heckman; 2025 Anthony Lieuallen
AppPublisher=
AppPublisherURL=
AppUpdatesURL=
AppVersion=1.6 Beta 1
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
DefaultDirName={commonpf}\allSnap
DefaultGroupName=allSnap
UninstallDisplayIcon={app}\allSnap.exe
LicenseFile=LICENSE
MinVersion=10.0
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
AllowNoIcons=yes


[Files]
Source: "x64\Release UNICODE\allSnap.exe"; DestDir: "{app}"
Source: "x64\Release UNICODE\snap_lib.dll"; DestDir: "{app}"
Source: "snap.wav"; DestDir: "{app}"
Source: "unsnap.wav"; DestDir: "{app}"
Source: "help\allSnap.chm"; DestDir: "{app}"


[Tasks]
Name: startup; Description: "Run allSnap on &startup"
Name: desktopicon; Description: "Create a &desktop icon";  Flags: unchecked
;Name: quicklaunchicon; Description: "Create a &quick launch icon";  Flags: unchecked


[Code]
function GetPathInstalled( AppID: String ): String;
var
   sPrevPath: String;
begin
  sPrevPath := '';
  if not RegQueryStringValue( HKLM,
    'Software\Microsoft\Windows\CurrentVersion\Uninstall\'+AppID+'_is1',
    'Inno Setup: App Path', sPrevpath) then
    RegQueryStringValue( HKCU, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\'+AppID+'_is1' ,
      'Inno Setup: App Path', sPrevpath);

  Result := sPrevPath;
end;


[Icons]
Name: "{group}\allSnap"; Filename: "{app}\allSnap.exe"; WorkingDir: "{app}"
Name: "{group}\allSnap Help"; Filename: "{app}\allSnap.chm"; WorkingDir: "{app}"
Name: "{userdesktop}\allSnap"; Filename: "{app}\allSnap.exe"; WorkingDir: "{app}"; Tasks: desktopicon
;Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\allSnap"; Filename: "{app}\allSnap.exe";WorkingDir: "{app}"; Tasks: quicklaunchicon
Name: "{userstartup}\allSnap"; Filename: "{app}\allSnap.exe"; WorkingDir: "{app}"; Tasks: startup
; NOTE: Most apps do not need registry entries to be pre-created. If you
; don't know what the registry is or if you need to use it, then chances are
; you don't need a [Registry] section.

[Registry]
; Start "Software\My Company\My Program" keys under HKEY_CURRENT_USER
; and HKEY_LOCAL_MACHINE. The flags tell it to always delete the
; "My Program" keys upon uninstall, and delete the "My Company" keys
; if there is nothing left in them.
Root: HKCU; Subkey: "Software\IvanHeckman"; Flags: uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\IvanHeckman\allSnap"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\IvanHeckman\allSnap\Settings"; ValueType: string; ValueName: "snap_sound_file"; ValueData: "{app}\snap.wav"
Root: HKCU; Subkey: "Software\IvanHeckman\allSnap\Settings"; ValueType: string; ValueName: "unsnap_sound_file"; ValueData: "{app}\unsnap.wav"

[Run]
Filename: "{app}\allSnap.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent unchecked
