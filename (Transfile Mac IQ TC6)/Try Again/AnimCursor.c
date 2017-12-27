#include "AnimCursor.h"/*** *	GetAnimCurs - Retrieve an animated cursor list and its cursors * *	Coding Notes: *	#A# - We need to make the AnimCurs non-purgeable for it�s life span. *	#B# - If the high bit of the frame field is set, AnimCurs is a list of color cursors. *	#C# - The cursor list is a list of CursHandles, but in the resource file the cursor list is a list *        of resource IDs in the upper 16 bits of the CursHandles.  That�s why there�s all this weird *        casting about in the GetCursor and GetCCursor calls. *	#D# - Replace the resource IDs with CursHandles. *	#E# - If the cursor is a non-color cursor ('CURS' resource), make it non-purgeable.  GetCColor *        returns copies of the 'crsr' resources so they are already non-purgeable. ***/pascal AnimCursHnd		GetAnimCurs(short AnimCursID)/* Resource ID of animated cursor list ('acur' resource) >> */{AnimCursHnd AnimCurs;  // => Animated cursor listCursHandle  TheCursor; // => One cursor in AnimCursshort       ColorCurs; // True if cursor list has color cursors, false otherwiseshort       Index;     // Index into animated cursor list	if ((AnimCurs = (AnimCursHnd)GetResource('acur',AnimCursID)) != (AnimCursHnd)nil) {		HNoPurge ((Handle) AnimCurs); // #A#		ColorCurs = (**AnimCurs).frame & 0x8000; // #B#		for (Index = 0; Index < (**AnimCurs).count; Index++) {			if (ColorCurs) // #C#				TheCursor = (CursHandle) GetCCursor (*((short *) &((**AnimCurs).cursors [Index])));			else				TheCursor = GetCursor (*((short *) &((**AnimCurs).cursors [Index])));			(**AnimCurs).cursors [Index] = TheCursor; // #D#			if ((!ColorCurs) && (TheCursor != (CursHandle)nil)) // #E#				HNoPurge((Handle)TheCursor);		}	}	return(AnimCurs);}/*** *	AnimateCurs - Animate a cursor list by one frame * *	Coding Notes: *	#A# - If the high bit of the frame field is set, AnimCurs is a list of color cursors. ***/pascal void		AnimateCurs(AnimCursHnd AnimCurs)	// => Cursor list to animate <>{CursHandle	TheCursor;	// => The cursor to displayshort		ColorCurs;	// True if AnimCurs is a list of color cursorsshort		FrameNum;	// Index in the cursor list of the cursor to display	ColorCurs = (**AnimCurs).frame & 0x8000;	// #A#	FrameNum = (**AnimCurs).frame & 0x7FFF;	if ((TheCursor = (**AnimCurs).cursors[FrameNum]) != (CursHandle)nil)		if (ColorCurs)			SetCCursor((CCrsrHandle)TheCursor);		else {			HLock((Handle)TheCursor);			SetCursor(*TheCursor);			HUnlock((Handle)TheCursor);		}	if (ColorCurs)		(**AnimCurs).frame = ((((**AnimCurs).frame & 0x7FFF) + 1) % (**AnimCurs).count) | 0x8000; // #A#	else		(**AnimCurs).frame = ((**AnimCurs).frame + 1) % (**AnimCurs).count;}/*** *	AnimateProgCurs - Animate a cursor list one frame to show progress * *	Coding Notes: *	#A# - If the high bit of the frame field is set, AnimCurs is a list of color cursors. *	#B# - The calculations do this mapping: * *             Val *              | *	Min----------------------------------------------Max * *	Which maps to: * *           ValLevel *              | *	0.0----------------------------------------------1.0 * *	Which maps to: * *           CursLevel *              | *	0.0----------------------------------------------(**AnimCurs).count in Fixed format * *	Which maps to: * *           CursInd *              | *	0-----------------------------------------------(**AnimCurs).count * ***/pascal void	AnimateProgCurs(AnimCurs, Min, Max, Val)AnimCursHnd	AnimCurs;	// => Animated cursor list to animate >>short		Min;		// Minimum value of Val >>short		Max;		// Maximum value of Val >>short		Val;		// Value to set cursor by >>{Fixed		ValLevel;	// If Val=Min then ValLevel=0.0, If Val=Max then ValLevel=1.0Fixed		CursLevel;	// If ValLevel=0.0 then CursLevel=0.0, If ValLevel=1.0 then CursLevel=countshort		CursInd;	// Truncated value of CursLevelCursHandle	TheCursor;	// => Cursor to display	Max -= Min;			// #B#	Val -= Min;	ValLevel = FixRatio (Val, Max);	CursLevel = ((**AnimCurs).count - 1) << 16;	CursLevel = FixMul (CursLevel, ValLevel);	CursInd = CursLevel >> 16;	if ((TheCursor = (**AnimCurs).cursors[CursInd]) != (CursHandle)nil)		if ((**AnimCurs).frame & 0x8000)	// #A#			SetCCursor ((CCrsrHandle) TheCursor);		else {			HLock ((Handle) TheCursor);			SetCursor (*TheCursor);			HUnlock ((Handle) TheCursor);		}}/*** *	DisposeAnimCurs - Dispose of a cursor list and its cursors * *	Coding Notes: *	#A# - If the high bit of the frame field is set, AnimCurs is a list of color cursors. *	#B# - CURS resources aren�t actually disposed of, they�re just made purgeable. ***/pascal void		DisposeAnimCurs(AnimCursHnd AnimCurs){short		Index;AnimCursPtr	p = *AnimCurs;	HLock(AnimCurs);	for(Index = 0;Index<p->count;Index++)		if (p->cursors)			if (p->frame & 0x8000)					// #A#				DisposCCursor((CCrsrHandle)p->cursors[Index]);			else				HPurge((Handle)p->cursors[Index]);	// #B#	HUnlock(AnimCurs);	ReleaseResource((Handle)AnimCurs);}