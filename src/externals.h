#pragma once

//class MVThread;
//class ChipThread;
//class ChipDriver;
//class Scenario;
class SystemConfig;
class MessagesInspector;
struct WindowAddress;
class MainChartWnd;

//extern MVThread* MV, *Assistant;
//extern ChipThread *ChipThrd;
//extern ChipDriver *ATmega;
extern MainChartWnd GlobalChart;

extern CEvent TerminateEvent[4];

extern SystemConfig MainCfg;

extern HICON StartIcon, PauseIcon, ResumeIcon, StopIcon, Start1Icon, Stop1Icon;	

extern WindowAddress EventsLog, TerminalWindow, ControllerWindow, PGADialog, MainFrame;

extern MessagesInspector GlobalInspector;
