////	Editor.h//#include "IQStruct.h"#define MAX_EDITOR_FONT_SIZE	10		// 10pt Maximalgr��e#define MIN_EDITOR_MEM			15000	// Unter 15kB RAM l�uft nix#define TE_LENGTH				4200	// L�nge meines TextEdit-Buffers#define EDITOR_FONT				courier#define ALLOW_RETURN			30		// Minimale Feldl�nge, ab der Returns erlaubt sind.typedef struct {	unsigned redrawNecessary:1;		// Ein Redraw ist n�tig;	unsigned recalcNecessary:1;		// Auch eine Sliderneuberechnung	unsigned leftArrow:1;			// Voriger Datensatz erw�nscht	unsigned rightArrow:1;			// N�chster Datensatz erw�nscht}EditRetType;EditRetType	Editor(ListH handle, IQFileType dataType, DateTimeRec *defaultDate);pascal Boolean	EditFilter(DialogPtr d,EventRecord *event, short *item);void	DoScrollBar(DialogPtr d,short iType);void	SetDialogRect(int entry, char *editText, DialogPtr dialog);void	CalcFieldLength(char *d, short w, DialogPtr dialog);void	ScrollAfterChangeEntry(DialogPtr d);void	ScrollAfterCursorKey(DialogPtr d);void	AddReturnsIfNecessary(ListH lHandle);Boolean	TestInsertLength(short number);void	WriteDataToList(short entry, CharsHandle textFromTE);Boolean	TimeToList(char *ptr,short laenge,char *hour,char *minute);Boolean	Ann1ToList(char *ptr,short laenge,char *month,char *day);Boolean	Ann2ToList(char *ptr,short laenge,char *month,char *week,char *day);Boolean DateToList(char *ptr,short length,short *year,char *month,char *day,Boolean ignoreDay);IQFileType	ChooseSchedulerType(void);