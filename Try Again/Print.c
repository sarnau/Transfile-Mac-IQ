/*** *	alle Routinen zum Drucken ***/#include "Print.h"#include "GlobalLib.h"#include "File.h"#include "AktDocStruct.h"#include "GlobalStruct.h"#include "DialogLib.h"#define	JobDlgID		-8191	/* resource ID of 'Job' or 'Print' DLOG, DITL, & hdlg		 */#define	StlDlgID		-8192	/* resource ID of 'Style' or 'PageSetup' DLOG, DITL, & hdlg	 */short		PrintStlDITL = 0;		// eigene DITL-Resource-IDshort		PrintJobDITL = 0;		// eigene DITL-Resource-ID// Types for accessing the Printing Manager dialog resources.static TPPrDlg	PrtJobDialog;		// pointer to job dialogstatic TPPrDlg	PrtStlDialog;		// pointer to style dialogshort			prFirstItem;		// erstes eigenes Item im Dialog-HookProcPtr			prPItemProc;		// Originalroutine f�r Dialog-Hooktypedef pascal Boolean (*PrDialogProcPtr)(TPPrDlg theDialog, short itemNo);pascal void		MyJobItems(TPPrDlg theDialog,short itemNo);pascal TPPrDlg	MyJobDlgInit(THPrint hPrint);pascal void		MyStlItems(TPPrDlg theDialog,short itemNo);pascal TPPrDlg	MyStlDlgInit(THPrint hPrint);pascal void		PrintIdleProc(void);short			gPrintPage;DialogPtr		PrintingStatusDialog;/*** *	Druckersetup-Einstellungen ***/OSErr		PageSetup(REG DocHandle d){REG OSErr		err;REG THPrint		p;	if (!d) return;								// kein Dokument offen	PrOpen();	err = PrError();	if ((err == fnfErr)||(err == -8172)) err = -8150;	if (!err) {		p = (THPrint)NewHandle(sizeof(TPrint)); // neue Struktur allozieren		if (!p) return(memFullErr);				// Speicher reichte nicht!		BlockMoveData(&ADOC.print,*p,sizeof(TPrint)); // Dokumentendaten �bertragen		if (!ADOC.printvalid)					// War die Struktur g�ltig?			PrintDefault(p);					// Nein, erstmal initialisieren		else			PrValidate(p);						// oder nur abtesten		if (!(err = PrError())) {			pfeil();			PrtStlDialog = PrStlInit(p);		// Dialogbox laden			if (PrError() == noErr) {				if (PrintStlDITL>0)					Append2hdlg(PrintStlDITL,StlDlgID);				if (PrDlgMain(p,MyStlDlgInit)) {					BlockMoveData(*p,&ADOC.print,sizeof(TPrint)); // in Dokumenten-Struktur kopieren					ADOC.printvalid = true;		// Dokumentendaten nun g�ltig!					AppDocumentDirty(gDoc,true);// Dokument dirty!				} else					err = userCanceledErr;		// Abbruch!			}		}		DisposHandle((Handle)p);				// Handle wieder verwerfen	}	PrClose();	return(err);}/*** *	 ***/OSErr	AppPrintDocument(REG DocHandle d,Boolean jobDlg,Boolean firstJob){REG OSErr		err;REG THPrint		p;REG TPPrPort	printPort;GrafPtr			oldPort;REG short		i,keepResFile,fstPage,lstPage,copies;TPrStatus		status;Str63			s;Boolean			valid;static THPrint	prMergeHndl = nil;	if (!d) {									// abmelden?		if (prMergeHndl) {						// Handle noch vorhanden?			DisposHandle((Handle)prMergeHndl);	// dann freigeben			prMergeHndl = nil;		}		return(noErr);							// alles ok!	}	PrintingStatusDialog = nil;	p = (THPrint)NewHandle(sizeof(TPrint));		// neue Struktur allozieren	if (!p) return(memFullErr);					// Speicher reichte nicht!	BlockMoveData(&ADOC.print,*p,sizeof(TPrint));	// Struktur f�llen!	GetPort(&oldPort);	pfeil();	PrOpen();	err = PrError();	if ((err == fnfErr)||(err == -8172)) err = -8150;	if (!err) {		keepResFile = CurResFile();		if (!ADOC.printvalid) {			PrintDefault(p);					// The document print record was never 			err = PrError();					// initialized. Now is is.		}					if (!err) {			PrValidate(p);						// Do this just 'cause Apple says so.			err = PrError();		}		pfeil();		if (!err) {			if (jobDlg) {						// User gets to click some buttons.				PrtJobDialog = PrJobInit(p);	// Dialogbox laden				if (PrError() == noErr) {					if (PrintJobDITL>0)						Append2hdlg(PrintJobDITL,JobDlgID);					if (!PrDlgMain(p,MyJobDlgInit))						PrSetError(err = userCanceledErr);					else						err = PrError();				}			}		}		if (!err) {			if (!firstJob) {				PrJobMerge(prMergeHndl,p);				err = PrError();			}		}		if (!err) {			// Put the defaulted/validated/jobDlg'ed print record in the doc.			biene();			fstPage	= (*p)->prJob.iFstPage;			if (fstPage == 0) fstPage = 1;			lstPage	= (*p)->prJob.iLstPage;			if (lstPage == 0) lstPage = 9999;			copies	= (*p)->prJob.iCopies;			if (copies == 0) copies = 1;			BlockMoveData(*p,&ADOC.print,sizeof(TPrint));			ADOC.printvalid = true;			(*p)->prJob.iFstPage = 1;		// �alle� Seiten ausdrucken (LaserWriter LC!)			(*p)->prJob.iLstPage = 9999;			PrintingStatusDialog = GetNewDialog(dPrStatusDlg,nil,(WindowPtr)-1);			if (PrintingStatusDialog) {				HLock((Handle)gDoc);	// Windownamen setzen, damit der Druckdatei-Name stimmt!				SetWTitle((WindowPtr)PrintingStatusDialog,AKTDOC.f.fss.name);				HUnlock((Handle)gDoc);				PrValidate(p);	// Do this just 'cause Apple says so.				HiliteButton(PrintingStatusDialog,1,255);				NumToString(fstPage,s);				ParamText(AKTDOC.f.fss.name,s,nil,nil);	// Dokumentname + Seitennummer				DrawDialog(PrintingStatusDialog);					// Hook in the proceed/pause/cancel dialog.			}			for (i=1; (i<=copies) && (!err); ++i) {				Boolean		openFlag = false;				// Dialogbox-Hook eintragen (Apple machts auch an dieser Stelle�)				(*p)->prJob.pIdleProc = (PrIdleProcPtr)PrintIdleProc;				// Restore the reource file to the printer driver�s				UseResFile(keepResFile);				for(gPrintPage = fstPage; gPrintPage <= lstPage; gPrintPage++) {					if ((gPrintPage - fstPage) % iPFMaxPgs == 0) {	// max. 128 Seiten!						if (gPrintPage != fstPage) {		// erster Durchgang?							PrCloseDoc(printPort);			// sonst das Dokument schlie�en!							PrPicFile(p,nil,nil,nil,&status);							err = PrError();						}						if (err) break;						// Fehler => Abbruch						printPort = PrOpenDoc(p,nil,nil);						openFlag = true;					// PrOpen aufgetreten						err = PrError();						if (err) break;						// Fehler => Abbruch						if (gPrintPage == fstPage) {		// erster Durchgang?							if (gG.Print) {								GrafPtr	savePort;								GetPort(&savePort);								SetPort(&printPort->gPort);								gPrintPage = -1;			// Seitennr = -1 <=> Init								{ G(Print)(printPort,p); }	// Init-Aufruf								gPrintPage = fstPage;								SetPort(savePort);							}						}					}					NumToString(gPrintPage,s);					ParamText(AKTDOC.f.fss.name,s,nil,nil);	// Dokumentname + Seitennummer					DrawDialog(PrintingStatusDialog);					PrOpenPage(printPort,nil);					err = PrError();					if (err == noErr) {						if (gG.Print) {							GrafPtr	savePort;							GetPort(&savePort);							SetPort(&printPort->gPort);							{ G(Print)(printPort,p); }							SetPort(savePort);						}					}					PrClosePage(printPort);					if ((!gPrintPage)||					// Seitennummer = 0 => Ende						(err = PrError())) break;		// oder ein Fehler?				}				gPrintPage = 0;				if (openFlag)							// PrOpen aufgetreten?					PrCloseDoc(printPort);			}		}		if (			(!err) &&			((*p)->prJob.bJDocLoop == bSpoolLoop) &&			(!(err = PrError()))		) {			PrPicFile(p,nil,nil,nil,&status);			err = PrError();		}	}	if (firstJob)	prMergeHndl = p;	else			DisposHandle((Handle)p);	if (PrintingStatusDialog) DisposDialog(PrintingStatusDialog);	PrClose();	SetPort(oldPort);	pfeil();	return(err);}/* PrintIdleProc will handle events in the 'Printing Status Dialog' which** gives the user the option to 'Proceed', 'Pause', or 'Cancel' the current** printing job during print time.**** The buttons:**		1: Proceed**		2: Pause**		3: Cancel*/pascal void		PrintIdleProc(void){REG Boolean		button,paused;DialogPtr		d;EventRecord		anEvent;GrafPtr			oldPort;short			item,keepResFile;short			type;Handle			h;Rect			r;static short	lastPage = -1;	GetPort(&oldPort);	UseResFile(keepResFile = CurResFile());	HiliteButton(PrintingStatusDialog,1,255);	paused = false;	do {		if (paused)			pfeil();		// W�hrend der Pause als Mauspfeil umschalten		else			biene();		if (UserAbort()) {			PrSetError(iPrAbort);               // Drucken abbrechen			gPrintPage = 0;						// abbrechen!			break;		}		GetNextEvent(mDownMask+mUpMask+updateMask,&anEvent);		if (lastPage != gPrintPage) {					// Seitennummer updaten			GetDItem(PrintingStatusDialog,4,&type,&h,&r);			SetPort(PrintingStatusDialog);			InvalRect(&r);								// neu zeichnen			lastPage = gPrintPage;		}		if (PrintingStatusDialog != FrontWindow())		// Drucken-Dialog			SelectWindow(PrintingStatusDialog);			// ist STETS das oberste Window		if (IsDialogEvent(&anEvent)) {			button = DialogSelect(&anEvent,&d,&item);			if ((button) && (d == PrintingStatusDialog)) {				switch (item) {					case 1:						HiliteButton(PrintingStatusDialog,2,0);		// Enable PAUSE						HiliteButton(PrintingStatusDialog,1,255);	// Disable PROCEED						paused = false;						break;					case 2:						HiliteButton(PrintingStatusDialog,2,255);	// Disable PAUSE						HiliteButton(PrintingStatusDialog,1,0);		// Enable PROCEED						paused = true;						break;					case 3:						PrSetError(iPrAbort);               // Drucken abbrechen						gPrintPage = 0;						// abbrechen!						paused = false;						break;				}			}		}	} while (paused != false); 	SetPort(oldPort);}/* 	ModifyDialogs	-	Append items onto an existing Printing Manager dialog		Version	When	Who			What	------- ----	---			----	0.1		9/11/86 Lew			Original MyAppendDITL code.					Rollins	1.0		3/01/88	Ginger		Converted Lew's code to Pascal, and published					Jernigan	Technical Note #95.						2.0		5/01/91	Zz			Fixed bug in MyAppendDITL - the call to PtrAndHand								was being passed a size that was 2 bytes too large.								Added Append2hdlg routine to support Help Manager								balloons for the items appended to the dialog.																-----		This program is basically an updated version of the code in Technical Note #95,	"How To Add Items to the Print Dialogs".  There was a bug in the MyAppendDITL	routine provided in that technote that has been fixed in this code.		The latest addition is a routine called Append2hdlg.  This routine is used	to add Help Manager 'Balloons' for the items appended onto the dialog.  As	you might have guessed, the way you add items to the hdlg resource is very	similar to the method used for DITL resources.  You basically append the	data, and then update the item count.		This code was written under some time contraints, so it hasn't been completely	tested, but it has been tested at least once with each of the Apple printer	drivers currently available.		It should also be noted that this code displays both the Page Setup and Print 	dialogs (unlike the original which only displayed the Print dialog), so there	are extra routines.  Basically, for every xxJobxxx routine, there is now a	xxStlxxx routine.  The routines are basically identical, the only difference	is the substitution of Stl for Job in the appropriate places.  The duplication	of code is actually meant to make things a little clearer.		I hope this helps,		...Zz			-----------------------------------------------------------------------------	NOTE: Apple reserves the top half of the screen (where the current DITL	items are located). Applications may use the bottom half of the screen to add 	items, but should not change any items in the top half 	of the screen.  An 	application should expand the print dialogs only as much as is absolutely 	necessary.	-----------------------------------------------------------------------------*//*** *	eigener Dialogbox-Hook f�r den Drucker-Job-Dialog ***/pascal void MyJobItems(REG TPPrDlg theDialog,REG short itemNo){REG short 	myItem;REG short 	firstItem;REG short	itemType;		// needed for GetDItem/SetDItem callREG Handle	itemH;Rect		itemBox;		firstItem = prFirstItem;			// remember, we saved this in myJobDlgInit	myItem = itemNo - firstItem + 1;	// "localize" current item No	if (myItem > 0) {					// if localized item > 0, it's one of ours		{ G(PrintDlg)(JobAktion,(DialogPtr)theDialog,itemNo); }	// Aktion f�r Job-Dialog	} else {					// chain to standard item handler, whose address is saved in prPItemProc		if (itemNo == 1) { G(PrintDlg)(JobExit,(DialogPtr)theDialog,0); }	// Ende vom Dialog		(*((PrDialogProcPtr)prPItemProc))(theDialog,itemNo);	}}/*** *	Diese Routine klinkt eine eigene Routine in den Drucker-Job-Dialog ein ***/pascal TPPrDlg MyJobDlgInit(REG THPrint hPrint){	if (PrintJobDITL>0) {		prFirstItem = MyAppendDITL((DialogPtr)PrtJobDialog,PrintJobDITL); // eigene DITL-Resource anf�gen		prPItemProc = (ProcPtr)PrtJobDialog->pItemProc;		// Original-Handler merken		PrtJobDialog->pItemProc = (PItemProcPtr)MyJobItems;	// eigenen Handler aufrufen		{ G(PrintDlg)(JobInit,(DialogPtr)PrtJobDialog,prFirstItem); }	// Init f�r Job-Dialog	}	return(PrtJobDialog);								// Dialogpointer zur�ckgeben}/*** *	eigener Dialogbox-Hook f�r den Drucker-Stl-Dialog ***/pascal void MyStlItems(REG TPPrDlg theDialog,REG short itemNo){REG short 	myItem;REG short 	firstItem;short		itemType;		// needed for GetDItem/SetDItem callHandle		itemH;Rect		itemBox;		firstItem = prFirstItem;			// remember, we saved this in myJobDlgInit	myItem = itemNo - firstItem + 1;	// "localize" current item No	if (myItem > 0) {					// if localized item > 0, it's one of ours		{ G(PrintDlg)(StlAktion,(DialogPtr)theDialog,itemNo); }	// Aktion f�r Stl-Dialog	} else {					// chain to standard item handler, whose address is saved in prPItemProc		if (itemNo == 1) { G(PrintDlg)(JobExit,(DialogPtr)theDialog,0); }	// Ende vom Dialog		(*((PrDialogProcPtr)prPItemProc))(theDialog,itemNo);	}}/*** *	Diese Routine klinkt eine eigene Routine in den Drucker-Stl-Dialog ein ***/pascal TPPrDlg MyStlDlgInit(THPrint hPrint){	if (PrintStlDITL>0) {		prFirstItem = MyAppendDITL((DialogPtr)PrtStlDialog,PrintStlDITL); // eigene DITL-Resource anf�gen		prPItemProc = (ProcPtr)PrtStlDialog->pItemProc;		// Original-Handler merken		PrtStlDialog->pItemProc = (PItemProcPtr)MyStlItems;	// eigenen Handler aufrufen		{ G(PrintDlg)(StlInit,(DialogPtr)PrtStlDialog,prFirstItem); }	// Init f�r Stl-Dialog	}	return(PrtStlDialog);								// Dialogpointer zur�ckgeben}