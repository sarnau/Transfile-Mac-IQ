/*** *	Struktur f�r AKTDOC.data ***/#pragma once#include "GlobalDefines.h"#include "IQStruct.h"#include <Printing.h>typedef struct {	Organizer	o;						// Daten vom Organizer	TPrint		print;					// Druckerstruktur	Boolean		printvalid;				// true: print-Struktur vorhanden	Boolean		OwFlag;					// true: Overwrite ist default beim Einlesen	Boolean		TimeSend;				// true: Uhrzeit mitsenden	Boolean		Idx[MaxIQFileType];		// selektierte Eintr�ge im Empfangsdialog	short		display[MaxIQFileType]; // darzustellende Eintr�ge als Bitarray	Str255		font;					// aktueller Font	short		fontSize;				// und Gr��e	short		columnOffset;			// Offset der rechten Spalte (Telefon etc.)	Boolean		fieldNames;				// true: Feldnamen werden mitgedruckt	Boolean		printHeader;			// true: Seitenheader wird gedruckt	short		columnsPerPage;			// Zahl der Spalten} AktDocStruct,*AktDocP,**AktDocH;#define ADOC		(**((AktDocStruct**)AKTDOC.data))