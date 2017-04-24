#pragma once


class Clipboard
{
public:
    enum
    {
        TYPE_TEXT,
        TYPE_WAV,
        TYPE_NUM_TYPES
    };
	
    void *GetData(int type);
    void ReleaseData(void *data);

	void SetData(int type, void *data, int dataSize);
};


extern Clipboard g_clipboard;
