#pragma once#include "Utilities.h"					// f�r LongJump#include "Windows.h"#define	kOSEvent				app4Evt#define	kSuspendResumeMessage	1#define	kResumeMask				1#define	kMouseMovedMessage		0xFA#define MAXSLEEP				0xFFFFFFFFextern EventRecord	gTheEvent;			// globaler Event-Recordextern int			gClicks;			// Anzahl der Mausklicksextern Boolean		gQuitApplication;	// true, dann Programmende#if USETEXTEDITextern TEHandle		gTEHandle;			// Text-Editextern WindPtr		gTEWindow;			// Window zum Text-Edit#endifextern JumpBuffer	gJumpBuffer;		// LongJump to Event-Loopextern WORD			gLastCursor;		// Cursorform bei Suspend (f�r Resume)Boolean	GetEvent(VOID);VOID	DoEvent(VOID);VOID	EventLoop(VOID);VOID	DoIdleTasks(VOID);