/*** *	Standard List Definition Procedure for simple text * *  Ernie Beernink  March 1985 * *	This is the standard List defProc.  Its primary task is to draw *	the contents of a list manager cell, either selected or deselected. *	It is passed a pointer to the cell's data, the length of the cell's *	data, and a rectangle in which to draw the cell.  When it is called, *	the clip region is set to that rect.  The cell data does NOT include a *	length byte. ***/#include <Script.h>#include <GestaltEqu.h>#define LineOffset		16/*** *	GetGestaltResult() ermittelt den R�ckgabewert der Gestalt Funktion. Falls ein *	Fehler auftritt, so wird 0 zur�ckgegeben. Somit ist diese Funktion nur dann *	brauchbar, wenn kein Fehler auftreten kann (Gegenbeispiel: AUX-Version erfragen) ***/long	GetGestaltResult(OSType gestaltSelector){long	gestaltResult;		if (Gestalt(gestaltSelector,&gestaltResult) == noErr)		return(gestaltResult);	else		return(0);}pascal void		main(short LMessage,Boolean LSelect,Rect *LRect,Cell LCell,short LDataOffset,						 short LDataLen,ListHandle LHandle){ListPtr		p = *LHandle;char		c;	switch(LMessage) {	case lInitMsg:	{ FontInfo	f;					GetFontInfo(&f);					p->indent.v = f.ascent;					p->indent.h = 4;					}					break;	case lDrawMsg:	{ Point	pnt = topLeft(*LRect);					AddPt(p->indent,&pnt);					MoveTo(pnt.h,pnt.v);	// Pen auf die Indent-Position setzen					PenNormal();			// und den Stift zur�cksetzen					c = HGetState(p->cells);					HLock(p->cells);					EraseRect(LRect);		// Rechteck l�schen					if (LDataLen > 0) {		// Daten �berhaupt vorhanden?						short	len		= LDataLen;						short	width	= LRect->right - LRect->left - p->indent.h - 1											- LineOffset;						if (TextWidth(*p->cells,LDataOffset,len) <= width) {							DrawText(*p->cells,LDataOffset,len);						} else {							short	face = p->port->txFace;							Str255	s;							Boolean	trunc = false;							TextFace(face | condense);//							if (len & 1) len++;	// L�nge sollte gerade sein							BlockMove(*p->cells + LDataOffset,s,len);							if (GetGestaltResult(gestaltSystemVersion) >= 0x0700) {								TruncText(width,(char*)s,&len,smTruncEnd);							} else {								while((TextWidth(s,0,len) > width)&&(len > 0)) {									if (!trunc) {										width -= CharWidth('�');										trunc = true;									}									len--;								}							}							if (len) {								DrawText(s,0,len);								if (trunc)									DrawChar('�');							}							TextFace(face);						}					}					HSetState(p->cells,c);					}	case lHiliteMsg:{					short	iconid	= (LSelect)?0x3000:0x3001;					Handle	h		= GetResource('ics#',iconid);					if (!h) {						if (LSelect) {			// Icon selektiert?							Rect	r = *LRect;							r.left = r.right - LineOffset;							InvertRect(&r);		// einfach nur invertieren						}						break;					}					{					BitMap	BoxBitmap;			// BitMap f�r das eigenen Windowelement					Rect	sr,dr;					GrafPtr	aktPort;					GetPort(&aktPort);					HLock(h);					sr.top = 0; sr.left = 0;					sr.right = LineOffset; sr.bottom = LineOffset;					dr = *LRect;					dr.left = dr.right - LineOffset;					BoxBitmap.baseAddr = *h;					BoxBitmap.rowBytes = 2;					BoxBitmap.bounds = sr;					CopyBits(&BoxBitmap,&(*LHandle)->port->portBits,&sr,&dr,srcCopy,nil);					HUnlock(h);					ReleaseResource(h);					}					}					break;	case lCloseMsg:	break;	}}