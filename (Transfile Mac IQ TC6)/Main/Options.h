//// Options.h//#pragma once#include "GlobalDefines.h"#include "Geos.h"#define	MAX_SCHEDULE_SIZE	12				// Maximale Fontgr��e im Scheduler#define DIALOG_FONT_SIZE	12				// Fontgr��e f�r default-Text#define MAX_FONT_SIZE		300				// Momentan bis 300 pt erlaubtvoid	OptionsMenuEnable(void);void	OptionsMenu(int	entry);void	DoLayout(void);void	FontMenu(int entry);void	FontSizeMenu(int entry);void	CalcSecondColumn(GrafPtr w);Boolean	DoFontChange(void);Boolean	Numeric(Str255 s);#if BETA && CACHEvoid	SwitchCacheDisable(void);void	DoClearCache(void);void	DoCheckCache(void);#endif