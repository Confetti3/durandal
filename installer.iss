; Inno Setup Script for Durandal
; Generated for Windows installer creation

#define MyAppName "Durandal"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Confetti3"
#define MyAppURL "https://github.com/Confetti3/durandal"
#define MyAppExeName "Durandal.exe"

[Setup]
AppId={{D4RND4L-QT6-NOTE-APP-2026}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=LICENSE
InfoBeforeFile=README.md
OutputDir=..
OutputBaseFilename=Durandal-Setup-{#MyAppVersion}
SetupIconFile=resources\icons\icon.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Main executable
Source: "release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
; Qt WebEngine helper
Source: "release\QtWebEngineProcess.exe"; DestDir: "{app}"; Flags: ignoreversion
; All DLLs
Source: "release\*.dll"; DestDir: "{app}"; Flags: ignoreversion
; Qt plugins
Source: "release\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs
Source: "release\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs
Source: "release\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs
Source: "release\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs
Source: "release\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs
Source: "release\resources\*"; DestDir: "{app}\resources"; Flags: ignoreversion recursesubdirs
Source: "release\generic\*"; DestDir: "{app}\generic"; Flags: ignoreversion recursesubdirs
Source: "release\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs
Source: "release\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs
Source: "release\qml\*"; DestDir: "{app}\qml"; Flags: ignoreversion recursesubdirs
Source: "release\qmltooling\*"; DestDir: "{app}\qmltooling"; Flags: ignoreversion recursesubdirs
Source: "release\position\*"; DestDir: "{app}\position"; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
