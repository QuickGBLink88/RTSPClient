# Microsoft Developer Studio Project File - Name="TsSourceStreamRecv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=TsSourceStreamRecv - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TsSourceStreamRecv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TsSourceStreamRecv.mak" CFG="TsSourceStreamRecv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TsSourceStreamRecv - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "TsSourceStreamRecv - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TsSourceStreamRecv - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /I ".\include" /I ""C:\Program Files\Visual Leak Detector\include"" /ZI /W3 /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "ARCH_IS_32BIT" /D "ARCH_IS_IA32" /D "_MBCS" /Gm PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MT /I ".\include" /I ""C:\Program Files\Visual Leak Detector\include"" /ZI /W3 /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "ARCH_IS_32BIT" /D "ARCH_IS_IA32" /D "_MBCS" /Gm PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /win32 
# ADD MTL /nologo /D"_DEBUG" /win32 
# ADD BASE RSC /l 2052 /d "_DEBUG" /i "$(IntDir)" 
# ADD RSC /l 2052 /d "_DEBUG" /i "$(IntDir)" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib Version.lib vfw32.lib winmm.lib quartz.lib /nologo /out:"..\Bin\$(ProjectName).exe" /incremental:no /libpath:".\lib" /libpath:""C:\Program Files\Visual Leak Detector\lib\Win32"" /debug /pdbtype:sept /subsystem:windows /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib Version.lib vfw32.lib winmm.lib quartz.lib /nologo /out:"..\Bin\$(ProjectName).exe" /incremental:no /libpath:".\lib" /libpath:""C:\Program Files\Visual Leak Detector\lib\Win32"" /debug /pdbtype:sept /subsystem:windows /machine:ix86 

!ELSEIF  "$(CFG)" == "TsSourceStreamRecv - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /I ".\include" /Zi /W3 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /c /GX 
# ADD CPP /nologo /MT /I ".\include" /Zi /W3 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /c /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /win32 
# ADD MTL /nologo /D"NDEBUG" /win32 
# ADD BASE RSC /l 2052 /d "NDEBUG" /i "$(IntDir)" 
# ADD RSC /l 2052 /d "NDEBUG" /i "$(IntDir)" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib quartz.lib winmm.lib ws2_32.lib vfw32.lib /nologo /out:"..\Bin\$(ProjectName).exe" /incremental:no /libpath:".\lib" /debug /pdbtype:sept /subsystem:windows /opt:ref /opt:icf /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib quartz.lib winmm.lib ws2_32.lib vfw32.lib /nologo /out:"..\Bin\$(ProjectName).exe" /incremental:no /libpath:".\lib" /debug /pdbtype:sept /subsystem:windows /opt:ref /opt:icf /machine:ix86 

!ENDIF

# Begin Target

# Name "TsSourceStreamRecv - Win32 Debug"
# Name "TsSourceStreamRecv - Win32 Release"
# Begin Group "源"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=.\CaptureVideo.cpp
# End Source File
# Begin Source File

SOURCE=.\CGdiPainter.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildView.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectServerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\cstreamsocket2.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\NSPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\PProfile.cpp
# End Source File
# Begin Source File

SOURCE=.\StatisticsViewDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp

!IF  "$(CFG)" == "TsSourceStreamRecv - Win32 Debug"

# ADD CPP /nologo /Yc"stdafx.h" /GZ /GX 
!ELSEIF  "$(CFG)" == "TsSourceStreamRecv - Win32 Release"

# ADD CPP /nologo /Yc"stdafx.h" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=.\VideoDisplayWnd.cpp
# End Source File
# End Group
# Begin Group "头"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=.\CaptureVideo.h
# End Source File
# Begin Source File

SOURCE=.\CGdiPainter.h
# End Source File
# Begin Source File

SOURCE=.\ChildView.h
# End Source File
# Begin Source File

SOURCE=.\ConnectServerDlg.h
# End Source File
# Begin Source File

SOURCE=.\CStreamSocket2.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\NSPlayer.h
# End Source File
# Begin Source File

SOURCE=.\PProfile.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StatisticsViewDlg.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\VideoDisplayWnd.h
# End Source File
# End Group
# Begin Group "资源"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx"
# Begin Source File

SOURCE=.\res\NSPlayer.ico
# End Source File
# Begin Source File

SOURCE=.\NSPlayer.rc
# End Source File
# Begin Source File

SOURCE=.\res\NSPlayer.rc2
# End Source File
# End Group
# Begin Group "SDK"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\avt_net.h
# End Source File
# End Group
# Begin Group "Util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DemuxTS.cpp
# End Source File
# Begin Source File

SOURCE=.\DemuxTS.h
# End Source File
# Begin Source File

SOURCE=.\Structure.h
# End Source File
# Begin Source File

SOURCE=.\vsparser.cpp
# End Source File
# Begin Source File

SOURCE=.\vsparser.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\res\NSPlayer.manifest

!IF  "$(CFG)" == "TsSourceStreamRecv - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TsSourceStreamRecv - Win32 Release"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project

