/*** *	Menus.c *	Men�verwaltung *	 *	�1991 �-Soft, Markus Fritze ***/#include "Menus.h"#include "WindowsMenu.h"#include "WindowsGlobal.h"#include "rsrcDefines.h"#include "MenuCmd.h"#include "GlobalStruct.h"#include "DoEvent.h"#include "GeosMore.h"#include "Help.h"short		gAppleId,gFileId,gEditId,gWindowsId; // Men�-IDs/*** *	Men�zeile einlesen ***/void	MenuBarInit(short MenuId)				// Men�leiste initialisieren{REG Handle		MBar;		// Handle der Men�leisteREG short		anz;REG short		*p;REG MenuHandle	m;	MBar = GetResource('MBAR',MenuId);		// MBAR einlesen	if (!MBar) {		SysError(dsMBarNFnd);		ExitToShell();						// keine Men�leiste da�	}	p = (short*)*MBar;	gAppleId = p[1];	gFileId = p[2];	gEditId = p[3];	gWindowsId = p[p[0]];					// das Windows-Men� liegt ganz rechts	ReleaseResource(MBar);	MBar = GetNewMBar(MenuId);				// Men�zeile lesen	CheckResource(MBar);	SetMenuBar(MBar);						// und setzen	DisposHandle(MBar);						// und wieder freigeben	m = GetMHandle(gAppleId);				// Handle des Apfel-Men�s	CheckResource((Handle)m);	AddResMenu(m,'DRVR');					// DAs anh�ngen	{ G(InitMenus)(); }	AppendWindowMenu(gWindowsId);			// Windows-Men� dranh�ngen	AppendHelpMenu();						// Hilfe-Men� setzen	DrawMenuBar();							// Men�zeile zeichnen}/*** *	Men�punkt enabled bzw. disabled? ***/Boolean		GetMenuAble(MenuHandle m,WORD item){	return(BTstBool((*m)->enableFlags,item));}/*** *	Mehrere Men�punkte enablen bzw. disablen. * *	Format: *	m		= MenuHandle des Men�s *	id[]	= Liste der Men�punkte *			0	= Ende *			<0	= Men�punkt disablen *			>0	= Men�punkt enablen ***/VOID	MenuAble(WORD menuid,CHAR *id){REG WORD		i;REG MenuHandle	m = GetMHandle(menuid);	while(i = *id++) {					// alle Men�punkte durch?		if (i < 0)			DisableItem(m,-i);			// negativ => disablen		else			EnableItem(m,i);			// positiv => enablen	}}/*** *	Men�punkte enablen bzw. disablen ***/void	AdjustMenus(void){REG	MenuHandle	m = GetMHandle(gFileId);REG WindowPtr	w = FrontWindow();Boolean			redrawMenuBar = false;	// Men�leiste nicht neu zeichnen	EnableItem(m,iNew);EnableItem(m,iOpen);#if !ONEDOC	if (MaxBlock() < 30000) {		if (CompactMem(30000) < 30000) {			DisableItem(m,iNew);DisableItem(m,iOpen);		}	}#endif	{	static CHAR	i[] = {-iClose,-iSave,-iSaveAs,-iRevert/*,-iDuplicate*/,-iPageSetup,-iPrint,0};	MenuAble(gFileId,i);	}	if (IsDAWindow(w)) {				// ein DA aktiv?		static CHAR	i[] = {-iNew,-iOpen,iClose,0};		MenuAble(gFileId,i);	} else	if (gDoc) {							// ein Dokument offen?		EnableItem(m,iClose);#if !ONEDOC//		EnableItem(m,iDuplicate);#endif		EnableItem(m,iPageSetup);EnableItem(m,iPrint);		if (!(*gDoc)->f.reallyReadOnly)			EnableItem(m,iSaveAs);		if (AppDocumentDirty(gDoc,-1)) {	// Dokument dirty?			EnableItem(m,iSave);			if (AppDocumentFile(gDoc))				EnableItem(m,iRevert);		}	}	if (IsDialog(gTopWindow)) {				// Dialog getopped?		if (GetMenuAble(gWindowMenu,0)) {			DisableItem(gWindowMenu,0);			redrawMenuBar = true;		}									// dann einige Men�s disablen		if (GetMenuAble(m,0)) {			DisableItem(m,0);			redrawMenuBar = true;		}	} else {		if (!GetMenuAble(gWindowMenu,0)) {			EnableItem(gWindowMenu,0);			redrawMenuBar = true;		}									// dann einige Men�s wieder enablen		if (!GetMenuAble(m,0)) {			EnableItem(m,0);			redrawMenuBar = true;		}	}	WMenuUpdate(&redrawMenuBar);		// �Windows�-Men� updaten	{ G(AdjustMenu)(&redrawMenuBar); }	if (IsAppWindow(w))		WINDOW(gTopWindow,menuupdate,(long)&redrawMenuBar);	if (IsDAWindow(w)) {				// DA ist aktiv!		static CHAR i[] = {undoCmd,cutCmd,copyCmd,pasteCmd,clearCmd,0};		MenuAble(gEditId,i);	}#if USETEXTEDIT	if (gTEHandle) {					// TextEdit aktiv?		static CHAR i[] = {-undoCmd,cutCmd,copyCmd,pasteCmd,clearCmd,0};		MenuAble(gEditId,i);	}#endif	if (redrawMenuBar)					// Men�zeile neu ausgeben?		DrawMenuBar();}/*** *	Men�punkt angew�hlt ***/void	DoMenuCommand(long m){REG short	title	= HIWord(m);		// Men�titelREG short	item	= LOWord(m);		// Men�punktREG WindPtr	wind	= gTopWindow;		// aktives Window	if (title == kHMHelpMenuID) {		// Help-Men� angeklickt?		MHelp(item);					// dann separat auswerten		HiliteMenu(0);		return;	}	if ((title == gAppleId)&&(item != 1)) {	// DA �ffnen?		REG Str255	accName;		GetItem(GetMHandle(gAppleId),item,accName);		OpenDeskAcc(accName);			// DA �ffnen	} else {		if (title == gAppleId) {			{ G(DoAbout)(); }			// �About� angew�hlt		} else if (title == gFileId) {	// �Datei�-Men�			switch(item) {			case iNew:		MNew();							break;			case iOpen:		MOpen();							break;			case iClose:	MClose();							break;			case iSave:		MSave();							break;			case iSaveAs:	MSaveAs();							break;			case iRevert:	MRevert();							break;//			case iDuplicate:MDuplicate();//							break;			case iPageSetup:MPageSetup();							break;			case iPrint:	MPrint();							break;			case iQuit:		MQuit();							break;			default:		{ G(DoMenu)(gFileId,item); }			}		} else if (title == gEditId) {	// Edit-Men�			if (!SystemEdit(item-1)) {#if USETEXTEDIT				if (gTEHandle) {		// TextEdit aktiv?					switch(item-1) {					case cutCmd:	TECut(gTEHandle);									if (ZeroScrap() == noErr) TEToScrap();									break;					case copyCmd:	TECopy(gTEHandle);									if (ZeroScrap() == noErr) TEToScrap();									break;					case pasteCmd:	if (TEFromScrap() == noErr) TEPaste(gTEHandle);									break;					case clearCmd:	TEDelete(gTEHandle);									break;					}				} else#endif					{ G(DoMenu)(gEditId,item); }			}		} else if (title == gWindowsId)			DoWindow(wind,item);	// Windows-Men�		else			{ G(DoMenu)(title,item); }	// sonstiger Men�punkt angew�hlt	}	HiliteMenu(0);}