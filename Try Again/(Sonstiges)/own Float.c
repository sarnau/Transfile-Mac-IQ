/*** *	WDEF-Funktion f�r Floats ***/#define REG		register#define	FloatTitleHeight	10		// H�he vom Float-Titel#define	FloatShadow			3		// Gr��e vom Windowschattenvoid	DrawWindow(WindowPtr w,long param);long	TestWindowHit(WindowPtr w,long param);void	CalcWindowRegions(WindowPtr w);/*** *	Einsprung in das WDEF-Modul ***/pascal	long main(short var,REG WindowPtr w,short message,REG long param){long	ret = 0L;	switch(message) {	case	wDraw:		DrawWindow(w,param);						break;	case	wHit:		ret = TestWindowHit(w,param);						break;	case	wCalcRgns:	CalcWindowRegions(w);						break;	}	return(ret);}/*** *	Zeichenroutine f�r unser Float ***/void	DrawWindow(REG WindowPtr w,REG long param){Rect		frame;		// diverse RechteckeRect		closer;		// Rechteck vom CloserPenState	savePen;	// geretteter StatusPattern		fill;		// F�llmuster f�r die TitelzeileREG short	i;			// Index	if(!((WindowPeek)w)->visible) return;	// Window unsichtbar => raus	frame = (*((WindowPeek)w)->strucRgn)->rgnBBox;	// Window-Rahmen	frame.bottom -= 1;	frame.right -= 1;	closer = frame;					// Closer-Rechteck errechnen	closer.top += 2;	closer.left += 8;	closer.bottom = closer.top + 7;	closer.right = closer.left + 7;	if (param == wInGoAway) {		InvertRect(&closer);		// Closer selecten		return;	} else if (param == wNoHit) {		GetPenState(&savePen);		PenNormal();		FrameRect(&frame);				// Window-Rahmen zeichnen		MoveTo(frame.left + FloatShadow,frame.bottom); // Schatten zeichnen		LineTo(frame.right, frame.bottom);		LineTo(frame.right,frame.top + FloatShadow);			frame.bottom = frame.top + (FloatTitleHeight + 1);		FrameRect(&frame);				// Titelzeile umrahmen		InsetRect(&frame,1,1);			// Titelrechteck etwas verkleinern		if (((WindowPeek)w)->hilited) {	// Window aktiv?			REG short		f;			REG short		i;			f = (frame.left & 1)? 0x55 : 0xAA;			for(i=0;i<=7;i++)			// F�llmuster zusammensetzen				fill[i] = ((i + frame.top) & 1) ? f : 0;			FillRect(&frame,fill);		// Titelzeile f�llen				if (((WindowPeek)w)->goAwayFlag) {	// Closer vorhanden?				frame = closer;				InsetRect(&frame,-1,-1);				EraseRect(&frame);		// Closer zeichnen				FrameRect(&closer);			}		} else {			for(i=0;i<=7;i++)			// F�llmuster = wei�				fill[i] = 0;			FillRect(&frame,fill);		// Titelzeile in wei�		}		SetPenState(&savePen);	}}/*** *	Routine f�r Klickposition in unserem Window ***/long	TestWindowHit(REG WindowPtr w,long param){Rect		r;REG Point	p = *(Point*)&param;	if (PtInRect(p,&(*((WindowPeek)w)->contRgn)->rgnBBox))		return(wInContent);	r = (*((WindowPeek)w)->strucRgn)->rgnBBox;	r.bottom = r.top + (FloatTitleHeight + 1);	r.right -= 1;	if (PtInRect(p,&r)) {		r.top += 2;		r.left += 8;		r.bottom = r.top + 7;		r.right = r.left + 7;		if ((((WindowPeek)w)->hilited) && (PtInRect(p,&r)))			return(wInGoAway);		return(wInDrag);	}	return(wNoHit);}/*** *	Routine zum Ausrechnen der Regionen ***/void	CalcWindowRegions(REG WindowPtr w){Rect			r;REG RgnHandle	h;	r = w->portRect;				// Content-Rgn errechnen	OffsetRect(&r,-w->portBits.bounds.left,-w->portBits.bounds.top);	RectRgn(((WindowPeek)w)->contRgn,&r);	r.top -= FloatTitleHeight + 1;	// Structure-Rgn errechnen	r.left -= 1;	r.bottom += 1;	r.right += 1;	RectRgn(((WindowPeek)w)->strucRgn,&r);	r.top += FloatShadow;			// Schatten zur Structure-Rgn	r.left += FloatShadow;	r.bottom += 1;	r.right += 1;	RectRgn(h = NewRgn(),&r);	UnionRgn(((WindowPeek)w)->strucRgn,h,((WindowPeek)w)->strucRgn);	DisposeRgn(h);}