#ifndef APP_VERSION
  #define APP_VERSION "0.0.0"
#endif
#ifndef VST3_PATH
  #define VST3_PATH "vitOTTx.vst3"
#endif

[Setup]
AppName=vitOTTx
AppVersion={#APP_VERSION}
AppPublisher=DsgDnB
AppPublisherURL=https://dsgdnb.com
DefaultDirName={commoncf64}\VST3
DirExistsWarning=no
DisableProgramGroupPage=yes
OutputBaseFilename=vitOTTx-Setup
Compression=lzma2
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
Uninstallable=yes
UninstallDisplayName=vitOTTx VST3

[Files]
Source: "{#VST3_PATH}"; DestDir: "{app}"; Flags: ignoreversion

[Messages]
SelectDirLabel3=vitOTTx will be installed into the following VST3 folder.
SelectDirBrowseLabel=This should be your VST3 plugin folder. Click Install to continue, or Browse to select a different folder.
