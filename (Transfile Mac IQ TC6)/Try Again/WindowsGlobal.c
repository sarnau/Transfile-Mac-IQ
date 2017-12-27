/*** *	WindowsGlobal.c * *	einige zus�tzliche Windowfunktionen, ohne Bezug zur Window-Library * *	�1991 �-Soft, Markus Fritze ***/#include "WindowsGlobal.h"#include "GlobalLib.h"#include "Windows.h"/*** *	Test, ob das Window zur Applikation geh�rt. �windowKind� ist bei System- *	Windows z.B. DAs negativ und �windowKind� kleiner �userKind� sind f�r *	Apple reserviert (mit Ausnahme von �dialogKind�) ***/Boolean		IsAppWindow(REG WindowPtr w){REG	WORD	i;	if (w) {		if (((WindPtr)w)->magic == WINDMAGIC) {			i = ((WindowPeek)w)->windowKind;			if ((i >= userKind)||(i == dialogKind))				return(true);		}	}	return(false);}/*** *	Test, ob das Window zu einem DA geh�rt. ***/Boolean		IsDAWindow(REG WindowPtr w){	if (w)				// DAs haben ein negatives �windowKind�		if (((WindowPeek)w)->windowKind < 0)			return(true);	return(false);}/*** *	Ist das Window eine Dialogbox? ***/Boolean		IsDialogWindow(REG WindowPeek w){	if (w) {		if	(w->windowKind == dialogKind)			return(true);	}	return(false);}/*** *	H�he der Windowtitelzeile ermitteln ***/WORD		WindowTHeight(REG WindowPtr w){	return((**(((WindowPeek)w)->contRgn)).rgnBBox.top-(**(((WindowPeek)w)->strucRgn)).rgnBBox.top);}/*** *	BringBehind * *	Move a window from far back to right behind another window ***/VOID		BringBehind(REG WindowPtr w,REG WindowPtr behindWindow){GrafPtr		savePort;	// Current portPoint		corner;		// Top left of visible region	if (!behindWindow) {			// hinter �kein� Window?		BringToFront((WindowPtr)w);	// dann Window ganz nach vorne		return;	}	GetPort(&savePort);				// Save current port	SetPort(w);						// Use this window's port	CopyRgn(w->visRgn,gUtilRgn);	// Save portion of window which is originally visible	SendBehind(w,behindWindow);		// Adjust the window's plane	// We must draw the newly exposed portion of the window. Find the	// difference between the present structure region and what was	// originally visible. Before doing this, we must convert the	// originally visible region to global coords.	corner = topLeft((**gUtilRgn).rgnBBox);	LocalToGlobal(&corner);	OffsetRgn(gUtilRgn, (corner.h - (**gUtilRgn).rgnBBox.left),						(corner.v - (**gUtilRgn).rgnBBox.top));	// Now we can difference the regions. Save space by putting the	// result back in theRgn. Before calling DiffRgn, theRgn is the	// originally visible region. Afterwards, theRgn is the newly	// exposed region of the window.	DiffRgn(((WindowPeek)w)->strucRgn,gUtilRgn,gUtilRgn);	PaintOne((WindowPeek)w,gUtilRgn);	// Draw newly exposed region	// Since window has moved forward, we must adjust the visible	// regions of this window and those behind it.	CalcVisBehind((WindowPeek)w,((WindowPeek)w)->strucRgn);	SetPort(savePort);					// Restore the original port}/*** *	Window zoomen (INTERNE Funktion! Will man ein Window zoomen, so ist WZoomClick() *	aufzurufen!) ***/VOID		ZoomToWindowDevice(REG WindowPtr w,LONG maxWidth,LONG maxHeight,WORD zoomDir,Boolean front){Rect			windRect,theSect,zoomRect,userRect;REG GDHandle	nthDevice,dominantGDevice;REG ULONG		sectArea,greatestArea;REG WORD		bias;	EraseRect(&w->portRect);			// Windowinhalt komplett l�schen	// Zoom-Rechteck aus der Window-Struktur holen	zoomRect = (*(WStateDataHandle)((WindowPeek)w)->dataHandle)->stdState;	userRect = (*(WStateDataHandle)((WindowPeek)w)->dataHandle)->userState;	if ((zoomDir == inZoomOut)&&(gQDVersion)) {	// Verg��ern und Color-Quickdraw?		windRect = w->portRect;		LocalToGlobal(&topLeft(windRect));	// in globale Koordinaten umrechnen		LocalToGlobal(&botRight(windRect));		bias = windRect.top - 1 - (*((WindowPeek)w)->strucRgn)->rgnBBox.top;		windRect.top -= bias;			// Windowrechteck _ohne_ Titelzeile		nthDevice = GetDeviceList();	// das erste Device holen		dominantGDevice = nil;			// kein Device gefunden!!!		greatestArea = 0;				// noch keine Schnittfl�che gefunden		while(nthDevice) {				// alle Devices durchgehen			if (TestDeviceAttribute(nthDevice,screenDevice))		// Bildschirm?				if (TestDeviceAttribute(nthDevice,screenActive)) {	// und aktiv?					// Schnittrechteck errechnen:					SectRect(&windRect,&(*nthDevice)->gdRect,&theSect);					// Schnittfl�che errechnen:					sectArea = (ULONG)(theSect.right - theSect.left) * (ULONG)(theSect.bottom - theSect.top);					// neues Maximum?					if (sectArea > greatestArea) {						greatestArea = sectArea;		// Fl�che merken						dominantGDevice = nthDevice;	// Device merken					}				}			nthDevice = GetNextDevice(nthDevice);		// zum n�chsten Device		}		if (dominantGDevice == GetMainDevice())			// auf dem Hauptbildschirm?			bias += GetMBarHeight();					// dann die Men�leiste ber�cksichtigen		if (dominantGDevice) {			REG Rect	*r = &(*dominantGDevice)->gdRect;	// Rechteck vom �Zoom�-Device			SetRect(&zoomRect,r->left+3,r->top+bias+3,r->right-3,r->bottom-3);		}	}	// maximale Breite setzen	if ((zoomRect.right - zoomRect.left) > maxWidth)		zoomRect.right = zoomRect.left + maxWidth;	// maximale H�he setzen	if ((zoomRect.bottom - zoomRect.top) > maxHeight)		zoomRect.bottom = zoomRect.top + maxHeight;	// Zoomen verschiebt das Window nur, wenn es _n�tig_ ist!	if ((zoomRect.right - zoomRect.left + userRect.left - 3) < gScreenRect.right)		if ((zoomRect.bottom - zoomRect.top + userRect.top - 3) < gScreenRect.bottom) {			OffsetRect(&zoomRect,-zoomRect.left,-zoomRect.top);			OffsetRect(&zoomRect,userRect.left,userRect.top);		}	// Zoom-Rechteck in der Window-Struktur setzen	(*(WStateDataHandle)((WindowPeek)w)->dataHandle)->stdState = zoomRect;	ZoomWindow(w,zoomDir,front);						// Window zoomen}