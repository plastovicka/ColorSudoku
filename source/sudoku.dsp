# Microsoft Developer Studio Project File - Name="sudoku" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=SUDOKU - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sudoku.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sudoku.mak" CFG="SUDOKU - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sudoku - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "sudoku - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "sudoku - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE "sudoku - Win32 NoPNG" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sudoku - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USE_PNG" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 libpng.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib version.lib /nologo /subsystem:windows /machine:I386 /out:"../sudoku.exe"
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=c:\_Petr\cw\hotkeyp\hotkeyp.exe -close window Sudoku
# End Special Build Tool

!ELSEIF  "$(CFG)" == "sudoku - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "_DEBUG"
# ADD RSC /l 0x405 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libpng.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib version.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../sudokudbg.exe" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=c:\_Petr\cw\hotkeyp\hotkeyp.exe -close window Sudoku
# End Special Build Tool

!ELSEIF  "$(CFG)" == "sudoku - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "sudoku___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "sudoku___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseW"
# PROP Intermediate_Dir "ReleaseW"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "USE_PNG" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib /nologo /subsystem:windows /machine:I386 /out:"../sudoku.exe"
# ADD LINK32 libpng.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib version.lib /nologo /subsystem:windows /machine:I386 /out:"../sudokuW.exe"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=c:\_Petr\cw\hotkeyp\hotkeyp.exe -close window Sudoku
# End Special Build Tool

!ELSEIF  "$(CFG)" == "sudoku - Win32 NoPNG"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "sudoku___Win32_NoPNG"
# PROP BASE Intermediate_Dir "sudoku___Win32_NoPNG"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "sudoku___Win32_NoPNG"
# PROP Intermediate_Dir "sudoku___Win32_NoPNG"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USE_PNG" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libpng.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib version.lib /nologo /subsystem:windows /machine:I386 /out:"../sudoku.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib version.lib /nologo /subsystem:windows /machine:I386 /out:"../sudokuNoPNG.exe"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=c:\_Petr\cw\hotkeyp\hotkeyp.exe -close window Sudoku
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "sudoku - Win32 Release"
# Name "sudoku - Win32 Debug"
# Name "sudoku - Win32 Release Unicode"
# Name "sudoku - Win32 NoPNG"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\game.cpp

!IF  "$(CFG)" == "sudoku - Win32 Release"

# ADD CPP /Yu"hdr.h"

!ELSEIF  "$(CFG)" == "sudoku - Win32 Debug"

# ADD CPP /Yu"hdr.h"

!ELSEIF  "$(CFG)" == "sudoku - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "sudoku - Win32 NoPNG"

# ADD BASE CPP /Yu"hdr.h"
# ADD CPP /Yu"hdr.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gener.cpp

!IF  "$(CFG)" == "sudoku - Win32 Release"

# ADD CPP /Yu"hdr.h"

!ELSEIF  "$(CFG)" == "sudoku - Win32 Debug"

# ADD CPP /Yu"hdr.h"

!ELSEIF  "$(CFG)" == "sudoku - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "sudoku - Win32 NoPNG"

# ADD BASE CPP /Yu"hdr.h"
# ADD CPP /Yu"hdr.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hdr.cpp
# ADD CPP /Yc"hdr.h"
# End Source File
# Begin Source File

SOURCE=.\lang.cpp
# ADD CPP /Yu"hdr.h"
# End Source File
# Begin Source File

SOURCE=.\pdf.cpp
# ADD CPP /Yu"hdr.h"
# End Source File
# Begin Source File

SOURCE=.\save.cpp

!IF  "$(CFG)" == "sudoku - Win32 Release"

# ADD CPP /Yu"hdr.h"

!ELSEIF  "$(CFG)" == "sudoku - Win32 Debug"

# ADD CPP /Yu"hdr.h"

!ELSEIF  "$(CFG)" == "sudoku - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "sudoku - Win32 NoPNG"

# ADD BASE CPP /Yu"hdr.h"
# ADD CPP /Yu"hdr.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sudoku.cpp
# ADD CPP /Yu"hdr.h"
# End Source File
# Begin Source File

SOURCE=.\sudoku.rc
# End Source File
# Begin Source File

SOURCE=.\think.cpp
# ADD CPP /Yu"hdr.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\hdr.h
# End Source File
# Begin Source File

SOURCE=.\lang.h
# End Source File
# Begin Source File

SOURCE=.\sudoku.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\sudoku.ico
# End Source File
# Begin Source File

SOURCE=.\sudotool.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\test.txt
# End Source File
# End Target
# End Project
