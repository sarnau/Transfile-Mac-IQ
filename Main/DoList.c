/*** *	DoList.c ***/#include "GlobalDefines.h"#include "Windows.h"#include "xRsrcDefines.h"#include "Memo.h"#include "DoList.h"#include "Telephone.h"/*** *	Window �ffnen ***/OSErr	OpenDoListWindows(void)		/* Window f�r Do List �ffnen */{	REG WindPtr	w1;	w1 = MyOpenWind(FULLER | (1L<<noOrigin),WindDoList,strDoList,DOLIST);	/* Window mit Slidern */	if(!w1) 		return(memFullErr);				// Fehler beim �ffnen 	MultiOpen(w1,DOLIST);	WFUNC(w1,menuupdate,telupdate);		// Darstellung disabeln etc.	return(noErr);								// alles ok!}