; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "FreeOrion"
!define PRODUCT_VERSION "0.3.14"
!define PRODUCT_PUBLISHER "FreeOrion Community"
!define PRODUCT_WEB_SITE "http://www.freeorion.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\${PRODUCT_NAME}"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

SetCompressor bzip2

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "..\default\COPYING"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\freeorion.exe"
;!define MUI_FINISHPAGE_RUN_PARAMETERS "--fullscreen 1"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "..\..\FreeOrion-0.3.14-Setup.exe"
InstallDir "$PROGRAMFILES\FreeOrion"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite try
  File "..\..\vcredist_x86.exe"
  ExecWait "$INSTDIR\vcredist_x86.exe /q"
  Delete "$INSTDIR\vcredist_x86.exe"

  File "..\GiGi.dll"
  File "..\GiGiOgre.dll"
  File "..\GiGiOgrePlugin_OIS.dll"
  File "..\OIS.dll"
  File "..\OgreMain.dll"
  File "..\OpenAL32.dll"
  File "..\Plugin_OctreeSceneManager.dll"
  File "..\Plugin_CgProgramManager.dll"
  File "..\Plugin_ParticleFX.dll"
  File "..\RenderSystem_GL.dll"
  File "..\alut.dll"
  File "..\cg.dll"
  File "..\boost_date_time-vc90-mt-1_42.dll"
  File "..\boost_filesystem-vc90-mt-1_42.dll"
  File "..\boost_python-vc90-mt-1_42.dll"
  File "..\boost_regex-vc90-mt-1_42.dll"
  File "..\boost_serialization-vc90-mt-1_42.dll"
  File "..\boost_signals-vc90-mt-1_42.dll"
  File "..\boost_system-vc90-mt-1_42.dll"
  File "..\boost_thread-vc90-mt-1_42.dll"
  File "..\glew32.dll"
  File "..\jpeg.dll"
  File "..\libexpat.dll"
  File "..\libogg.dll"
  File "..\libpng13.dll"
  File "..\libvorbis.dll"
  File "..\libvorbisfile.dll"
  File "..\python26.dll"
  File "..\wrap_oal.dll"
  File "..\z.dll"
  File "..\zlib1.dll"
  File "..\freeorionca.exe"
  File "..\freeoriond.exe"
  File "..\freeorion.exe"
  File "..\FreeOrion.ico"
  File "..\OISInput.cfg"
  File "..\ogre_plugins.cfg"

  File /r /x .svn "..\default"

  CreateDirectory "$SMPROGRAMS\FreeOrion"
  CreateShortCut "$SMPROGRAMS\FreeOrion\FreeOrion Fullscreen.lnk" "$INSTDIR\freeorion.exe" "--fullscreen"
  CreateShortCut "$SMPROGRAMS\FreeOrion\FreeOrion Windowed.lnk" "$INSTDIR\freeorion.exe"
  CreateShortCut "$DESKTOP\FreeOrion.lnk" "$INSTDIR\freeorion.exe" "--fullscreen 1"
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\FreeOrion.org.url" "InternetShortcut" "URL" "http://www.freeorion.org"
  CreateShortCut "$SMPROGRAMS\FreeOrion\FreeOrion.org.lnk" "$INSTDIR\FreeOrion.org.url"
  CreateShortCut "$SMPROGRAMS\FreeOrion\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\FreeOrion.ico"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$INSTDIR\GiGi.dll"
  Delete "$INSTDIR\GiGiOgre.dll"
  Delete "$INSTDIR\GiGiOgrePlugin_OIS.dll"
  Delete "$INSTDIR\OIS.dll"
  Delete "$INSTDIR\OgreMain.dll"
  Delete "$INSTDIR\OpenAL32.dll"
  Delete "$INSTDIR\Plugin_OctreeSceneManager.dll"
  Delete "$INSTDIR\Plugin_CgProgramManager.dll"
  Delete "$INSTDIR\Plugin_ParticleFX.dll"
  Delete "$INSTDIR\RenderSystem_GL.dll"
  Delete "$INSTDIR\alut.dll"
  Delete "$INSTDIR\cg.dll"
  Delete "$INSTDIR\boost_date_time-vc90-mt-1_42.dll"
  Delete "$INSTDIR\boost_filesystem-vc90-mt-1_42.dll"
  Delete "$INSTDIR\boost_python-vc90-mt-1_42.dll"
  Delete "$INSTDIR\boost_regex-vc90-mt-1_42.dll"
  Delete "$INSTDIR\boost_serialization-vc90-mt-1_42.dll"
  Delete "$INSTDIR\boost_signals-vc90-mt-1_42.dll"
  Delete "$INSTDIR\boost_system-vc90-mt-1_42.dll"
  Delete "$INSTDIR\boost_thread-vc90-mt-1_42.dll"
  Delete "$INSTDIR\glew32.dll"
  Delete "$INSTDIR\jpeg.dll"
  Delete "$INSTDIR\libexpat.dll"
  Delete "$INSTDIR\libogg.dll"
  Delete "$INSTDIR\libpng13.dll"
  Delete "$INSTDIR\libvorbis.dll"
  Delete "$INSTDIR\libvorbisfile.dll"
  Delete "$INSTDIR\python26.dll"
  Delete "$INSTDIR\wrap_oal.dll"
  Delete "$INSTDIR\z.dll"
  Delete "$INSTDIR\zlib1.dll"
  Delete "$INSTDIR\freeorionca.exe"
  Delete "$INSTDIR\freeoriond.exe"
  Delete "$INSTDIR\freeorion.exe"
  Delete "$INSTDIR\FreeOrion.ico"
  Delete "$INSTDIR\OISInput.cfg"
  Delete "$INSTDIR\ogre_plugins.cfg"

  RMDir /r "$INSTDIR\default"

  Delete "$SMPROGRAMS\FreeOrion\Uninstall.lnk"
  Delete "$SMPROGRAMS\FreeOrion\Website.lnk"
  Delete "$DESKTOP\FreeOrion.lnk"
  Delete "$SMPROGRAMS\FreeOrion\FreeOrion.lnk"
  Delete "$SMPROGRAMS\FreeOrion\FreeOrion windowed.lnk"
  RMDir "$SMPROGRAMS\FreeOrion"

  Delete "$INSTDIR\FreeOrion.org.url"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"


  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd