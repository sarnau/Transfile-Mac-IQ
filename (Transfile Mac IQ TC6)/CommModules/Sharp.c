#include "Sharp.h"#include "Geos.h"#include "GeosMore.h"#include "IQComm.h"#include "SerialComm.h"#include "DoEvent.h"#include "rsrcDefines.h"#include "xRsrcDefines.h"#include "Install.h"#include "GlobalLib.h"#include "GlobalStruct.h"#include "Document.h"#include "Menus.h"#include "Windows.h"#include "Utilities.h"#include "File.h"#include "List.h"#include "Memo.h"#include "DialogLib.h"#include "MeterWindow.h"#include "WindowsMenu.h"#include <Serial.h>#include <String.h>#include "MySound.h"#include "CheckList.h"#include "code/decode.h"#include "AktDocStruct.h"#include <stdio.h>#define DOCODING	1		// True, wenn unkodiert abgespeichert wird, wenn kein Passwort vorhanden#define	VERSIONNO	1		// aktuelle Version#define MAGIC		0xa3b75f36	// ein beliebiges Magictypedef struct {	short	version;		// Versionsnummer	long	magic;			// kodiertes Magic-Wort (mu� == MAGIC sein)} FHeader;Boolean	DoCoding = true;OSErr	FSWriteCod(short fileRefNum,long *count,void *buf);OSErr	FSReadCod(short fileRefNum,long *count,void *buf);/*** *	kodierte Bytes schreiben ***/OSErr	FSWriteCod(short fileRefNum,long *count,void *buf){REG OSErr	err;	if (*count == 0L) return(noErr);			// nix zu schreiben	if(DoCoding)		code_data(buf,*count,AKTDOC.f.Passwort);	err = FSWrite(fileRefNum,count,buf);		// codierten Block schreiben	if(DoCoding)		decode_data(buf,*count,AKTDOC.f.Passwort);	return(err);}/*** *	Einen Datenblock (Handle) wegschreiben ***/OSErr		SharpWriteBlock(REG Handle h,REG short fileRefNum){REG OSErr	err;long		count;long		temp;char		hstate;	if (h == nil) {							// unbenutzer Block		count = sizeof(long); temp = 0L;		err = FSWriteCod(fileRefNum,&count,&temp);	// L�nge = 0		return(err);	}	hstate = HGetState(h);	HLock((Handle)h);	count = sizeof(long); temp = GetHandleSize((Handle)h);	err = FSWriteCod(fileRefNum,&count,&temp);	// L�nge vom Directory	if (err) return(err);	count = temp;	err = FSWriteCod(fileRefNum,&count,*h);	// einen Datensatz schreiben	HSetState(h,hstate);	return(err);}/*** *	Organizer-Daten wegschreiben speichern ***/OSErr	SharpWrite(REG OrganizerH o,REG short fileRefNum){REG OSErr	err;REG ListH	*l;REG ListH	lh;	{	Handle	h = (Handle)GetResource('FEAT',128);	if(h && (*h)[0])		DoCoding = AKTDOC.f.Passwort[0] != 0;	if(h)		ReleaseResource(h);	}	{ long			count = sizeof(FHeader);	FHeader	f = { VERSIONNO,MAGIC };	err = FSWriteCod(fileRefNum,&count,&f);	// den Header schreiben	if (err) return(err);	}	err = SharpWriteBlock((Handle)o,fileRefNum);	// Organizer-Struktur schreiben	if (err) return(err);	err = SharpWriteBlock((Handle)((*o)->dir),fileRefNum);	// Directory schreiben	if (err) return(err);	l = &((*o)->schedule);				// Ptr auf die erste Listhandle	while(l != &((*o)->Null)) {			// Ende vom ListHandle-Array erreicht?		lh = *l;		if (lh == nil) {				// kein Datensatz in dieser Datei?			err = SharpWriteBlock((Handle)lh,fileRefNum);	// leeren Datensatz schreiben			if (err) return(err);		}		while(lh) {						// alle Listenelemente durchgehen			err = SharpWriteBlock((Handle)lh,fileRefNum);	// Datensatz schreiben			if (err) return(err);			lh = (*lh)->next;		}		l++;	}	{ long	count = Random() & 0xFF; Str255 s;	// zuf�llige Anzahl an Bytes anh�ngen	err = FSWriteCod(fileRefNum,&count,&s);		// den Header schreiben	if (err) return(err);	}	{	short i; Str255 s; long	count;		vStrcpy((char*)s,(char*)AKTDOC.f.Passwort);	// Passwort �bertragen		CtoPstr((STR)s);		for(i=1;i<=s[0];i++)					// gesamten String durchgehen			s[i] ^= 0x45;		count = s[0]+1;							// L�nge vom Passwort		err = FSWrite(fileRefNum,&count,&s);	// den Header schreiben		if (err) return(err);	}	return(err);}/*** *	Sharp-Dokument speichern ***/OSErr	SharpSave(REG DocHandle d){REG OSErr	err;REG AktDocH	ad = (AktDocH)(*d)->data;	// Handle der AktDoc-Strukur	HLock((Handle)ad);	err = SharpWrite((OrganizerH)ad,(*d)->f.refNum);	HUnlock((Handle)ad);	return(err);}/*** *	kodierte Bytes einlesen und dekodieren ***/OSErr	FSReadCod(short fileRefNum,long *count,void *buf){REG OSErr	err;	if (*count == 0L) return(noErr);		// nix zu schreiben	err = FSRead(fileRefNum,count,buf);		// codierten Block einlesen	if(DoCoding)		decode_data(buf,*count,AKTDOC.f.Passwort);	// und dekodieren	return(err);}/*** *	Einen Datenblock (Handle) einlesen ***/OSErr		SharpReadBlock(REG Handle *h,REG short fileRefNum,REG Boolean flag){REG OSErr	err;long		temp;long		count;char		hstate;	count = sizeof(long);	err = FSReadCod(fileRefNum,&count,&temp);	// Header einlesen	if (err) return(err);	if (count != sizeof(long)) return(ioErr);	if (temp == 0L) return(26);				// ein leerer Eintrag?	if (flag)								// alte Handle nutzen?		SetHandleSize((Handle)*h,temp);	else		*h = NewHandle(temp);				// Speicher f�r den Block allozieren	err = MemError();	if (err) return(err);	hstate = HGetState(*h);	HLock((Handle)*h);	count = temp;	err = FSReadCod(fileRefNum,&temp,**h);	// Block einlesen	if (temp != count) return(ioErr);	HSetState(*h,hstate);	return(err);}/*** *	Sharp-Dokument laden ***/OSErr	SharpLoad(REG DocHandle d){REG WORD		fileRefNum = (*d)->f.refNum;REG OSErr		err;OrganizerH		o;						// Ptr auf die Organizer-StrukturREG ListH		*li,*l,lp;REG LONG		len;	{ long count = sizeof(FHeader); FHeader f; Boolean first = true;	while(true) {		DoCoding = false;		do {			SetFPos(fileRefNum,fsFromStart,0L);	// Ptr auf Dateianfang			err = FSReadCod(fileRefNum,&count,&f);	// den Header einlesen			if (err) return(err);			// Lesefehler => raus			if (f.magic == MAGIC) break;	// Passwort erkannt!			if (DoCoding) break;			// zweiter Versuch durch�			DoCoding = true;				// neues Format probieren		} while(1);		if (f.magic != MAGIC) {				// Passwort unbekannt			WORD	item;			if (!first) OwnBeep(negative);			item = PasswortDialog(167);			if (item != 1) return(userCanceledErr);	// Abbruch			first = false;			continue;		}		if (f.version != VERSIONNO) {		// Format unbekannt			OwnBeep(negative);			DoDialog(159,1);				// Fehler melden			return(userCanceledErr);		// Abbrechen ohne Fehlercode!		}		break;								// abbrechen	}	}	o = GetOrgH(gDoc);						// Handle auf die Organizer-Struktur	len = GetHandleSize((Handle)o);					// Sind �berhaupt Daten vorhanden?	if (len>0)		SharpClrAll(o);						// alle Strukturen verwerfen	err = SharpReadBlock((Handle*)&o,fileRefNum,true);	// AktDoc-Struktur einlesen	if (err<0) return(err);	if (err == 26) return(ioErr);			// Datei defekt!!!	SetHandleSize((Handle)o,sizeof(AktDocStruct));	// auf die WIRKLICHE Gr��e von AktDocStruct	err = MemError();						// aufblasen.	if (err) return(err);	li = &((*o)->schedule);					// Ptr auf die erste Listhandle	while(li != &((*o)->Null))				// Ende vom ListHandle-Array erreicht?		*li++ = nil;						// alle evtl. Datenhandles l�schen	HLock((Handle)o);	err = SharpReadBlock(&((*o)->dir),fileRefNum,false); // Directory einlesen	if (err<0) {		HUnlock((Handle)o);		return(err);	}	if (err == 26) (*o)->dir = nil;		// kein Directory vorhanden	li = &((*o)->schedule);				// Ptr auf die erste Listhandle	while(li != &((*o)->Null)) {		// Ende vom ListHandle-Array erreicht?		lp = nil;						// kein Vorg�nger		l = li;							// ab hier geht es los		do {							// alle Listenelemente durchgehen			if (lp) {				HLock((Handle)lp);				// nicht der erste Datensatz? => Vorg�nger locken!				(*lp)->next = nil;			}			err = SharpReadBlock((Handle*)l,fileRefNum,false);			if (err<0) {				if (lp) HUnlock((Handle)lp);				HUnlock((Handle)o);				return(err);			}			if (err == 26) break;		// keine Daten im Block => Abbruch			if (lp) HUnlock((Handle)lp);			(**l)->prev	= lp;			// Vorg�nger setzen			if (lp)				(*lp)->next	= *l;		// Nachfolger setzen			lp	= *l;					// als n�chsten Vorg�nger merken			l	= &(**l)->next;			// hier kommt der n�chste Datensatz hin�		} while ((*l) != nil);			// letzter Datensatz		li++;	}	IQSetDefault(o);	// Organizer-Variablen sicherheitshalber neu setzen	HUnlock((Handle)o);	return(noErr);}/*** *	Wird nach dem Anmelden der Men�leiste, Laden der Infos, etc. aufgerufen ***/Boolean	SDCBAppendMenu(WORD index,USTR portname,USTR indriver,USTR outdriver);Boolean	SDCBAppendMenu(WORD index,USTR portname,USTR indriver,USTR outdriver){	AppendMenu(GetMenu(mSharpSerMenu),portname);	return(false);}void	SharpInit(void){	SDLookup(SDCBAppendMenu);		// Treibernamen ans Men� anh�ngen	if (!gS.serPort[0])				// kein Port aktiv?		SharpSchnittstelle(1);		// ersten Port w�hlen	if (!gS.aktOrganizer)		gS.aktOrganizer = ZQ5000;	// Default-Organizer	if (!gS.stadt) {		Str255		s;		LONG		num;		GetIndString(s,1002,1);		// Default-Stadt ermitteln		StringToNum(s,&num);		gS.stadt = num;				// Berlin ist Default	}	SharpMUpdate();					// Men� updaten}/*** *	Sharp-Men�s updaten ***/void	SharpMUpdate(void){REG MenuHandle	ms = GetMHandle(mSharp);REG MenuHandle	mss = GetMHandle(mSharpTyp);static CHAR		md[] = {-mSharpSend,-mSharpReceive,-mSharpBackup,-mSharpRestore,-mSharpClear,0};static CHAR		mAll[] = {mSharpSend,mSharpReceive,mSharpBackup,mSharpRestore,					mSharpClear,mSharpSub,mSharpSub2,0};Str63			s;	MenuAble(mSharp,mAll);						// alles enablen	if (gDoc == nil) {							// kein Dokument vorhanden!		DisableItem(ms,mSharpSend);		DisableItem(ms,mSharpReceive);		DisableItem(ms,mSharpClear);		DisableItem(ms,mSharpSub);	}	CopyPString(SDName,gS.serPort);				// aus den Einstellungen holen	{	REG MenuHandle	msser = GetMHandle(mSharpSerMenu);	// Men� der Schnittstellen	WORD			i;		for(i=1;i<99;i++) {						// gew�hlte Einstellung abhaken			GetItem(msser,i,(StringPtr)&s);			CheckItem(msser,i,ComparePString(s,SDName));		}	}	CheckItem(ms,mSharpNach,gS.OffFlag==true);	{	REG WORD		i,j;		REG MenuHandle	m = gWindowMenu;		// Handle vom Window-Men�		i = gS.aktOrganizer;					// kein Dokument offen		if (gDoc) {			REG OrganizerH	o = GetOrgH(gDoc);	// Handle auf den Organizer ermitteln			if (o)				i = (*o)->type;					// Typ vom aktuellen Dokument		}		CheckItem(mss,1,(i & IQALL) == ZQ5000);			// aktuellen Organizer abhaken		CheckItem(mss,2,(i & IQALL) == IQ7000);		CheckItem(mss,3,(i & IQALL) == IQ7600);		CheckItem(mss,4,(i & IQALL) == IQ8000);		CheckItem(mss,5,(i & IQALL) == IQ8400);		CheckItem(mss,7,(i & RAMCARD) == RAMCARD);		CheckItem(mss,8,(i & TIMEEXPCARD) == TIMEEXPCARD);		CheckItem(mss,9,(i & SPREATSHEET) == SPREATSHEET);		if ((i & IQALL) == ZQ5000) {			// ein 5000er hat keine Karten!			DisableItem(mss,7);			DisableItem(mss,8);			DisableItem(mss,9);		} else {								// sonst sind Karten erlaubt			EnableItem(mss,7);			EnableItem(mss,8);			EnableItem(mss,9);		}		DisableItem(m,j = MenuId(6,nil));		if (i & IQGetOrgMask(BUSINESS))			EnableItem(m,j);					// Business-Card		DisableItem(m,j = MenuId(7,nil));		if (i & IQGetOrgMask(OUTLINE))			EnableItem(m,j);					// Outliner		DisableItem(m,j = MenuId(8,nil));		if (i & IQGetOrgMask(DOLIST))			EnableItem(m,j);					// Do List		DisableItem(m,j = MenuId(9,nil));		if (i & IQGetOrgMask(EXPENSE))			EnableItem(m,j);					// Expense Manager		DisableItem(m,j = MenuId(10,nil));		if (i & IQGetOrgMask(USERFILE1))			EnableItem(m,j);					// User Files		DisableItem(m,j = MenuId(11,nil));		if (i & IQGetOrgMask(TODO))			EnableItem(m,j);					// ToDo	}}/*** *	Sharp-Typ setzen, ggf. warnen ***/#include "WindowsGlobal.h"void	SetSharpTyp(REG UWORD typ){REG WORD		i;REG WindowPeek	w;REG OrganizerH	o = GetOrgH(gDoc);IQFileType		GetWindowType(REG WindPtr w);	(*o)->type = typ;			// Typ im aktuellen Dokument setzen	IQSetDefault(o);	AppDocumentDirty(gDoc,true);// Dokument ist �dirty�	gS.aktOrganizer = typ;	gSaveSettings = true;		// Einstellungen dirty	SharpMUpdate();				// Men� updaten	again:	w = LMGetWindowList();		// Handle des ersten Windows	while (w) {					// Ende der Windowliste?		if (IsAppWindow((WindowPtr)w)&&		// ein eigenes Window?			(((WindPtr)w)->Doc == gDoc)&&	// geh�rt das Window zu diesem Dokument?			(!IQIsAvailable(o,GetWindowType((WindPtr)w)))) {	// Typ aber unbekannt?				CloseWind((WindPtr)w);		// Window schlie�en				goto again;		// Liste erneut durchgehen		}		w = w->nextWindow;		// Nein: ein Window weiter	}	CheckDoc(gDoc);				// Daten abtesten	DoBitFieldRedraw(-1L);}/*** *	Daten senden ***/void	SendData(void){REG OrganizerH	o = GetOrgH(gDoc);REG UWORD		i;REG OSErr		err;REG DialogPtr	d;						// Ptr auf diverse Dialoge	d = GetCenteredDialog(dSharpMessage,nil);	if (d) {		ShowDItem(d,1);		HideDItem(d,2);		HideDItem(d,3);		SetUserItem(d,6,mWindowDraw);		DrawDialog(d);	}	biene();	CheckDoc(gDoc);	err = SDOpen();					// serielle Schnittstelle (Port A) �ffnen	if (!err) {		Str255		s;		UWORD		saveTimeout = SDTimeout;		char		CLstr[128];		short		dataanz;		SDTimeout = 5*60;			// 5s Timeout		vStrcpy(CLstr,"c03=t%");		GetPopupCond(gS.SendFlag,CLstr);		mWakt = 0;		mWmax = 1;		CLstr[2] = '0';				// Funktion 0: z�hlen		for(i=0;i<MaxIQFileType-1;i++) {			if (ADOC.Idx[i]==true) {			// Daten senden?				CLCount = 0;				SearchOList(o,i+1,CLstr,i+1);	// Daten senden				mWmax += CLCount;			}		}		CLstr[2] = '3';				// Funktion 3: Unterprogramm aufrufen		err = noErr;		if ((ADOC.TimeSend)&&(NewLinkFormat(gDoc)))			err = IQSetTime(o);		for(i=0;i<MaxIQFileType-1;i++) {			if (err) break;			// Abbruch, wenn Fehler!			if (ADOC.Idx[i]==true) {			// Daten senden?				UWORD	retry;				UWORD	oldPtr = mWakt;				if (d) {					GetIndString(s,sSharpNames,i+1);					SetIText((Handle)GetCH(d,4),s);				}				for(retry=0;retry<3;retry++) {	// 3 Versuche hat der Organizer�					mWakt = oldPtr;					err = IQSendHeader(o,i+1,(gDoc)?ADOC.OwFlag:gS.OwFlag);					if (err == userCanceledErr) break;					if (err) continue;			// Wiederholen, wenn Fehler					if ((i>=(USERFILE1-1))&&(i<=(USERFILE3-1))) {	// ein Userfile (7700er)?						REG WORD	freeindex = i-USERFILE1+USER1FREE+1;						CLFErr = noErr;						CLFProc = IQS[i].s;						SearchOList(o,freeindex,"c03=t%",freeindex);	// Freifeld senden						CLFProc = Return;			// Funktion wieder sichern!						err = CLFErr;				// Fehlercode merken					}					if (!err) {						CLFErr = noErr;						CLFProc = IQS[i].s;						SearchOList(o,i+1,CLstr,i+1);	// Daten senden						CLFProc = Return;			// Funktion wieder sichern!						err = CLFErr;				// Fehlercode merken					}					if (err == userCanceledErr) break;					if (((err >= 0x41)&&(err <= 0x4B))||((err >= 0xFE)&&(err <= 0xFF))) break;					if (err) {						IQSendChecksum(o);						continue;		// Wiederholen, wenn Fehler					}					err = IQSendChecksum(o);					if (err == userCanceledErr) break;					if (((err >= 0x41)&&(err <= 0x4B))||((err >= 0xFE)&&(err <= 0xFF))) break;					if (err) continue;	// Wiederholen, wenn Fehler// jetzt: 3 Versuche	Pause(10);			// 1/6s Pause					if (err == noErr) break;	// Fertig, wenn kein Fehler				}			}		}		mWindowUpdate(mWmax);		SDTimeout = saveTimeout;	// Timeout zur�cksetzen	}	if (d)		DisposeDialog(d);	SDClose();						// serielle Schnittstelle schlie�en	IQSharpOffline();	pfeil();	IQSharpErr(err);}/*** *	Daten vom Sharp einlesen ***/void	SharpReceive(void){REG OSErr		err;REG DialogPtr	d;						// Ptr auf diverse Dialogeshort			button;					// Exit-ButtonREG ListHandle	l;						// f�r die Auswahllistenchar			index[MaxIQFileType];	// �bersetzungstabelle: ListIndex => IQIndex	if (DoDialog(dSharpTurnOnReceive,3) == 3) return; // Abfrage VOR dem Start	err = SDOpen();						// serielle Schnittstelle (Port A) �ffnen	if (!CheckOSError(err)) return;		// Fehler beim �ffnen der Schnittstelle	biene();	err = IQGetOrganizer(GetOrgH(gDoc)); // Directory einlesen	pfeil();	SDClose();							// serielle Schnittstelle schlie�en	if (IQSharpErr(err)) return;	d = GetCenteredDialog(dSharpReceive,nil);	if (!d) return;						// Dialog nicht zu �ffnen	l=NewList(d,6,false,true,0);	(*l)->selFlags = lNoExtend|lUseSense|lExtendDrag;	// Flags f�r Mehrfachselektierung	{	short i,j; Str255 s; Cell c;		for(i=0;i<MaxIQFileType-1;i++) {			GetIndString(s,sSharpNames,i+1);			if (s[0] && IQFindFile(GetOrgH(gDoc),(IQFileType)i+1)) {				j = AddListMgr(PtoCstr(s),l);	// an die Liste anh�ngen				index[i] = j;					// Index merken				c.h = 0; c.v = j;				LSetSelect(ADOC.Idx[i],c,l);	// Zelle selecten, wenn n�tig			} else				index[i] = -1;					// nicht benutzt		}	}	LAutoScroll(l);	LDoDraw(true,l);	OutlineDialogItem(d,1);			// Default-Button zeichnen	HiliteControl(GetCH(d,9),IQCheckRamcard(GetOrgH(gDoc))?0:255);	while (d) {		SetCtlValue(GetCH(d,4),ADOC.OwFlag==true);	// Radio-Buttons		SetCtlValue(GetCH(d,5),ADOC.OwFlag==false);		SetCtlValue(GetCH(d,9),(ADOC.o.type & RAMCARD)==RAMCARD);	// Radio-Buttons		SetCtlValue(GetCH(d,8),(ADOC.o.type & RAMCARD)!=RAMCARD);		ModalDialog((ModalFilterProcPtr)OwnDialogFilter,&button);		if ((button==1)||(button==3)) break;		switch(button) {		case	4:	ADOC.OwFlag = true;		// Radiobuttons					gS.OwFlag	= true;					gSaveSettings = true;	// Einstellungen dirty					break;		case	5:	ADOC.OwFlag = false;	// Radiobuttons					gS.OwFlag	= false;					gSaveSettings = true;	// Einstellungen dirty					break;		case	9:	ADOC.o.type |= RAMCARD;		// Radiobuttons					break;		case	8:	ADOC.o.type &= ~RAMCARD;	// Radiobuttons					break;		}	}	if (button == 1) {					// �Empfangen� angeklickt		short	i; Cell c;		for(i=0;i<MaxIQFileType-1;i++) {			c.h = 0; c.v = index[i];	// ausgew�hlte Zellen merken			if ((c.v>=0)&&(IQFindFile(GetOrgH(gDoc),(IQFileType)i+1)))				ADOC.Idx[i] = LGetSelect(false,&c,l);			else				ADOC.Idx[i] = false;	// Zelle nicht aktiv		}		BlockMove(ADOC.Idx,gS.Idx,sizeof(gS.Idx));	// selektierte Eintr�ge		gSaveSettings = true;			// Einstellungen dirty	}	DisposeObjects(d);	DisposDialog(d);	if (button == 1) {					// �Empfangen� angeklickt		OrganizerH	o = GetOrgH(gDoc);		short		i;		d = GetCenteredDialog(dSharpMessage,nil);		if (d) {			HideDItem(d,1);			ShowDItem(d,2);			HideDItem(d,3);			SetUserItem(d,6,mWindowDraw);			DrawDialog(d);		}		biene();		err = SDOpen();					// serielle Schnittstelle (Port A) �ffnen		if (!err) {			Str255		s;			mWmax = 1; mWakt = 0;			for(i=0;i<MaxIQFileType-1;i++) {				if (ADOC.Idx[i]==true) {					if (ADOC.OwFlag)	// true, wenn Overwrite						SharpIndexClr(o,i+1);					mWmax++;			// Gruppen z�hlen					if ((i>=(USERFILE1-1))&&(i<=(USERFILE3-1))) {	// ein Userfile (7700er)?//						mWmax++;		// Anwender-Freifelder-Gruppen auch z�hlen							SharpIndexClr(o,i+1-USERFILE1+USER1FREE);					}				}			}			{			ULONG	redrawTypeMask = 0;			for(i=0;i<MaxIQFileType-1;i++) {				if (ADOC.Idx[i]) {		// Daten einlesen?					if (d) {						GetIndString(s,sSharpNames,i+1);						SetIText((Handle)GetCH(d,4),s);					}					mWindowUpdate(mWakt + 1);					err = IQReadFile(o,i+1);	// Datei einlesen					redrawTypeMask |= 1L<<i;				}				if (err) break;			// Abbruch, wenn Fehler			}			DoBitFieldRedraw(redrawTypeMask);			}			mWindowUpdate(mWmax);		}		if (d)			DisposDialog(d);		SetSharpTyp((*o)->type);		AppDocumentDirty(gDoc,true);	// Dokument ist �dirty�		SDClose();						// serielle Schnittstelle schlie�en		IQSharpOffline();		pfeil();		if (IQSharpErr(err)) return;	} else {		IQSharpOffline();	}}/*** *	Daten vom Sharp einlesen ***/void	SharpSend(void){REG OSErr		err;REG DialogPtr	d;						// Ptr auf diverse Dialogeshort			button;					// Exit-ButtonREG ListHandle	l;						// f�r die Auswahllistenchar			index[MaxIQFileType];	// �bersetzungstabelle: ListIndex => IQIndex	if (DoDialog(dSharpTurnOnSend,3) == 3) return; // Abfrage VOR dem Start	err = SDOpen();						// serielle Schnittstelle (Port A) �ffnen	if (!CheckOSError(err)) return;		// Fehler beim �ffnen der Schnittstelle	biene();	err = IQGetOrganizer(GetOrgH(gDoc));	// Directory einlesen	pfeil();	SDClose();							// serielle Schnittstelle schlie�en	if (IQSharpErr(err)) return;	d = GetCenteredDialog(dSharpSend,nil);	if (!d) return;						// Dialog nicht zu �ffnen	l=NewList(d,6,false,true,0);	(*l)->selFlags = lNoExtend|lUseSense|lExtendDrag;	// Flags f�r Mehrfachselektierung	{	short i,j; Str255 s; Cell c;		for(i=0;i<MaxIQFileType-1;i++) {			GetIndString(s,sSharpNames,i+1);			if (s[0] && IQFindFile(GetOrgH(gDoc),(IQFileType)i+1)) {				j = AddListMgr(PtoCstr(s),l);	// an die Liste anh�ngen				index[i] = j;					// Index merken				c.h = 0; c.v = j;				LSetSelect(ADOC.Idx[i],c,l);	// Zelle selecten, wenn n�tig			} else				index[i] = -1;					// nicht benutzt		}	}	LAutoScroll(l);	LDoDraw(true,l);	NewPopup(d,8,157,13,gS.SendFlag);			// zu sendende Gruppe	HiliteControl(GetCH(d,9),(NewLinkFormat(gDoc))?0:255);	OutlineDialogItem(d,1);			// Default-Button zeichnen	HiliteControl(GetCH(d,11),IQCheckRamcard(GetOrgH(gDoc))?0:255);	while (d) {		SetCtlValue(GetCH(d,4),ADOC.OwFlag==true);	// Radio-Buttons		SetCtlValue(GetCH(d,5),ADOC.OwFlag==false);		SetCtlValue(GetCH(d,9),ADOC.TimeSend==true);// Uhrzeit mitsenden?		SetCtlValue(GetCH(d,11),(ADOC.o.type & RAMCARD)==RAMCARD);	// Radio-Buttons		SetCtlValue(GetCH(d,10),(ADOC.o.type & RAMCARD)!=RAMCARD);		ModalDialog((ModalFilterProcPtr)OwnDialogFilter,&button);		if ((button==1)||(button==3)) break;		switch(button) {		case	4:	ADOC.OwFlag = true;		// Radiobuttons					gS.OwFlag	= true;					gSaveSettings = true;	// Einstellungen dirty					break;		case	5:	ADOC.OwFlag = false;	// Radiobuttons					gS.OwFlag	= false;					gSaveSettings = true;	// Einstellungen dirty					break;		case	9:	ADOC.TimeSend 	^= 1;	// Zeit-Senden toggeln					gS.TimeSend		^= 1;					gSaveSettings	= true;					break;		case	11:	ADOC.o.type |= RAMCARD;		// Radiobuttons					break;		case	10:	ADOC.o.type &= ~RAMCARD;	// Radiobuttons					break;		}	}	if (button == 1) {					// �Senden� angeklickt		short	i; Cell c;		for(i=0;i<MaxIQFileType-1;i++) {			c.h = 0; c.v = index[i];	// ausgew�hlte Zellen merken			if ((c.v>=0)&&(IQFindFile(GetOrgH(gDoc),(IQFileType)i+1)))				ADOC.Idx[i] = LGetSelect(false,&c,l);			else				ADOC.Idx[i] = false;	// Zelle nicht aktiv		}		BlockMove(ADOC.Idx,gS.Idx,sizeof(gS.Idx));	// selektierte Eintr�ge		gS.SendFlag = GetPopupValue(d,8);			// zu sendende Gruppe		gSaveSettings = true;			// Einstellungen dirty	}	DisposeObjects(d);	DisposDialog(d);	if (button == 1) {					// �Senden� angeklickt		SendData();	} else {		IQSharpOffline();	}}/*** *	Leerfunktion f�r nicht implementierte Backup-Funktion ***/static short	gBackupFRefNum;OSErr	IQDoBackup(OrganizerH o,IQFileType dummy);OSErr	IQDoBackup(OrganizerH o,IQFileType dummy){REG OSErr	err = noErr;REG UBYTE	buf[SERIALBUF];REG LONG	len;LONG		count;	do {		err = SDRead(buf,1);			// erstes Byte der Zeile lesen		if (err<0) break;		if (*buf == 0x1A) break;		// CTRL-Z = Ende erreicht		err = SDRead(buf+1,SERIALBUF-1);// Rest der Zeile einlesen		if (err<0) break;		len = err + 1;					// Gesamtl�nge der Zeile		count = len;		err = FSWrite(gBackupFRefNum,&count,buf); // Block schreiben		if ((count != len)&&(!err)) err = ioErr;		if (err) break;	} while(0==0);	if(err>0) err = noErr;				// positiver Fehler = Anzahl d. gelesenen Bytes	if (err>=0) {		count = 1;		err = FSWrite(gBackupFRefNum,&count,buf); // CTRL-Z am Ende eines jeden Blockes schreiben		if ((count != 1)&&(!err)) err = ioErr;	}	return(err);}/*** *	Daten vom Sharp sichern ***/void	SharpBackup(void){REG OSErr			err;REG OrganizerH		o;REG DialogPtr		d = nil;			// Default: kein Dialog offenFInfo				f;StandardFileReply	reply;long				fpos;Str63				untitled;OSType				saveType;	GetIndString(untitled,STRcommon,strBackupFname);	// einen �Dummy�-Namen setzen	CopyPString(reply.sfFile.name,untitled);	if (!DisplayPutFile(&reply,false)) return;			// Daten wegschreiben? Nein!	biene();	saveType = gG.FileTypes[0]; gG.FileTypes[0] = 'YBAX';// Backup-Filetype	err = CreateOpenFile(&(reply.sfFile),&gBackupFRefNum);	gG.FileTypes[0] = saveType;	if (!CheckOSError(err)) return;	FCopyResource(&reply.sfFile,'STR ',-16396,'STR ',-16396);	// Resource �bertragen	SetFPos(gBackupFRefNum,fsFromStart,0);				// an den Dateistart	if (DoDialog(dSharpTurnOnReceive,3) == 3) {			// Abfrage VOR dem Start		FSClose(gBackupFRefNum);		FDelete(&reply.sfFile);							// Backup-Datei wieder l�schen		FFlushVol(&reply.sfFile);						// Volume updaten		return;	}	biene();	o = (OrganizerH)NewHandleClear(sizeof(Organizer));	if (!CheckOSError(MemError())) {	// Speicher reichte nicht		FSClose(gBackupFRefNum);		FDelete(&reply.sfFile);			// Backup-Datei wieder l�schen		FFlushVol(&reply.sfFile);		// Volume updaten		return;	}	err = SDOpen();						// serielle Schnittstelle (Port A) �ffnen	if (!CheckOSError(err)) {		DisposHandle((Handle)o);		FSClose(gBackupFRefNum);		FDelete(&reply.sfFile);			// Backup-Datei wieder l�schen		FFlushVol(&reply.sfFile);		// Volume updaten		return;	}	(*o)->type = -1;					// interner Organizer => Dirty-Flag NICHT �ndern	err = IQGetOrganizer(o);			// Directory einlesen	if ((!err)&&((*o)->dir)) {			// kein Fehler UND ein Directory vorhanden?		LONG		size = GetHandleSize((Handle)(*o)->dir);		WORD		offset;		LONG		count;		WORD		type = (*o)->type;		HLock((Handle)(*o)->dir);		count = sizeof(WORD);		err = FSWrite(gBackupFRefNum,&count,&type);	// Organizertyp schreiben		if ((count != sizeof(WORD))&&(!err)) err = ioErr;		if (!err) {			count = sizeof(LONG);			err = FSWrite(gBackupFRefNum,&count,&size);	// L�nge vom Directory schreiben			if ((count != sizeof(LONG))&&(!err)) err = ioErr;		}		if (!err) {			count = size;			err = FSWrite(gBackupFRefNum,&count,*(*o)->dir);	// Directory schreiben			if ((count != size)&&(!err)) err = ioErr;		}		HUnlock((Handle)(*o)->dir);		if (!err) {			d = GetCenteredDialog(dSharpMessage,nil);			if (d) {				HideDItem(d,1);				ShowDItem(d,2);				// Backup-Message darstellen				HideDItem(d,3);				SetUserItem(d,6,mWindowDraw);// Meter-Balken anmelden				DrawDialog(d);			}			mWmax = size/21L; mWakt = 0;	// Meter-Balken-Variablen setzen			for(offset=0;offset<mWmax;offset++) {	// alle Directory-Eintr�ge durchgehen				mWindowUpdate(mWakt + 1);	// Meter-Balken ein St�ck weiter				if (d) {					UCHAR	stemp[12];					BlockMove(*(*o)->dir+offset*21+8,stemp+1,11);					stemp[0] = 11;					SetIText((Handle)GetCH(d,4),stemp);		// Gruppen-Name				}				err = IQReadEntry(o,offset,IQDoBackup,UNKNOWNFILE);	// Gruppe einlesen				if (err) break;			// ein Fehler! => raus			}			mWindowUpdate(mWmax);		// Meter-Balken auf das Maximum		}	}	SDClose();							// serielle Schnittstelle schlie�en	IQSharpOffline();					// Sharp offline holen	pfeil();	if (d) DisposeDialog(d);			// Dialog freigeben, wenn vorhanden	IQSharpErr(err);					// irgendein Fehler bzgl. des Sharps? => melden	if ((*o)->dir)						// kein Directory?		DisposeHandle((*o)->dir);		// sonst das Directory freigeben	DisposHandle((Handle)o);					// Organizer-Handle wieder freigeben	if (!err)		err = GetFPos(gBackupFRefNum,&fpos);// Dateiende setzen	if (!err)		err = SetEOF(gBackupFRefNum,fpos);	FSClose(gBackupFRefNum);	if (err)		FDelete(&reply.sfFile);	FFlushVol(&reply.sfFile);			// Volume updaten}/*** *	Komplettes Restore in den Organizers machen ***/VOID		DoRestore(FSSpec *sf){REG OSErr			err;REG OrganizerH		o;REG DialogPtr		d = nil;			// Default: kein Dialog offenLONG				fpos;	biene();	err = FOpenDF(sf,fsRdPerm,&gBackupFRefNum);	if (!CheckOSError(err)) return;	SetFPos(gBackupFRefNum,fsFromStart,0);	// an den Dateistart	if (DoDialog(dSharpTurnOnSend,3) == 3) {// Abfrage VOR dem Start		FSClose(gBackupFRefNum);		return;	}	biene();	o = (OrganizerH)NewHandleClear(sizeof(Organizer));	if (!CheckOSError(MemError())) {		// Speicher reichte nicht		FSClose(gBackupFRefNum);		return;	}	err = SDOpen();							// serielle Schnittstelle (Port A) �ffnen	if (!CheckOSError(err)) {		DisposHandle((Handle)o);		FSClose(gBackupFRefNum);		return;	}	(*o)->type = -1;						// interner Organizer => Dirty-Flag NICHT �ndern	err = IQGetOrganizer(o);				// Directory einlesen	if ((!err)&&((*o)->dir)) {				// kein Fehler UND ein Directory vorhanden?		LONG		count,size;		WORD		type,offset;		Handle		dir;		WORD		savetimeout = SDTimeout;		SDTimeout = 5*60;					// 5s Timeout!		count = sizeof(WORD);		err = FSRead(gBackupFRefNum,&count,&type);	// Organizertyp einlesen		if ((count != sizeof(WORD))&&(!err)) err = ioErr;		if ((type & ~SPREATSHEET) != ((*o)->type & ~SPREATSHEET) ) {	// Organizertyp stimmt nicht!			err = 1234;						// Fehler!		}		if (!err) {			count = sizeof(LONG);			err = FSRead(gBackupFRefNum,&count,&size);	// L�nge vom Directory einlesen			if ((count != sizeof(LONG))&&(!err)) err = ioErr;			if (!err) {				dir = NewHandle(size);				if (!(err = MemError())) {					HLock((Handle)dir);					count = size;					err = FSRead(gBackupFRefNum,&count,*dir);	// Directory einlesen					if ((count != size)&&(!err)) err = ioErr;					HUnlock((Handle)dir);				}			}		}		if (!err) {			REG UWORD	header = IQGetHeader(o);			REG OSErr	applerr = noErr;			d = GetCenteredDialog(dSharpMessage,nil);			if (d) {				ShowDItem(d,1);				HideDItem(d,2);				// Restore-Message darstellen				HideDItem(d,3);				SetUserItem(d,6,mWindowDraw);// Meter-Balken anmelden				DrawDialog(d);			}			mWmax = size/21L; mWakt = 0;	// Meter-Balken-Variablen setzen			HLock((Handle)dir);			for(offset=0;offset<mWmax;offset++) {	// alle Directory-Eintr�ge durchgehen				REG USTR	sP = (USTR)*dir+offset*21;				UCHAR		c;				mWindowUpdate(mWakt + 1);	// Meter-Balken ein St�ck weiter				if (d) {					UCHAR	stemp[12];					BlockMove(sP+8,stemp+1,11);					stemp[0] = 11;					SetIText((Handle)GetCH(d,4),stemp);		// Gruppen-Name				}				err = SDWriteCmd('S'|header);		// CHR$(3 oder 4)+"S" senden				if (err) break;				SDChecksum = 0;						// Pr�fsumme l�schen				sP[4] = '1';						// damits klappt�				sP[5] |= 1;							// Overwrite!				err = SDWrite(sP,21);				// Infoblock senden				if (err) break;				do {					count = 1;					err = FSRead(gBackupFRefNum,&count,&c);	// ein Zeichen einlesen					if (err) break;					if (c == 0x1A) break;			// CTRL-Z = Ende					err = SDWrite(&c,1);			// das Byte senden					if (err) break;				} while(0==0);				if (err) break;				err = IQSendChecksum(o);			// noch eine Pr�fsumme, dann fertig!				if ((err == 72)||(err == 73)) {		// Applikation nicht vorhanden?					applerr = err;					err = noErr;				}				if (err) break;			}			mWindowUpdate(mWmax);		// Meter-Balken auf das Maximum			HUnlock((Handle)dir);			if ((applerr)&&(!err))		// irgendeine Applikation nicht gefunden?				err = applerr;			// Fehler!			if (!err)					// kein Fehler?				IQSetTime(o);			// dann nochmal versuchen die Uhrzeit zu senden		}		SDTimeout = savetimeout;	}	SDClose();							// serielle Schnittstelle schlie�en	IQSharpOffline();					// Sharp offline holen	pfeil();	if (d) DisposeDialog(d);			// Dialog freigeben, wenn vorhanden	if (err == 1234) {					// falscher Organizer?		DoDialog(173,1);				// Jo!	} else {		IQSharpErr(err);				// irgendein Fehler bzgl. des Sharps? => melden	}	if ((*o)->dir)						// kein Directory?		DisposeHandle((*o)->dir);		// sonst das Directory freigeben	DisposHandle((Handle)o);					// Organizer-Handle wieder freigeben	FSClose(gBackupFRefNum);}/*** *	Komplettes Restore in den Organizers machen * *	(Men�aufruf mit File-Selector) ***/void	SharpRestore(void){OSType				saveType;Boolean				flag;StandardFileReply	reply;	saveType = gG.FileTypes[0]; gG.FileTypes[0] = 'YBAX';// Backup-Filetype	flag = !DisplayGetFile(&reply);	gG.FileTypes[0] = saveType;	if (flag) return;					// kein Restore	DoRestore(&reply.sfFile);}/*** *	Sharp-Organizer l�schen ***/void	SharpLoeschen(void){REG OSErr		err;REG DialogPtr	d;						// Ptr auf diverse Dialogeshort			button;					// Exit-ButtonREG ListHandle	l;						// f�r die Auswahllistenchar			index[MaxIQFileType];	// �bersetzungstabelle: ListIndex => IQIndex	if (DoDialog(dSharpTurnOnSend,3) == 3) return; // Abfrage VOR dem Start	err = SDOpen();						// serielle Schnittstelle (Port A) �ffnen	if (!CheckOSError(err)) return;		// Fehler beim �ffnen der Schnittstelle	biene();	err = IQGetOrganizer(GetOrgH(gDoc));	// Directory einlesen	pfeil();	SDClose();							// serielle Schnittstelle schlie�en	if (IQSharpErr(err)) return;	d = GetCenteredDialog(dSharpClear,nil);	if (!d) return;						// Dialog nicht zu �ffnen	l=NewList(d,4,false,true,0);	(*l)->selFlags = lNoExtend|lUseSense|lExtendDrag;	// Flags f�r Mehrfachselektierung	{	short i,j; Str255 s; Cell c;		for(i=0;i<MaxIQFileType-1;i++) {			GetIndString(s,sSharpNames,i+1);			if (s[0] && IQFindFile(GetOrgH(gDoc),(IQFileType)i+1)) {				j = AddListMgr(PtoCstr(s),l);	// an die Liste anh�ngen				index[i] = j;					// Index merken				c.h = 0; c.v = j;				LSetSelect(false,c,l);			// Zelle selecten, wenn n�tig			} else				index[i] = -1;			// nicht benutzt		}	}	LAutoScroll(l);	LDoDraw(true,l);	OutlineDialogItem(d,1);				// Default-Button zeichnen	HiliteControl(GetCH(d,6),IQCheckRamcard(GetOrgH(gDoc))?0:255);	while (d) {		SetCtlValue(GetCH(d,6),(ADOC.o.type & RAMCARD)==RAMCARD);	// Radio-Buttons		SetCtlValue(GetCH(d,7),(ADOC.o.type & RAMCARD)!=RAMCARD);		ModalDialog((ModalFilterProcPtr)OwnDialogFilter,&button);		if ((button==1)||(button==3)) break;		switch(button) {		case	6:	ADOC.o.type |= RAMCARD;		// Radiobuttons					break;		case	7:	ADOC.o.type &= ~RAMCARD;	// Radiobuttons					break;		}	}	if (button == 3) {					// �L�schen� angeklickt		short	i; Cell c;		for(i=0;i<MaxIQFileType-1;i++) {			c.h = 0; c.v = index[i];	// ausgew�hlte Zellen merken			if ((c.v>=0)&&(IQFindFile(GetOrgH(gDoc),(IQFileType)i+1)))				ADOC.Idx[i] = LGetSelect(false,&c,l);			else				ADOC.Idx[i] = false;	// Zelle nicht aktiv		}		BlockMove(ADOC.Idx,gS.Idx,sizeof(gS.Idx));	// selektierte Eintr�ge		gSaveSettings = true;			// Einstellungen dirty	}	DisposeObjects(d);	DisposDialog(d);	if ((button == 3)&&(DoDialog(dSharpClearShure,1) != 1)) {	// �L�schen� angeklickt		short		i;		d = GetCenteredDialog(dSharpMessage,nil);		if (d) {			HideDItem(d,1);			HideDItem(d,2);			ShowDItem(d,3);			SetUserItem(d,6,mWindowDraw);			DrawDialog(d);		}		biene();		err = SDOpen();					// serielle Schnittstelle (Port A) �ffnen		if (!err) {			OrganizerH	o = GetOrgH(gDoc);			Str255		s;			mWmax = 1; mWakt = 0;			for(i=0;i<MaxIQFileType-1;i++) {				if (ADOC.Idx[i]==true) mWmax++;		// Gruppen z�hlen			}			for(i=0;i<MaxIQFileType-1;i++) {				if (ADOC.Idx[i]) {					// Daten l�schen?					UWORD	retry;					if (d) {						GetIndString(s,sSharpNames,i+1);						SetIText((Handle)GetCH(d,4),s);					}					mWindowUpdate(mWakt + 1);					for(retry=0;retry<3;retry++) {	// 3 Versuche hat der Organizer�						err = IQSendHeader(o,i+1,true);		// true = Overwrite						if (err == userCanceledErr) break;						if (err) continue;			// Wiederholen, wenn Fehler						err = IQSendChecksum(o);						if (err == userCanceledErr) break;						if (err) continue;			// Wiederholen, wenn Fehler// jetzt: 3 Versuche	Pause(10);					// 1/6s Pause						if (err == noErr) break;	// Fertig, wenn kein Fehler					}					if (err) break;					// Abbruch, wenn Fehler				}			}			mWindowUpdate(mWmax);		}		if (d)			DisposeDialog(d);		SDClose();						// serielle Schnittstelle schlie�en		IQSharpOffline();		pfeil();		IQSharpErr(err);	} else {		IQSharpOffline();	}}/*** *	Schnittstelle umgeschaltet ***/void	SharpSchnittstelle(short entry){	GetItem(GetMHandle(mSharpSerMenu),entry,(StringPtr)&SDName);	// Namen der Schnittstelle	Strpcpy((StringPtr)&gS.serPort,(StringPtr)&SDName);	gSaveSettings = true;							// Einstellungen dirty}/*** *	Sharp-Men� angew�hlt ***/void	SharpMenu(short entry){REG OSErr		err;VOID			BetaDialog(void);	switch(entry) {	case	mSharpSend:					SharpSend();						// Daten empfangen					break;	case	mSharpReceive:					SharpReceive();						// Daten empfangen					break;	case	mSharpClear:					SharpLoeschen();					// Sharp l�schen					break;	case	mSharpBackup:					SharpBackup();						// Backup machen					break;	case	mSharpRestore:					SharpRestore();						// Restore zur�ckspielen					break;	case	mSharpNach:					if ((gTheEvent.modifiers & (optionKey|cmdKey))==(optionKey|cmdKey)) {						gAppTrue = !gAppTrue;			// �Cheatmode� aktivieren						BetaDialog();						UnloadSeg(BetaDialog);			// Segment rauswerfen						break;					}					gS.OffFlag = !gS.OffFlag;			// Off-Flag toggeln					gSaveSettings = true;				// Einstellungen dirty					break;	}	SharpMUpdate();					// Men� updaten}/*** *	Sharp-Untermen� angew�hlt ***/void	SharpMenuTyp(WORD entry){REG WORD	typ = ADOC.o.type;		// illegaler TypREG WORD	oldtyp = typ;	switch(entry) {	case	1:	typ = ZQ5000;				break;	case	2:	typ &= ALLCARDS;				typ |= IQ7000;				break;	case	3:	typ &= ALLCARDS;				typ |= IQ7600;				break;	case	4:	typ &= ALLCARDS;				typ |= IQ8000;				break;	case	5:	typ &= ALLCARDS;				typ |= IQ8400;				break;	case	7:	if (typ == ZQ5000) break;				if (typ & RAMCARD) {					typ = typ & ~RAMCARD;				} else {					typ &= IQALL;					typ |= RAMCARD;				}				break;	case	8:	if (typ == ZQ5000) break;				if (typ & TIMEEXPCARD) {					typ = typ & ~TIMEEXPCARD;				} else {					typ &= IQALL;					typ |= TIMEEXPCARD;				}				break;	case	9:	if (typ == ZQ5000) break;				if (typ & SPREATSHEET) {					typ = typ & ~SPREATSHEET;				} else {					typ &= IQALL;					typ |= SPREATSHEET;				}				break;	default:	typ = 0;	}	if (((oldtyp & IQALL) != (typ & IQALL))||		(((oldtyp & TIMEEXPCARD) == TIMEEXPCARD)&&((typ & TIMEEXPCARD) != TIMEEXPCARD)))		if (DoDialog(dSharpLostData,1) != 3) return;	// Abbruch!	SetSharpTyp(typ);}