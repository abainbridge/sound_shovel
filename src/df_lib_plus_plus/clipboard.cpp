// Own header
#include "clipboard.h"

// Deadfrog lib headers
#include "df_common.h"

// Platform headers
#include <windows.h>


Clipboard g_clipboard;


static int s_types[]{ CF_TEXT, CF_WAVE };


void *Clipboard::GetData(int type)
{
    DebugAssert(type < TYPE_NUM_TYPES);

    if (!OpenClipboard(NULL))
        return NULL;

    HANDLE clipboardData = GetClipboardData(s_types[type]);
    if (!clipboardData)
        return NULL;

    // Return a pointer to the data associated with the handle returned from 
    // GetClipboardData.
    return GlobalLock(clipboardData);
}


void Clipboard::ReleaseData(void *data)
{
	// Unlock the global memory.
	GlobalUnlock(data);

	// Close the Clipboard, which unlocks it so that other applications can 
    // examine or modify its contents.
	CloseClipboard();
}


void Clipboard::SetData(int type, void *data, int dataSize)
{
    if (type == TYPE_WAV)
        return; // TODO.

    if (!OpenClipboard(NULL))
        return;

    // Empty the Clipboard. This also has the effect of allowing Windows to 
    // free the memory associated with any data that is in the Clipboard.
	EmptyClipboard();

	HGLOBAL clipboardHandle = GlobalAlloc(GMEM_DDESHARE, dataSize);

	// Calling GlobalLock returns a pointer to the data associated with the 
    // handle returned from GlobalAlloc()
	void *windowsData = GlobalLock(clipboardHandle);
    if (!windowsData)
        return;

	// Copy the data from the local variable to the global memory.
	memcpy(windowsData, data, dataSize);
			  
	// Unlock the memory - don't call GlobalFree because Windows will free the
	// memory automatically when EmptyClipboard is next called. 
	GlobalUnlock(clipboardHandle);
			  
	// Set the Clipboard data by specifying that ANSI text is being used and 
    // passing the handle to the global memory.
	SetClipboardData(s_types[type], clipboardHandle);
			  
	// Close the Clipboard which unlocks it so that other applications can 
    // examine or modify its contents.
	CloseClipboard();
}
