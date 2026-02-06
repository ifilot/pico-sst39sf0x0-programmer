; ==============
; NSIS Installer
; ==============

!define APP_PUBLISHER "Ivo Filot"
!define APP_EXE "pico-sst39sf0x0-programmer.exe"

; Installer icon
Icon "gui/assets/icon/eeprom_icon.ico"
UninstallIcon "gui/assets/icon/eeprom_icon.ico"

OutFile "${APP_NAME}-${APP_VERSION}-setup.exe"

; Install inside user profile
InstallDir "$LOCALAPPDATA\Programs\${APP_NAME}"
InstallDirRegKey HKCU "Software\${APP_NAME}" "InstallDir"

; No admin rights required
RequestExecutionLevel user

Page directory
Page instfiles

Section "Install"
    SetOutPath "$INSTDIR"

    ; Install shortcut icon
    File "gui/assets/icon/eeprom_icon.ico"

    ; Copy application files
    File /r "dist\${APP_NAME}\*"

    ; Save install location
    WriteRegStr HKCU "Software\${APP_NAME}" "InstallDir" "$INSTDIR"
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; --- Register app in Windows Installed Apps (user scope) ---
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayIcon" "$INSTDIR\${APP_EXE}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "Publisher" "${APP_PUBLISHER}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayVersion" "${APP_VERSION}"
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoModify" 1
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoRepair" 1

    ; Start menu shortcut
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortcut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\eeprom_icon.ico"

    ; Desktop shortcut
    CreateShortcut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\eeprom_icon.ico"
SectionEnd

Section "Uninstall"
    Delete "$INSTDIR\${APP_EXE}"
    Delete "$INSTDIR\eeprom_icon.ico"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir /r "$INSTDIR"

    Delete "$DESKTOP\${APP_NAME}.lnk"
    RMDir /r "$SMPROGRAMS\${APP_NAME}"

    DeleteRegKey HKCU "Software\${APP_NAME}"
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
SectionEnd
