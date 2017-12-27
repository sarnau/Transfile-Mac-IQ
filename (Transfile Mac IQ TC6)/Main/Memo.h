//// Memo.h: Header-File f�r Memo.c//#pragma once#include "Windows.h"#include "IQStruct.h"#include "xRsrcDefines.h"extern char	zeilenDerTerminarten[9];#define SECOND_COLUMN_OFFSET	100		// Einr�ckung zw. Bezeichnung und Daten#define COLUMN_LINE_OFFSET		5		// Abstand zw. Daten und Linie#define	RETURN					'\r'	// Feldtrenner#define CRLF					'\t'	// Cr eines Sharps (i. e. Tab)#define END_OF_LIST				255		// Endekennung f�r Feldnamenliste#define MAX_NAMES				18		// 18 Eintr�ge f�r Freifeldnamen etc.#define MAX_LENGTH				62		// L�nge von Str63 - Style-Byte#define VPAGE					0		// Scrollfaktor Vertikal 0=Ganze Seite#define MARK_CHAR				'*'		// F�r Mark*ierte Datens�tze//#define FOUR_CHAR				'4'		// F�r 4-zeilige     -"-//#define EIGHT_CHAR			'8'		// F�r 8-zeilige     -"-#define WYSIWYG					40		// Statt WYSIWYG konstant 40 Zeichen#define WINDOW_BORDER			16		// Defines f�r Window-Staggering#define Y_ADD					18#define X_ADD					5#define WINDOW_MIN_X			130#define WINDOW_MIN_Y			70typedef struct {	short	visible;					// Maske, welche Felder sichtbar sind	short	setLength;					// Maximall�nge des gesamten Datensatzes	Str63	text[MAX_NAMES];			// Feldnamen der einzelnen Felder	short	maxLen[MAX_NAMES];			// Maximall�nge der einzelnen Felder	char	data[4200];					// Inhalt des aktuellen Datensatzes}dataset,*datasetP,**datasetH;typedef enum {redrawSingle, redrawAll, redrawAndRecalc}RedrawType;typedef enum {displayNormal,displayEditor,displayPrint}displayType;typedef enum {getTheNextOne,getThePreviousOne}TheMode;OSErr	OpenMemoWindows(void);WindPtr	MyOpenWind(long Type,short id,short titleIndex,IQFileType type);void	testadjust(WindPtr w);void	testklick(void);ListH	NextListH(ListH handle, TheMode mode);void	FillOut(IQFileType type, datasetH actset);void	DisplayEntry(int x, int y, dataset **actset, int zeilenHoehe, ListH lHandle, displayType displayMode);ListH	LookForEntry(long zeilen,ListH handle,long *y,long maxsize,IQFileType type);short	EntryLength(ListH handle,IQFileType type);short	CountSpecialLines(IQFileType type);void 	CopyListToDataset(datasetH theSet, ListH theList);void	AddTimeToString(char *s, char hour, char minute,DateTimeRec	*theDate);Boolean	CheckEntry(ListH handle,WindPtr w);void	testactivate(void);void	ShortenEntry(char *text);void	memoupdate(void);void	multidraw(void);short	FieldWidthCalc(ListH handle);void	GetFreeFields(datasetH actset, ListH lhandle, IQFileType type, IQFileType first, IQFileType data,short start);void	MultiOpen(WindPtr w1, IQFileType type);void	BearbeitenMenu(short entry);void	DeselectAllEntries(IQFileType type);void	DoMultiRedraw(IQFileType type, RedrawType rType, ListH lHandle);void	DoBitFieldRedraw(ULONG typearray);Boolean	DoMultiRedrawSub(void);void	DoMark(SSType what,WORD id);Boolean	GetTelName(REG IQFileType id,USTR s);void	MySearchOList(IQFileType type,char *s);