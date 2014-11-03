#pragma once

#include "targetver.h"
#include <afx.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <atlstr.h>
#include "zlib.h"
#include "MyTime.h"

struct Compressor: public z_stream
{
	enum modes {ZIP, UNZIP} mode; MyTimer Timer;
	typedef int (Compressor::*CodecCallback)();
	struct
	{
		sec dt;
		float ratio;
	} LastSession;
protected:
	size_t ChunkSize; int flush, have, level, status;
	BYTE *in, *out;	
public:
	Compressor(Compressor::modes _mode, int _level = -1, size_t _ChunkSize = 16384);
	~Compressor();
protected:
	int ZipUnzip(CFile *src, CFile *dst, CodecCallback codec);
	int Zip();
	int UnZip();
public:
	int Process(CFile *src, CFile *dst);	
};
