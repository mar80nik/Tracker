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
	size_t ChunkSize; int flush, have, level, status;
	BYTE *in, *out;
	CodecCallback codec;

	Compressor(Compressor::modes _mode, int _level = -1, size_t _ChunkSize = 16384);
	~Compressor();
protected:
	void Read(CFile *src);
	void Write(CFile *dst);
	int ZipUnzip(CFile *src, CFile *dst, CodecCallback codec);
	int Zip();
	int UnZip();
	BOOL IsZipCompleted() const;
	BOOL IsReadCompleted() const;
public:
	int Process(CFile *src, CFile *dst);	
};

int zip(CFile *src, CFile *dst, const int level);
int unzip(CFile *src, CFile *dst);