//
//	MODULE:		TextDocument.cpp
//
//	PURPOSE:	Basic implementation of a text data-sequence class
//
//	NOTES:		www.catch22.net
//
#include <windows.h>
#include "TextDocument.h"

//
//	TextDocument constructor
//
TextDocument::TextDocument()
{
	buffer     = 0;
	length     = 0;
	linebuffer = 0;
	numlines   = 0;
}

//
//	TextDocument destructor
//
TextDocument::~TextDocument()
{
	clear();
}

//
//	Initialize the TextDocument with the specified file
//
bool TextDocument::init(char *filename)
{
	HANDLE hFile;
	
	hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	return init(hFile);
}

//
//	Initialize using a file-handle
//
bool TextDocument::init(HANDLE hFile)
{
	ULONG numread;

	if((length = GetFileSize(hFile, 0)) == 0)
		return false;

	// allocate new file-buffer
	if((buffer = new char[length]) == 0)
		return false;

	// read entire file into memory
	ReadFile(hFile, buffer, length, &numread, 0);

	// work out where each line of text starts
	if(!init_linebuffer())
		clear();

	CloseHandle(hFile);
	return true;
}

//
//	Initialize the line-buffer, so we know where every
//  line starts within the file
//
bool TextDocument::init_linebuffer()
{
	ULONG i         = 0;
	ULONG linestart = 0;

	// allocate the line-buffer
	if((linebuffer  = new ULONG[length]) == 0)
		return false;

	numlines = 0;

	// loop through every byte in the file
	for(i = 0; i < length; )
	{
		if(buffer[i++] == '\r')
		{
			// carriage-return / line-feed combination
			if(buffer[i] == '\n')
				i++;

			// record where the line starts
			linebuffer[numlines++] = linestart;
			linestart = i;
		}
	}

	linebuffer[numlines] = length;

	return true;
}

//
//	Empty the data-TextDocument
//
bool TextDocument::clear()
{
	if(buffer)
	{
		delete buffer;
		buffer = 0;
		length = 0;
	}

	if(linebuffer)
	{
		delete linebuffer;
		linebuffer  = 0;
		numlines = 0;
	}

	return true;
}

//
//	Retrieve the specified line of text and store it in "buf"
//
ULONG TextDocument::getline(ULONG lineno, char *buf, size_t len)
{
	char *lineptr;
	ULONG linelen;

	if(lineno >= numlines || buffer == 0 || length == 0)
		return 0;

	// find the start of the specified line
	lineptr = buffer + linebuffer[lineno];

	// work out how long it is, by looking at the next line's starting point
	linelen = linebuffer[lineno+1] - linebuffer[lineno];
	
	// make sure we don't overflow caller's buffer
	linelen = min(len, linelen);

	memcpy(buf, lineptr, linelen);
	
	return linelen;
}

//
//	Return the number of lines
//
ULONG TextDocument::linecount()
{
	return numlines;
}

//
//	Return the length of longest line
//
ULONG TextDocument::longestline(int tabwidth)
{
	ULONG i;
	ULONG longest = 0;
	ULONG xpos = 0;

	for(i = 0; i < length; i++)
	{
		if(buffer[i] == '\r')
		{
			if(buffer[i+1] == '\n')
				 i++;

			longest = max(longest, xpos);
			xpos = 0;
		}
		else if(buffer[i] == '\t')
		{
			xpos += tabwidth - (xpos % tabwidth);
		}
		else
		{
			xpos ++;
		}
	}

	longest = max(longest, xpos);
	return longest;
}