#include "IQComm.h"#include "SerialComm.h"#include "Geos.h"			// wegen des Zugriffs auf die Settings#include "Sharp.h"			// wegen SetSharpTyp()#include "DialogLib.h"		// wegen DoDialog()#include "GeosMore.h"		// wegen CheckOSError()#include "Utilities.h"		// wegen HLockH()#include "GlobalLib.h"		// wegen GetAppResource()#include <string.h>			// wegen strncmp()#include <packages.h>		// International Utilities#include <stdio.h>			// wegen sprintf()#include "IQSharpSenden.h"#include "IQSharpEmpfang.h"#include "UserFile.h"		// f�r CreateUserFileDefaults in IQSetDefault/*** *	Struktur f�r Organizer-Kommunikation ***/struct IQInfoStruct IQS[] =	{			{ SCHEDULE,	'0110',"SCHEDULE1  ",IQDlSchedule,	IQUlSchedule,	IQALL },			{ ANN1,		'0110',"ANN     1  ",IQDlAnn1,		IQUlAnn1,		IQALL },			{ ANN2,		'0110',"ANN     2  ",IQDlAnn2,		IQUlAnn2,		IQALL },			{ PERIOD,	'0110',"PERIOD  1  ",IQDlPSchedule,	IQUlPSchedule,	IQ8x00 },			{ DALARM,	'0110',"D ALARM 1  ",IQDlDAlarm,	IQUlDAlarm,		IQ8x00 },			{ TEL1DATA,	'0200',"TEL     1  ",IQDlTelData,	IQUlTelData,	IQALL },			{ TEL2DATA,	'0200',"TEL     2  ",IQDlTelData,	IQUlTelData,	IQALL },			{ TEL3DATA,	'0200',"TEL     3  ",IQDlTelData,	IQUlTelData,	IQALL },			{ TEL1FILE,	'0200',"TEL FILE1  ",IQDlTelFile,	IQUlTelFile,	IQ8x00 },			{ TEL2FILE,	'0200',"TEL FILE2  ",IQDlTelFile,	IQUlTelFile,	IQ8x00 },			{ TEL3FILE,	'0200',"TEL FILE3  ",IQDlTelFile,	IQUlTelFile,	IQ8x00 },			{ TEL1FREE,	'0200',"TEL FREE1  ",IQDlTelFree,	IQUlTelFree,	IQ8x00 },			{ TEL2FREE,	'0200',"TEL FREE2  ",IQDlTelFree,	IQUlTelFree,	IQ8x00 },			{ TEL3FREE,	'0200',"TEL FREE3  ",IQDlTelFree,	IQUlTelFree,	IQ8x00 },			{ MEMO,		'0300',"MEMO    1  ",IQDlMemo,		IQUlMemo,		IQALL },			{ OUTLINE,	'1D00',"OUTLINE 1  ",IQDlOutline,	IQUlOutline,	IQ8x00 },			{ BUSINESS,	'1E00',"BUSINESS1  ",IQDlBusiness,	IQUlBusiness,	IQ8x00 },			{ BUSFREE,	'1E00',"BUS FREE1  ",IQDlBusFree,	IQUlBusFree,	IQ8x00 },			{ USERDIC,	'1F00',"USER'S  DIC",IQDlUserDic,	IQUlUserDic,	IQ8x00 },			{ DOLIST,	'0B10',"DO LIST    ",IQDlDoList,	IQUlDoList,		ZQ5000|TIMEEXPCARD },			{ EXPENSE,	'0B10',"EXPENSE    ",IQDlExpense,	IQUlExpense,	ZQ5000|TIMEEXPCARD },			{ TIME,		'0B10',"TIME       ",IQDlTime,		IQUlTime,		TIMEEXPCARD },			{ TODO,		'3110',"TODO    1  ",IQDlTodo,		IQUlTodo,		IQ8400 },			{ USERFILE1,'2F00',"USER    1  ",IQDlUserfile,	IQUlUserfile,	IQ7600 },			{ USERFILE2,'2F00',"USER    2  ",IQDlUserfile,	IQUlUserfile,	IQ7600 },			{ USERFILE3,'2F00',"USER    3  ",IQDlUserfile,	IQUlUserfile,	IQ7600 },			{ USER1FREE,'2F00',"USER    1  ",IQDlUserfile,	IQUlUserfile,	IQ7600 },			{ USER2FREE,'2F00',"USER    2  ",IQDlUserfile,	IQUlUserfile,	IQ7600 },			{ USER3FREE,'2F00',"USER    3  ",IQDlUserfile,	IQUlUserfile,	IQ7600 }			};/*** *	Testen, ob eine bestimmte Datei vom Organizer-Typ unterst�tzt wird ***/Boolean		IQIsAvailable(OrganizerH o,REG IQFileType typ){REG WORD	i;REG Boolean	ret = false;	if (!o) return(ret);				// kein Organizer => keine Daten	i = (*o)->type & ~RAMCARD;	switch(typ) {						// Userfile-Freifelder auf Userfile ummappen	case USER1FREE:	case USER2FREE:	case USER3FREE:	typ = USERFILE1;					break;	}	if (i & IQGetOrgMask(typ))		ret = true;	return(ret);}/*** *	m�gliche Organizer zu einer bestimmten Datei ermitteln ***/WORD		IQGetOrgMask(REG IQFileType typ){REG WORD	i;	for(i=0;i<MaxIQFileType;i++)		if (IQS[i].id == typ) return(IQS[i].orgmask);	// erlaubte Organizer	return(0);							// kein Organizer}/*** *	String vom Sharp IQ ins Macintosh-Format wandeln ***/VOID	IQToMacintosh(REG USTR s){OSErr		err;REG USTR	*h = (USTR*)GetAppResource('O->M',128,&err);	if ((!h)||(err != noErr)) return;	while(*s)		*s++ = *(*h + *s);		// Zeichen konvertieren}/*** *	String vom Macintosh-Format ins Sharp IQ wandeln ***/VOID	IQToSharpIQ(REG USTR s){REG UWORD	i;OSErr		err;REG USTR	*h = (USTR*)GetAppResource('O->M',128,&err);	if ((!h)||(err != noErr)) return;	while(*s) {		for(i=0;i<256;i++)			if ((*h)[i] == *s) break;		if (i<256)			*s = i;				// Zeichen konvertieren		s++;	}}/*** *	Hexbyte holen ***/UBYTE	IQGetHexB(REG USTR p){REG UWORD	a,b;	a = *p++ - '0';	if (a > 10) a -= 7;	b = *p++ - '0';	if (b > 10) b -= 7;	return(a<<4|b);}/*** *	Hexbyte schreiben ***/VOID	IQSetHexB(REG USTR p,UBYTE byte){REG UWORD	a = byte & 0x0F;	byte >>= 4;	*p = byte+'0';	if (byte>=10) *p += 7;	p++;	*p = a+'0';	if (a>=10) *p += 7;	p++;}/*** *	Dezimalzahl holen ***/UWORD	IQGetDez(REG USTR p,UWORD i){REG UWORD	a = 0;REG UWORD	j;	if (*p == ' ') {			// Leerzeichen?		p += i;					// Pointer hinter die �Zahl�		return(-1);				// und -1 zur�ckgeben	}	for(j=0;j<i;j++) {		a *= 10;		a += (*p++ - '0');	}	return(a);}/*** *	Dezimalzahl mit n Stellen ausgeben holen ***/USTR	IQSetDez(REG USTR p,UWORD i,UWORD zahl){Str255		s;REG UWORD	j;	if(zahl == -1) {			// Magic-Number?		for(j=0;j<i;j++)			*p++ = ' ';			// dann Leerzeichen nehmen		return(p);	}	NumToString(zahl+100000,s);	// maximal 5 Stellen	j = 7-i;	while(j < 7)		*p++ = s[j++];	return(p);}/*** *	String bis zum CR bzw. Nullbyte �bertragen ***/USTR	IQGetStringCr(REG USTR d,REG USTR s){USTR	dest = d;	while((*s != 13)&&(*s != 0))// String bis zum CR oder Nullbyte kopieren		*d++ = *s++;	*d = 0;						// String mit Nullbyte abschlie�en	IQToMacintosh(dest);		// Zeichen im String wandeln	return(d);					// Ptr auf das Stringende zur�ckgeben}/*** *	String bis zum ersten Nullbyte �bertragen ***/USTR	IQGetStringNull(REG USTR d,REG USTR s){USTR	dest = d;	while(*s != 0)				// String bis zum Nullbyte kopieren		*d++ = *s++;	*d = 0;						// String mit Nullbyte abschlie�en	IQToMacintosh(dest);		// Zeichen im String wandeln	return(d);}/*** *	Fehlercode vom Organizer holen * *	Parameter: Organizerhandle ***/OSErr		IQGetError(REG OrganizerH o){REG OSErr	err;UBYTE		buf[2];	if (!o) return(-1);					// kein Organizer angegeben	err = SDRead(buf,1);				// Fehlercode empfangen	if (err<0) return(err);	switch(buf[0]) {	case	0x06:	case	0x02:	break;				// kein Fehler!	case	0x0A:	if (IQGetHeader(o) == (LINKFORM2<<8)) {	// neueres Format mit genaueren Fehlercodes?						err = SDRead(buf,2);		// Fehlercode abholen						if (err<0) return(err);						err = IQGetHexB(buf);		// Fehlercode wandeln						if (err == 0xFF) return(userCanceledErr);// Weil Ramkarten teilweise (?!?) kein �Daily Alarm� und �UserDic� haben://						if (err = 79) err = noErr;	// Datei nicht gefunden = kein Fehler						return(err);				// und zur�ckgeben					}	default:		return(ioErr);		// unbekannte R�ckmeldung vom Organizer	}	return(noErr);}/*** *	Checksum senden ***/OSErr		IQSendChecksum(REG OrganizerH o){REG OSErr	err;UCHAR		s[8];REG UCHAR	c;	if (!o) return(-1);					// kein Organizer angegeben	err = SDWrite((USTR)"\032",1);		// Daten mit CTRL-Z abschlie�en	if (err) return(err);	sprintf((STR)s,"%4.4X",SDChecksum);	// Pr�fsumme senden	c = s[0]; s[0] = s[2]; s[2] = c;	// oberes Byte drehen	c = s[1]; s[1] = s[3]; s[3] = c;	// unteres Byte drehen	s[4] = 13; s[5] = 10; s[6] = 26;	// CR/LF + CTRL-Z senden	s[7] = 0;							// String terminieren (nicht n�tig, aber sch�ner)	err = SDWrite(s,7);					// Hex-Pr�fsumme senden	if (err) return(err);	err = IQGetError(o);				// Fehlercode holen	return(err);}/*** *	Header f�r das Senden zum Sharp ***/OSErr		IQSendHeader(REG OrganizerH o,REG IQFileType index,Boolean flag){REG UWORD	header = IQGetHeader(o);REG OSErr	err;UCHAR		s[22];REG UWORD	i;	if (!o) return(-1);					// kein Organizer angegeben	err = SDWriteCmd('S'|header);		// CHR$(3 oder 4)+"S" senden	if (err) return(err);	s[4] = '1';							// ZUM Sharp senden	s[5] = (((*o)->type & RAMCARD)!=RAMCARD)?'0':'8';	// Ins Expansiom-RAM?	s[5] |= (flag & 0x01);				// Append oder Overwrite	s[6] = 13; s[7] = 10;				// CR/LF	s[19] = 13; s[20] = 10;				// CR/LF	s[21] = 0;							// String terminieren (nicht n�tig, aber sch�ner)	for(i=0;i<sizeof(IQS)/sizeof(IQInfoStruct);i++) {	// alle Eintr�ge durchgehen		if (IQS[i].id == index) {				// Eintrag gefunden			*(long*)s = IQS[i].applno;			// Application No.			BlockMove(IQS[i].name,&s[8],11);	// Data Name			break;		}	}	SDChecksum = 0;						// Pr�fsumme l�schen	err = SDWrite(s,21);				// Infoblock senden	return(err);}/*** *	Befehls-Header je nach Organizer zur�ckgeben * *	Achtung: gleich um 8 Bits ins obere Wort geschoben! Praktisch, da mit *			 dem Befehl so einfach zu verodern. ***/WORD	IQGetHeader(REG OrganizerH o){REG WORD	header = -1;	if (!o) return(-1);					// kein Organizer angegeben	switch((*o)->type & IQALL) {	case ZQ5000:	case IQ7000:	case IQ7600:	header = LINKFORM1;					break;	case IQ8000:	case IQ8400:	header = LINKFORM2;					break;	}	return(header<<8);}/*** *	IQReadDir ***/OSErr		IQReadDir(REG UBYTE typ,REG UCHAR dirid,REG Handle *dir){UBYTE		buf[SERIALBUF];REG OSErr	err;REG ULONG	o;	err = SDWriteCmd(typ<<8|dirid);	// CHR$(3 oder 4)+"D" senden	if (err) return(err);	if (typ == LINKFORM2) {		SDSetBrk('\r');				// bis CR lesen		err = SDRead(buf,200);		// Header 2 (Rechnertyp) �berlesen		if (err < 0) return(err);		err = SDRead(buf,1);		// LF vom Header 2 �berlesen		if (err < 0) return(err);	}	*dir = NewHandle((Size)nil);	// eine leere Handle einrichten	err = MemError();	if (err) return(err);			// Ein Fehler?!?	SDSetBrk(0x1A);					// bis CR lesen	while(1) {		err = SDRead(buf,1);		// Dateiende-Code lesen		if (err < 0) break;		if (buf[0] == 0x1A) break;	// CTRL-Z = Ende vom Directory		err = SDRead(&buf[1],20);	// Rest vom Eintrag lesen		if (err < 0) break;		o = GetHandleSize(*dir);	// alte Gr��e		SetHandleSize(*dir,o+21);	// neue Gr��e = alte Gr��e + einen Eintrag		err = MemError();		if (err < 0) break;		BlockMove(buf,**dir+o,21);	// und Block �bertragen	}	if (err >= 0) {		SDSetBrk(0x1A);				// bis zum CTRL-Z lesen		err = SDRead(buf,20);		// Pr�fsumme �berlesen	}	if (err < 0) {		DisposHandle(*dir);		*dir = nil;		return(err);	}	return(noErr);					// alles ok.}/*** *	Einen Eintrag im Directory vorhanden? ***/Boolean		IQFindFile(REG OrganizerH o,REG IQFileType index){REG OrganizerP	op = *o;REG UWORD		i;REG ULONG		size;REG ULONG		offs;REG CHAR		flag = (op->type & RAMCARD)?'8':'0';	// Ramkarte auslesen?REG Boolean		retflag = false;	if (!o) return(false);						// kein Organizer angegeben	if (!op->dir) return(false);				// kein Directory => nix gefunden	size = GetHandleSize(op->dir);				// Gr��e vom Directory	if (!size) return(false);					// Gr��e == 0 => nix gefunden	HLock(op->dir);	for(i=0;i<sizeof(IQS)/sizeof(IQInfoStruct);i++) {	// alle Eintr�ge durchgehen		if (IQS[i].id == index) {					// Eintrag gefunden			for(offs=0;offs<size;offs += 21) {		// im Directory suchen				if (!strncmp(*(op->dir)+offs+8,(STR)IQS[i].name,11)&&					(*(*(op->dir)+offs+5)==flag))	{	// gleicher Eintrag?					retflag = true;					// gefunden					break;				}			}		}	}	HUnlock(op->dir);	return(retflag);}/*** *	Test, ob der Organizer mit Ramkarte ausgestattet ist ***/Boolean		IQCheckRamcard(REG OrganizerH o){REG ULONG		size;		// Gr��e vom DirectoryREG ULONG		offs;REG UCHAR		**dir;REG USTR		dirP;	if (o) {										// kein Organizer angegeben		dir = (UCHAR**)(*o)->dir;		if (dir) {									// Directory eingelesen?			size = GetHandleSize((Handle)dir);		// Gr��e vom Directory			dirP = *dir;			for(offs=0;offs<size;offs += 21,dirP += 21) {	// im Directory suchen				if ((dirP[4]=='0')&&(dirP[5]=='8')) // Ramkarte erkannt?					return(true);					// Ja!			}		}	}	return(false);}/*** *	Test, ob der Organizer mit Spreatsheet-Karte ausgestattet ist ***/Boolean		IQCheckSpreatsheet(REG OrganizerH o){REG ULONG		size;		// Gr��e vom DirectoryREG ULONG		offs;REG UCHAR		**dir;REG USTR		dirP;	if (o) {										// kein Organizer angegeben		dir = (UCHAR**)(*o)->dir;		if (dir) {									// Directory eingelesen?			size = GetHandleSize((Handle)dir);				// Gr��e vom Directory			dirP = *dir;			for(offs=0;offs<size;offs += 21,dirP += 21) {	// im Directory suchen				if ((dirP[0]=='1')&&(dirP[1]=='C')					&&(dirP[2]=='0')&&(dirP[3]=='0')) // Ramkarte erkannt?					return(true);					// Ja!			}		}	}	return(false);}/*** *	Default-Organizer setzen ***/void	IQSetDefault(REG OrganizerH o){REG Intl0Hndl	int0 = (Intl0Hndl)IUGetIntl(0);	// international Resource ladenREG UWORD		lang;REG UCHAR		hstate;Str255			s;LONG			num;REG OrganizerP	op;	if (!o) return;						// kein Organizer angegeben	hstate = HLockH((Handle)o);	op = *o;	// Den Rechner kann man nur mit dem Directory ermitteln	if (!op->type) {					// Typ noch nicht gesetzt?		if (op->dir) {			if (!IQFindFile(o,TIME)&&IQFindFile(o,EXPENSE)) {	// Time Accouting-Manager vorhanden?				op->type = ZQ5000;			// Dann SICHER ein 5000er!			} else {				if (IQFindFile(o,OUTLINE)) {	// Outliner vorhanden?					op->type = IQ8000;		// dann ein 8000er					if (IQFindFile(o,TODO)) {	// �To Do� Liste vorhanden?						op->type = IQ8400;	// dann ein 8400er					}				} else {					op->type = IQ7000;		// sonst ein 7000er					if (IQFindFile(o,USERFILE1)) {	// Userfiles vorhanden?						op->type = IQ7600;	// dann ein 7600er					}				}			}			op->type &= ~TIMEEXPCARD;			if (IQFindFile(o,TIME))				op->type |= TIMEEXPCARD;			op->type &= ~SPREATSHEET;			if (IQCheckSpreatsheet(o))				op->type |= SPREATSHEET;		} else {			op->type = gS.aktOrganizer;		// Default-Organizer		}	}	if ((op->type & IQ8000)||(op->type & IQ8400)) {		op->width = 40;					// Breite vom Display		op->height = 8;					// H�he vom Display	} else {							// Die Displaygr��e ist vom 5000er und 7000er gleich		op->width = 16;					// Breite vom Display		op->height = 8;					// H�he vom Display	}	if (IQIsAvailable(o,USERFILE1))		// Gibt es UserFiles?		CreateUserFileDefaults(o);		// Dann n�tigenfalls Freifeld-Defaults erstellen		// Die Stadt l��t sich GAR NICHT ermitteln	GetIndString(s,1002,1);				// Default-Stadt ermitteln	StringToNum(s,&num);	op->stadt	= num;					// und merken	// Datumsformat aus dem Macintosh ermitteln	op->CalendarMode = true;			// Montag -> Sonntag	op->TimeMode = (*int0)->timeCycle != 0;		// 12 oder 24h Uhr	op->DataMode = (*int0)->dateOrder == dmy;	// DD.MM.YYYY	lang = 0;							// US-English ist default	switch((*int0)->intl0Vers>>8) {	case	verGermany:	lang = 2; break;	case	verFrance:	lang = 3; break;	case	verItaly:	lang = 4; break;	case	verSpain:	lang = 5; break;	case	verSweden:	lang = 6; break;	case	verFinland:	lang = 7; break;	}	op->language = lang; 				// Language	HSetState((Handle)o,hstate);}/*** *	zwei Directorys mergen (doppelte Eintr�ge nicht �bernehmen) * *	(b wird zu a hinzugef�gt) ***/OSErr		IQMergeDirs(REG Handle a,REG Handle b){REG ULONG	sizn,offn;REG ULONG	sizo,offo;REG OSErr	err = noErr;	sizn = GetHandleSize(a);			// Gr��e des neuen Directory	sizo = GetHandleSize(b);			// Gr��e des alten Directory	for(offo=0;offo<sizo;offo += 21) {	// im alten Directory suchen		HLock(a);		HLock(b);		for(offn=0;offn<sizn;offn += 21) {	// neues Directory durchgehen			if (!strncmp(*b+offo+8,*a+offn+8,11)) // gleicher Eintrag?				break;					// dann raus		}		HUnlock(b);		HUnlock(a);		if (offn == sizn) {				// Eintrag noch nicht vorhanden!			SetHandleSize(a,sizn+21);	// neue Gr��e = alte Gr��e + einen Eintrag			err = MemError();			if (err<0) break;			// Fehler => raus			BlockMove(*b+offo,*a+sizn,21); // Block �bertragen			sizn = GetHandleSize(a);	// L�nge updaten		}	}	return(err);}/*** *	Organizer-Struktur initialisieren und f�llen ***/OSErr	IQGetOrganizer(REG OrganizerH o){UBYTE			buf[SERIALBUF];REG OSErr		err;REG WORD		oldType;REG UCHAR		hstate;Handle			d;REG OrganizerP	op;	if (!o) return(-1);					// kein Organizer angegeben	hstate = HLockH((Handle)o);	op = *o;	oldType = op->type;	if (op->dir) {		DisposHandle(op->dir);			// alten Directory-Block freigeben		op->dir = nil;	}	err = SDWriteCmd('\003Z');			// CHR$(3)+"Z" senden	if (err) return(err);	SDSetBrk(0x1A);						// bis CTRL-Z lesen	err = SDRead(buf,SERIALBUF);		// Bytes einlesen	if ((err < 0)&&(err != volOffLinErr)) return(err);	if (err == volOffLinErr) {			// keine Bytes angekommen, d.h. alter Rechner		err = IQReadDir(LINKFORM1,'D',&op->dir);		if (err == noErr) {			err = IQReadDir(LINKFORM1,'d',&d);	// altes Directory einlesen			if (!err)				err = IQMergeDirs(op->dir,d);			else				err = noErr;			if (d) DisposHandle(d);		// zweites Directory freigeben			op->type = 0;			IQSetDefault(o);			// Default-Organizer setzen		}	} else {		REG UWORD	i;		op->type = IQ8000;					// Default: ein 8000er		SDSetBrk(0x1A);						// bis CTRL-Z lesen		err = SDRead(buf,SERIALBUF);		// Bytes einlesen		if (err<0) return(err);		err = IQReadDir(LINKFORM2,'D',&op->dir);	// neues Directory einlesen		if (!err) {			err = IQReadDir(LINKFORM1,'D',&d);		// altes Directory einlesen			if (!err)				err = IQMergeDirs(op->dir,d);			if (d) DisposHandle(d);					// zweites Directory freigeben			if (!err) {				err = IQReadDir(LINKFORM2,'d',&d);	// altes Directory einlesen				if (!err) { 					err = IQMergeDirs(op->dir,d);				} else					err = noErr;				if (d) DisposHandle(d);				// zweites Directory freigeben			}		}		op->type = 0;		IQSetDefault(o);					// Organizer setzen		i = IQGetHexB(buf);					// System-Status holen		op->CalendarMode = BTstBool(i,2);		op->TimeMode = BTstBool(i,3);		op->DataMode = BTstBool(i,4);		op->language = IQGetHexB(&buf[24]);	// Language holen		op->width = IQGetHexB(&buf[28]);	// Breite vom Display		op->height = IQGetHexB(&buf[32]);	// H�he vom Display	}	if ((err<0)&&(op->dir != nil)) {		// ein Fehler?		DisposHandle(op->dir);				// => Directory-Block freigeben		op->dir = nil;	}	if ((oldType != op->type)&&(oldType>=0))		SetSharpTyp(op->type);	HSetState((Handle)o,hstate);	return(err);}/*** *	Organizer offline schalten ***/OSErr	IQDisconnect(VOID){	return(SDWriteCmd('\004E'));	// CHR$(4)+"E" (Exit) senden}/*** *	Organizer ausschalten ***/OSErr	IQTurnoff(VOID){	return(SDWriteCmd('\004O'));	// CHR$(4)+"O" (Off) senden}/*** *	Sharp offline bzw. ausschalten ***/OSErr	IQSharpOffline(VOID){	SDOpen();						// serielle Schnittstelle (Port A) �ffnen	SDWrite((USTR)"\032XXXX\r\n\032",8);	// aus evtl. �bertragung rausholen�	if (gS.OffFlag)					// automatisches Ausschalten?		IQTurnoff();				// Ja�	else		IQDisconnect();				// Verbindung abbrechen	SDClose();}/*** *	Sharp-IQ-Fehler melden ***/Boolean	IQSharpErr(REG OSErr err){	if (!err) return(false);	switch(err) {	case volOffLinErr:			DoDialog(dSharpOffline,1);	// Sharp meldet sich nicht			break;	case userCanceledErr:			DoDialog(dSharpCanceled,1);	// �bertragung abgebrochen			break;	default:			CheckOSError(err);			// evtl. Fehlermeldung ausgeben			break;	}	return(true);}/*** *	Pr�fsumme einlesen und testen ***/OSErr	IQEmpfChecksum(VOID){REG OSErr	err;UBYTE		s[8];UWORD		c = SDChecksum;			// Pr�fsumme merken NUN merken	SDSetBrk(0x1A);					// ab jetzt bis zum CTRL-Z lesen	err = SDRead(s,8);				// Pr�fsumme einlesen	if (err<0) return(err);	if ((IQGetHexB(&s[0])|IQGetHexB(&s[2])<<8) != c)		return(ioErr);				// Pr�fsumme ist falsch!	else		return(noErr);}/*** *	Einen Eintrag aus dem Directory vom Organizer lesen ***/OSErr	IQReadFile(REG OrganizerH o,REG IQFileType index){REG UWORD	i;REG ULONG	size;								// Gr��e vom DirectoryREG ULONG	offs;REG OSErr	err = noErr;REG UWORD	flag = ((*o)->type & RAMCARD)?'8':'0';// Ramkarte auslesen?	if (!o) return(-1);						// kein Organizer angegeben	if (!(*o)->dir) return(-1);				// kein Directory	size = GetHandleSize((*o)->dir);		// Gr��e vom Directory	HLock((*o)->dir);	for(i=0;i<sizeof(IQS)/sizeof(IQInfoStruct);i++) {	// alle Eintr�ge durchgehen		if (err) break;		if (IQS[i].id == index) {						// Eintrag gefunden			for(offs=0;offs<size;offs += 21) {			// im Directory suchen				if (!strncmp(*((*o)->dir)+offs+8,(STR)IQS[i].name,11)&&					(*(*((*o)->dir)+offs+5)==flag)) {	// gleicher Eintrag?					err = IQReadEntry(o,offs/21L,IQS[i].r,index);	// Dir-Index �bergeben					break;				}			}		}	}	HUnlock((*o)->dir);	return(err);}/*** *	Einen Eintrag aus dem Directory vom Organizer lesen ***/OSErr		IQReadEntry(REG OrganizerH o,REG ULONG offset,OSErr	(*f)(OrganizerH o,REG IQFileType index),IQFileType index){REG UWORD		header = IQGetHeader(o);REG OSErr		err;REG USTR		s;REG UBYTE		buf[SERIALBUF];	if (!o) return(-1);					// kein Organizer angegeben	SDSetBrk('\r');						// Bis zum CR lesen	err = SDWriteCmd('R'|header);		// Lesen vom Organizer	if (err) return(err);	if (header == LINKFORM2<<8) {		static USTR s = (USTR)"0000\t0000\t01222222\t\r\n";		err = SDWrite(s,(UWORD)Strlen((STR)s));	// Info Block 2 senden		if (err) return(err);	}	s = (UCHAR *)(*(*o)->dir)+offset*21;	BlockMove(s,buf,21);	buf[3] = '0';						// eigene Flags l�schen (ist beim Sharp eh immer '0')	buf[4] = '1';						// Lesen VOM Organizer	err = SDWrite(buf,21);				// Info Block 1 senden	if (err) return(err);	err = IQGetError(o);				// Fehlercode holen	if (err) return(err);	SDSetBrk('\n');						// Bis zum LF lesen	if (header == LINKFORM2<<8) {		// Info Block 2 �berlesen		err = SDRead(buf,200);		if (err<0) return(err);	}	SDChecksum = 0;						// Pr�fsumme l�schen	err = SDRead(buf,200);				// Info Block 1 Teil 1 lesen	if (err<0) return(err);	if (err<7) return(ioErr);			// Info Block zu kurz	err = SDRead(buf,200);				// Info Block 1 Teil 2 lesen	if (err<0) return(err);	if (err<7) return(ioErr);			// Info Block zu kurz	SDSetBrk('\n');						// ab jetzt bis zum CR/LF lesen	if ((index>=USERFILE1)&&(index<=USERFILE3))		// ein Userfile (7700er)?		err = (*f)(o,index-USERFILE1+USER1FREE);	// erst die Freifelder einlesen	if (err>=0)		err = (*f)(o,index);			// Daten einlesen	if (err<0) return(err);	return (IQEmpfChecksum());			// Pr�fsumme einlesen und testen}/*** *	Sharpuhrzeit und Ort setzen ***/OSErr	IQSetTime(REG OrganizerH o){REG OSErr	err;Str63		data;REG USTR	s = &data[0];DateTimeRec	d;	if (!o) return(-1);				// kein Organizer angegeben	if (IQGetHeader(o) != (LINKFORM2<<8)) return(ioErr);	// klappt nur auf neueren Sharps	err = SDWriteCmd('\003T');		// CHR$(3)+"T" senden	if (err) return(err);			// Abbruch, wenn Fehler	IQSetHexB(s,(*o)->stadt);		// Ort = Berlin	s += 2;	*s++ = 13; *s++ = 10;	GetTime(&d);					// Uhrzeit vom Macintosh holen	s = IQSetDez(s,4,d.year);	s = IQSetDez(s,2,d.month);	s = IQSetDez(s,2,d.day);	s = IQSetDez(s,2,d.hour);	s = IQSetDez(s,2,d.minute);	*s++ = 13; *s++ = 10; *s = 0;	SDChecksum = 0;					// Pr�fsumme l�schen	err = SDWrite(data,s-data);		// Zeit abschicken	if (err) return(err);	err = IQSendChecksum(o);	return(err);}