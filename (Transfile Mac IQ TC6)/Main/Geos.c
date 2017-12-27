////	Geos.c//#include "GlobalDefines.h"#include "DoEvent.h"#include "GeosMore.h"#include "GlobalLib.h"#include "Install.h"#include "Geos.h"#include "rsrcDefines.h"#include "xRsrcDefines.h"#include "Calendar.h"#include "GlobalStruct.h"#include "Float.h"#include "Memo.h"#include "Telephone.h"#include "Outline.h"#include "Business.h"#include "DoList.h"#include "ToDo.h"#include "Expense.h"#include "Userfile.h"#include "Options.h"#include "Menus.h"#include "String.h"#include "IQComm.h"#include "Scheduler.h"#include "Sharp.h"#include "AktDocStruct.h"#include "Suchen.h"#include "DialogLib.h"#include "MyPrint.h"#include "WindowsMenu.h"#include "Print.h"#include "GestaltEqu.h"#include "SoundInput.h"#include "Sound.h"#include "MySound.h"#include "MoreOptions.h"#if __option(profile)#include <stdio.h>#include <profile.h>#endifint				gWindows = 0; 		// Anzahl der offenen WindowsControlHandle	gTheControl;extern WindPtr	gTheFloatWind;SaveEinst		gS;					// globale Einstellungen (Struktur in Geos.h)GlobalStruct	gG;					// Globale Funktionen extern short	gOpenNo[MaxWindowClass];// Nummer der offenen Windows einer Artextern short	gOpenCount[MaxWindowClass];// Anzahl der offenen Windows einer Artextern short	gNoWindowsOpen;			// Anzahl der insgesamt ge�ffneten WindowsVOID		GlobalIdle(VOID);OSErr		AutoLoadYBAX(FSSpec *f,WORD count,WORD whatToDo);OSErr		AutoLoadYDAT(FSSpec *f,WORD count,WORD whatToDo);/*** *	Doppelklick auf eine YBAX-Datei (Backup) ***/OSErr		AutoLoadYBAX(FSSpec *f,WORD count,WORD whatToDo){	if (whatToDo == appOpen)		DoRestore(f);				// Restore vom Organizer	gQuitApplication = true;		// Programm verlassen	return(userCanceledErr);}/*** *	Doppelklick auf eine YDAT-Datei (normales Datenfile) ***/OSErr		AutoLoadYDAT(FSSpec *f,WORD count,WORD whatToDo){OSErr		err = noErr;DocHandle	d;#if ONEDOC	MClose();							// altes Dokument schlie�en	if (gDoc)							// Dokument noch offen?		return(userCanceledErr);		// dann raus!#endif	err = AppOpenDocument(&d,f,fsRdWrPerm);		// neues Dokument anlegen	err = AppInitDocument(d,iOpen,err);			// das Dokument �ffnen	if (CheckOSError(err)) {			// ein Fehler?!		if (whatToDo != appOpen) {		// Drucken?			gPrintPage = 1;				// ab Seite 1 geht es los			err = AppPrintDocument(gDoc,(count == 1),(count == 1));	// das Dokument drucken			CheckOSError(err);			// ein Fehler?!			AppDisposeDocument(gDoc);	// Dokument wieder verwerfen		}	}	gPrintPage = 0;						// keine weiteren Seiten�	return(err);}////	das �Hauptprogramm�//void	main(void){	short		i;	SetMinimumStack(32768);			// Stackgr��e auf 32k setzen//	DebugStr("Activating Discipline; DSCA ON; G;");#if __option(profile)	InitProfile(400,200);	freopen("Profiler Report","w", stdout);#endif	gG.DoAbout = (ProcPtr)DoAbout;	// �About� wurde angeklickt 	gG.DoMenu = (ProcPtr)DoMenu;	// Men�punkt wurde angew�hlt 	gG.InitMenus = (ProcPtr)InstallMenus; // Hierarische Men�s installieren	gG.InitDoc = (ProcPtr)InitDoc;	// Funktion, die die Dokument-Struktur f�llt 	gG.Exit = (ProcPtr)Exit;		// Programmende, IRQs ausklinken 	gG.Print = (ProcPtr)DoPrint;	// Ausdrucken	PrintJobDITL = 155;				// Dialog-ID f�r den Drucker-Dialog	gG.PrintDlg = PrintDialog;		// Drucker-Dialog//	gG.WElement = (ProcPtr)WindElement; // zus�tzliches Window-Element angeklickt //	gG.AdjustMenu = (ProcPtr)OptionsMenuEnable;	// Men�punkte enablen/disablen	gG.Idle = (ProcPtr)GlobalIdle;	// globale Idle-Funktion	gG.FileTypesCount = 1;	gG.FileTypes[0] = docFileType;	// ein (maximal 4!) Filetyp f�r Open, etc 	gG.SleepTime = 5;				// maximale Zeit (in 1/60Hz) zwischen zwei Events 	gG.AutoLoadCount = 2;	gG.AutoLoad[0].Filetype = docFileType;	gG.AutoLoad[0].DoLoad = AutoLoadYDAT;	gG.AutoLoad[1].Filetype = 'YBAX';	gG.AutoLoad[1].DoLoad = AutoLoadYBAX;	InitAll();						// Utilities, etc. init 	WindowInit();					// Window-Library init 	for( i=0 ; i<MaxWindowClass ; i++)	// Alle Window-Nummern auf 0	{		gOpenNo[i]=gOpenCount[i]=0;	}	InitDocument();					// Dokumenten-Struktur anmelden 	MenuBarInit(MENU_ID);			// Men�leiste anmelden 	LoadInstall();					// Einstellungen laden 	gSaveSettings = false;			// Einstellungen nicht sichern 	if(gS.version!=PREF_VERSION)	// Preferences nicht gefunden 		InitInstall();	InitMenuChecks();				// H�kchen plazieren 	SharpInit();					// Sharp-Men� init	EventLoop();	//	SaveInstall();					// Einstellungen speichern //	DebugStr("Turning Discipline off; DSCA OFF; G;");	G(Exit)();						// Programmende }////	Programmende nach schweren Fehlern �IRQs abmelden�//void	Exit(void){	ExitToShell();					// letzter Befehl: Programm verlassen }////	Hierarische Men�s installieren//void InstallMenus(void){	short		anz,i,font,saveFont,style;	Str255		s,t1,t2;	MenuHandle	m;	GrafPtr		graf;//	Point		p;//	Boolean		TestOutline=false;	for(anz = LOWER_HIER_LIMIT; anz < UPPER_HIER_LIMIT; anz++){		// Tempor�r portiert aus SF 		m = GetMenu(anz);		if(m)			InsertMenu(m, -1);	}	// Font-Men�	m = GetMHandle(mFont);					// Handle des Font-Men�s	CheckResource((Handle)m);	AddResMenu(m,'FONT');					// Fonts anh�ngen	anz=CountMItems(m);		GetPort(&graf);	saveFont=graf->txFont;					// Alten Font merken		GetIndString(t1,strSearchStrings,strProp1);	// Zwei Vergleichstexte	GetIndString(t2,strSearchStrings,strProp2);	for( i=1 ; i<=anz ; i++ )	{		GetItem(m,i,s);						// Menu-Text holen		GetFNum(s,&font);		TextFont(font);						// Fonts der Reihe nach durchprobieren		style=0;		if(TextWidth(t1,0,t1[0])==TextWidth(t2,0,t2[0]))	// Proportional?			style=underline;//		if(TestOutline)						// Gibt's Outline-Fonts?//			if(IsOutline(p,p))//				style|=outline;		SetItemStyle(m,i,style);			// Dann Outlined darstellen	}	TextFont(saveFont);						// Originalfont restaurieren#if	!BETA || !CACHE	m=GetMenu(mOptionen);	for( i=0 ; i<4 ; i++ )		DelMenuItem(m,optDummy3);#endif}////	Preferences setzen, wenn Pref-Datei nicht vorhanden//void	InitInstall(void){	Intl0Hndl	res;	short		language,i;		gS.version=PREF_VERSION;			// Ab jetzt sind sie vorhanden 	res=(Intl0Hndl)IUGetIntl(0);		// Vers-Resource f�r dieses Programm 	if(res)								// Wenn vorhanden, dann Sprachnummer kopieren 	{		language=(**res).intl0Vers>>8;		if(language<13)					// Sprachcode in Men�nummer umrechnen 			gS.language=language+LANG_OFFSET;		else		{			if(language==15)			// Australien 				gS.language=13+LANG_OFFSET;			else				if((language>=17) && (language<=19))					gS.language=language-3+LANG_OFFSET;		}	}									// Sonst bleibt's 0 = verUS 	for( i=0 ; i<MaxIQFileType ; i++ )		gS.display[i]=255;				// 8 Felder sichtbar	GetFontName(applFont,gS.font);		// applFont ist default-Font	gS.fontSize=12;						// default-Size = 12pt	gS.FloatShouldBeOpen=false;			// Float ist nicht offen	gS.fieldNames=true;					// Drucken mit Feldnamen	gS.printHeader=true;				// Header mitdrucken	gS.columnsPerPage=1;				// Einspaltiger Druck}////	�About�� aus dem Drop-Down-Men� wurde angew�hlt.//void	DoAbout(void){GDHandle			g;Point				myCorner;Handle				hndl;Str255				s,defaultText;StandardFileReply	reply;short				refNum,type;SFTypeList			typeList[2]={'AIFF','AIFC'};OSType				quality[3]={siGoodQuality,siBetterQuality,siBestQuality};OSType				fileType;static SndChannelPtr	chan=nil;OSErr				ret;#if __option(profile)	if(!gAppTrue && (gTheEvent.modifiers & controlKey))	{		DumpProfile();		alert(1,"[1][Profile has been written.][Great!]");		return;	}#endif	if(!gAppTrue && (gTheEvent.modifiers & optionKey))	{		if(!(GetGestaltResult(gestaltSoundAttr)&24) ||			!(GetGestaltResult(gestaltStandardFileAttr)&(1L<<gestaltStandardFile58)))				return;						// Hier fehlt was am System		if(chan)				SndDisposeChannel(chan,true);		// Sound-Channel wieder freigeben		if(alert(2,"[2][Moin! Worum geht's?][Record|Play]")==1)		{						type=alert(2,"[2][Welche Qualit�t w�re genehm?][Good|Better|Best]");						CopyPString(s,"\pWie soll der Sound hei�en?");	// Text �ber dem File-Selektor			CopyPString(defaultText,"\pGeiler Sound");		// Default-Name			StandardPutFile(s,defaultText,&reply);			if(!reply.sfGood)				// File-Selektor wurde abgebrochen				return;							// Datei anlegen. Best Quality hat FileType 'AIFF', die anderen 'AIFC'			if(type<3)				fileType='AIFC';				// Compressed			else				fileType='AIFF';				// Nicht Compressed							if(FSpCreate(&(reply.sfFile),'????',fileType,reply.sfScript)!=noErr)				return;						if(FSpOpenDF(&(reply.sfFile),fsRdWrPerm,&refNum)!=noErr) // Data-Fork �ffnen				return;						SetPt(&myCorner,50,50);			// Hier soll die Dialogbox hin			SndRecordToFile(nil,myCorner,quality[type-1],refNum);						FSClose(refNum);		}else		{			StandardGetFile(nil,2,(OSType*)typeList,&reply);			if(!reply.sfGood)				// File-Selektor wurde abgebrochen				return;				if(FSpOpenDF(&(reply.sfFile),fsRdPerm,&refNum)!=noErr) // Data-Fork �ffnen				return;						// Einen Sound-Channel anlegen, da wir sonst nicht asynchron spielen k�nnen			chan=nil;		// Sound Channel in den Application-Heap			if(SndNewChannel(&chan,sampledSynth,initMono,nil)==noErr)				// 200000 ist unsere Buffergr��e (wird alloziert), true hei�t asynchron				SndStartFilePlay(chan,refNum,0,200000,nil,nil,nil,false);						FSClose(refNum);		}	}	hndl=GetResource('vers',1);	if(hndl)		ParamText((*(VersRec**)hndl)->shortVersion,nil,nil,nil);	if(gQDVersion) {					// Color Quickdraw vorhanden? 		g = GetMainDevice();			// aktuellen Screen holen		if ((*(*g)->gdPMap)->pixelSize > 1) { // mehr als s/w?	 		DoDialog(DLOGcolabout,1);	// dann den Farb-Dialog 			return; 		}	}#if BETA	OwnBeep(about);#endif	DoDialog(DLOGabout,1);}////	f�r die Library unbekanntes Men� wurde angew�hlt//Boolean			PersDateFormat(void);void	DoMenu(int title,int entry){	MenuHandle		menuH;	Handle			xWindow;	extern void		DoImport(void);extern void		DoExport(void);	switch(title)	{		case 0:					// ignorieren; nix wurde angeklickt 			break;		case mAblage:			switch(entry) {			case iImport:					DoImport();					UnloadSeg(DoImport);					break;			case iExport:					DoExport();					UnloadSeg(DoExport);					break;			}			break;		case mBearbeiten:			if (entry < mEditLINE2) {				DoCutCopy(entry);			} else				BearbeitenMenu(entry);			break;		case mDatum:			// Date & Time-Format 			if (entry == 5)						// pers�nliches Format?				if (!PersDateFormat())			// Ja!					break;						// es wurde �Abbruch� gew�hlt�			menuH=GetMHandle(mDatum);			// CheckMark anpassen:			CheckItem(menuH,gS.language,false);	// Erst alten entfernen			gS.language = entry;				// neue Sprache setzen			gSaveSettings = true;				// Einstellungen sichern			CheckItem(menuH,entry,true);		// Dann neuen setzen			DoMultiRedraw(SCHEDULE, redrawAll, nil);			DoMultiRedraw(ANN1, redrawAll, nil);			DoMultiRedraw(TODO, redrawAll, nil);	// Alles redrawn, wo eine Zeitangabe vorkommt			DoMultiRedraw(DOLIST, redrawAll, nil);			DoMultiRedraw(EXPENSE, redrawAll, nil);			break;		case mSonstiges:			SonstigesMenu(entry);			break;		case mSharp:			SharpMenu(entry);			break;		case mSharpTyp:			SharpMenuTyp(entry);			break;		case mSharpSerMenu:			SharpSchnittstelle(entry);			break;		case mOptionen:			OptionsMenu(entry);			break;		case mFont:			FontMenu(entry);			break;		case mFontsize:			FontSizeMenu(entry);			break;		case mGrossKlein:			DoMark(entry+strSSGross-grGross,strSearchStrings);			break;		case mAttribute:			DoMark(entry,strAttributes);			break;		default:			alert(1,"[1][An unknown menu|has been selected][IGNORE]");	}}////	H�kchen beim Programmstart auf default-Werte setzen//void	InitMenuChecks(void){	MenuHandle		m;	Str255			s,s2;	long			num;	short			i,anz,size,font;	Boolean			setSomething=false;//	Point			thePt={8,1},thePt2={16,1};	m=GetMHandle(mDatum);					// CheckMark im Datumsmen� 	CheckItem(m,gS.language,true);		// FontSizes testen und Style festlegen	m = GetMHandle(mFontsize);				// Handle des FontSize-Men�s	CheckResource((Handle)m);	if(gDoc)								// Gibt es ein Dokument?	{		CopyPString(s2,ADOC.font);			// Dann Font des Dokuments,		size=ADOC.fontSize;	}	else	{		CopyPString(s2,gS.font);			// Sonst aus den Preferences.		size=gS.fontSize;	}	GetFNum(s2,&font);	anz=CountMItems(m)-1;					// Alle Items exkl. "Other�"	for( i=1 ; i<anz ; i++ )	{		GetItem(m,i,s);						// Menu-Text holen		StringToNum(s,&num);		if(RealFont(font,(short)num))		// Existiert die Font-Gr��e?			SetItemStyle(m,i,outline);		// Dann outlined darstellen		if((short)num==size)		{			CheckItem(m,i,true);			// aktuelle Gr��e? Dann abhaken			setSomething=true;		}		else			CheckItem(m,i,false);	}	if(!setSomething)						// Akt. Fontgr��e nicht gefunden?		CheckItem(m,anz+1,true);			// Dann "Andere" abhaken//	if(IsOutline(thePt,thePt2))				// Ist das ein Outline-Font?//		SetItemStyle(m,anz+1,outline);		// Dann outlined darstellen	// Fonts testen und aktuellen abhaken	m = GetMHandle(mFont);					// Handle des Font-Men�s	anz=CountMItems(m);	for( i=1 ; i<=anz ; i++ )	{		GetItem(m,i,s);						// Menu-Text holen		if(ComparePString(s2,s))			CheckItem(m,i,true);			// aktueller Font? Dann abhaken	}	#if BETA && !CACHE	m = GetMHandle(mOptionen);					// Handle des Font-Men�s	DisableItem(m,optCacheDisable);	DisableItem(m,optClearCache);	DisableItem(m,optCheckCache);#endif}////	eigene �interne� Funktionen f�r Dokumente//OSErr	DoNew(void);OSErr	MyOpen(void);OSErr	MyRevert(void);Boolean	MyWindow(long);void	MyClose(void);OSErr	SwitchFont(void);////	Diese Funktion wird aufgerufen, wenn eine neue Dokumentenstruktur//	angelegt wurde und die Dokumenten-Funktionen vom Hauptprogramm auf-//	zuf�llen sind. Da das Einsetzen in die Stuktur stets fehlerfrei//	klappt, gibt es keinen R�ckgabewert. In dieser Funktion darf NICHTS//	anderes gemacht werden.//void	InitDoc(REG DocHandle d)	// Dokument erzeugen {	DFUNC(d,new,DoNew);				// neues Dokument erzeugen	DFUNC(d,open,MyOpen);			// Dokument �ffnen	DFUNC(d,revert,MyRevert);		// Dokument wurde neu geladen	DFUNC(d,close,MyClose);			// Dokument schlie�en	DFUNC(d,window,MyWindow);		// Window zum Dokument �ffnen	DFUNC(d,save,SharpSave);		// MRF: Dokument speichern	DFUNC(d,load,SharpLoad);		// MRF: Dokument laden}////	neues Dokument erzeugen//OSErr	DoNew(void){ 	OSErr		err;//	short		i,anz;//	MenuHandle	m;//	Str255		s;	SetHandleSize(AKTDOC.data,sizeof(AktDocStruct));	if (MemError())	return(memFullErr);	// Speicher reichte nicht� 	HLock(AKTDOC.data);	memset(*AKTDOC.data,0,sizeof(AktDocStruct));	HUnlock(AKTDOC.data);	IQSetDefault((OrganizerH)AKTDOC.data);	// MRF: Default-Organizer setzen	ADOC.OwFlag = gS.OwFlag;		// Append/�berschreiben-Flag	BlockMove(gS.Idx,ADOC.Idx,sizeof(gS.Idx));	// selektierte Eintr�ge	BlockMove(gS.display,ADOC.display,sizeof(gS.display));	// selektierte Eintr�ge	CopyPString(ADOC.font,gS.font);	ADOC.fontSize=gS.fontSize;	ADOC.fieldNames=gS.fieldNames;	ADOC.printHeader=gS.printHeader;	ADOC.columnsPerPage=gS.columnsPerPage;	// Nicht vorher einklinken, da sonst vor der Initialisierung schon aufgerufen	DFUNC(gDoc,activate,SwitchFont);	// Beim Dokumentenwechsel den akt. Font �ndern	SwitchFont();	err = OpenCalendarWindows();			// Fenster �ffnen 	if (err != noErr)		DisposHandle(AKTDOC.data);			// Speicher freigeben 	return(err);							// alles ok! }////	neues Dokument �berladen//OSErr	MyRevert(void){	SetSharpTyp((*GetOrgH(gDoc))->type);}////	neues Dokument �ffnen//OSErr	MyOpen(void){//	LockHandleHigh(AKTDOC.data);	// f�r IRQ-Routinen!!! 	// Nicht vorher einklinken, da sonst vor der Initialisierung schon aufgerufen	DFUNC(gDoc,activate,SwitchFont);	// Beim Dokumentenwechsel den akt. Font �ndern	SwitchFont();	return(OpenCalendarWindows());}////	eigene Funktion: Dokument schlie�en, Speicher freigeben//void	MyClose(void){	SharpClrAll(GetOrgH(gDoc));}OSErr	SwitchFont(void){	short		i,anz;	MenuHandle	m;	Str255		s;	Boolean		setSomething,oldDirty;	long		num;	// Fonts testen und aktuellen abhaken	m = GetMHandle(mFont);					// Handle des Font-Men�s	anz=CountMItems(m);	for( i=1 ; i<=anz ; i++ )	{		GetItem(m,i,s);						// Menu-Text holen		if(ComparePString(ADOC.font,s))		// = Richtiger Font?			CheckItem(m,i,true);			// angeklickter Font? Dann abhaken		else			CheckItem(m,i,false);			// Sonst Haken l�schen	}	// Desgleichen f�r die Fontgr��e	setSomething=false;	m = GetMHandle(mFontsize);				// Handle des Font-Men�s	anz=CountMItems(m)-1;	for( i=1 ; i<anz ; i++ )	{		GetItem(m,i,s);						// Menu-Text holen		StringToNum(s,&num);		if((short)num==ADOC.fontSize)		{			CheckItem(m,i,true);			// aktuelle Gr��e? Dann abhaken			setSomething=true;		}		else			CheckItem(m,i,false);			// Sonst Haken l�schen	}	if(!setSomething)						// Akt. Gr��e nicht im Men�?		CheckItem(m,anz+1,true);			// Dann "Andere" abhaken	else		CheckItem(m,anz+1,false);			// Sonst nicht	return(noErr);}////	eigene Funktion: Window �ffnen//	Diese Funktion wird aufgerufen, wenn im Drop-Down-Men� eine Windowklasse//	angew�hlt wurde. Man hat dann das zur Klasse geh�rende Window zu �ffnen.//Boolean	MyWindow(long id){	REG OSErr	err;	switch(id)	{		case WindFloat://			if(gTheFloatWind)				// Float schon offen?//			{//				CloseWind(gTheFloatWind);	// Dann schlie�en//			}else			ShowFloat(gTopWindow);			// wird f�r Buttonart gebraucht.			err=noErr;						// klappt immer			break;		case WindCalendar:			err = OpenCalendarWindows();	// Kalender-Fenster �ffnen 			break;		case WindSchedule:			err = OpenSchedWindows();		// Scheduler-Fenster �ffnen 			break;		case WindMemo:			err = OpenMemoWindows();		// Memo-Fenster �ffnen 			break;		case WindTelephone:			err = OpenTelWindows();			// Telefon-Fenster �ffnen 			break;		case WindBusiness:			err = OpenBusinessWindows();	// Business-Card-Fenster �ffnen 			break;		case WindOutline:			err = OpenOutlineWindows();		// Outliner-Fenster �ffnen 			break;		case WindDoList:			err = OpenDoListWindows();		// Do-List-Fenster �ffnen 			break;		case WindExpense:			err = OpenExpenseWindows();		// Expense-Manager-Fenster �ffnen 			break;		case WindUserFile:			err = OpenUserfileWindows();	// UserFile-Fenster �ffnen 			break;		case WindToDo:			err = OpenToDoWindows();		// ToDo-Fenster �ffnen 			break;		default:			alert(1,"[3][This window cannot be opened|because it's not implemented!][ABORT]");			return(false);	}	if(err==memFullErr)		DoDialog(DLOGOutOfMemory,1);		// Kein Speicher f�r ein neues Window	return(err==noErr);}////	eigene idle-Routine, die in allen Windows einen Z�hler ausgibt//#if 0void	dummy(void){Str63 			s1;static	int		count = 0;	NumToString(count++,(StringPtr)&s1);//	utext(65,65,(char*)&s1);}#endifvoid	testidle(void){//	WindowAktion((WindPtr)gMacWind,-1,(long)dummy);	// in allen Windows! }void	testclose(void){	short		id=gMacWind->class;	Handle		xWindow;		xWindow=gMacWind->MoreMem;		if((XWIN->windowType==ANN1) && (XWIN->theUnion.schedule.l))		DisposeDay(gMacWind);	if(!(--gOpenCount[id]))				// Jetzt alle Windows dieser Art geschlossen		gOpenNo[id]=0;#if CACHE	if(XWIN->cacheEnable && XWIN->theUnion.cache)	// Ist ein Cache angemeldet?		DisposHandle((Handle)XWIN->theUnion.cache);	// Dann sollten wir ihn freigeben#endif	DisposHandle((Handle)xWindow);		// Die ganze xWindStruct freigeben	TestHideFloat();	if(--gNoWindowsOpen<0)				// Ein Window weniger offen		SysBeep(BEEPTIME);				// Weniger als 0 = Fehler}////	Wenn ein Window getopped wird, dann mu� das Float upgedated werden//void	testactivate(void){	if(gTheFloatWind)		UpdateFloat(gMacWind);}////	globale Idle-Funktion z.B. zum enablen bzw. disablen von Men�titeln//void	GlobalIdle(void){REG Boolean		redraw = false;REG MenuHandle	m;	if (!gDoc) {		m = GetMHandle(mBearbeiten);		if (GetMenuAble(m,0)) {			DisableItem(m,0);			redraw = true;		}		m = GetMHandle(mSonstiges);		if (GetMenuAble(m,0)) {			DisableItem(m,0);			redraw = true;		}		m = GetMHandle(mOptionen);		if (GetMenuAble(m,0)) {			DisableItem(m,0);			redraw = true;		}		m = gWindowMenu;		if (GetMenuAble(m,0)) {			DisableItem(m,0);			redraw = true;		}		m = GetMHandle(gFileId);		DisableItem(m,iImport);		DisableItem(m,iExport);	} else {		m = GetMHandle(mBearbeiten);		if (!GetMenuAble(m,0)) {			EnableItem(m,0);			redraw = true;		}		m = GetMHandle(mSonstiges);		if (!GetMenuAble(m,0)) {			EnableItem(m,0);			redraw = true;		}		m = GetMHandle(mOptionen);		if (!GetMenuAble(m,0)) {			EnableItem(m,0);			redraw = true;		}		m = gWindowMenu;		if (!GetMenuAble(gWindowMenu,0)) {			EnableItem(gWindowMenu,0);			redraw = true;		}		m = GetMHandle(gFileId);		EnableItem(m,iImport);		EnableItem(m,iExport);	}	if (redraw) DrawMenuBar();}////	TextEdit-Eingabefeld beenden//#if USETEXTEDITvoid	TEExit(void){	InvalRect(&((**gTEHandle).viewRect));	TEDispose(gTEHandle);	gTEHandle = nil;	gTEWindow = nil;}#endif