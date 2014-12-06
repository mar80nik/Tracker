#include "stdafx.h"
#include "compressor.h"

int Compressor::ZipUnzip(CFile *src, CFile *dst, CodecCallback codec)
{
	int ret = Z_OK;
	do 
	{
		TRY
		{
			avail_in = src->Read(in, ChunkSize); next_in = in;	
			flush = (avail_in != ChunkSize ? Z_FINISH : Z_NO_FLUSH);
		}
		AND_CATCH(CFileException, pEx)
		{
			pEx->ReportError(); ret = Z_ERRNO; break;
		}
		END_CATCH
			do 
			{   
				avail_out = ChunkSize;	next_out = out;	
				ret = (this->*codec)();
				TRY 
				{
					have = ChunkSize - avail_out;
					dst->Write(out, have);
				}
				CATCH (CFileException, pEx)
				{
					pEx->ReportError(); ret = Z_ERRNO;
				}
				END_CATCH
			} 
			while (avail_out == 0 && ret == Z_OK);
	} 
	while (flush != Z_FINISH);
	return ret;
}

/////////////////////////////////////////////////////////////////////
Compressor::Compressor(modes _mode, int _level, size_t _ChunkSize):
	ChunkSize(_ChunkSize), in(NULL), out(NULL), flush(Z_NO_FLUSH), have(0), mode(_mode), level(_level)
{
	in = (BYTE*)malloc(ChunkSize); out = (BYTE*)malloc(ChunkSize);	
}

Compressor::~Compressor()
{
	if (in != NULL)		free(in);
	if (out != NULL) 	free(out);	
}

int Compressor::Process( CFile *src, CFile *dst )
{
	int ret = Z_OK; Timer.Start();
	zalloc = NULL; zfree = NULL; opaque = Z_NULL; total_in = total_in = 0; LastSession.ratio = 0;

	switch (mode)
	{
	case ZIP:
		if (deflateInit(this, level) == Z_OK)
		{
			ret = ZipUnzip(src, dst, &Compressor::Zip);
			(void)deflateEnd(this);
		}
		break;
	case UNZIP:
		if (inflateInit(this) == Z_OK)
		{
			ret = ZipUnzip(src, dst,  &Compressor::UnZip);
			(void)inflateEnd(this);
		}
		break;
	}

	Timer.Stop();
	LastSession.dt = Timer.GetValue(); 
	if (total_in != 0 && total_out != 0)
	{
		switch (mode)
		{
		case ZIP:	LastSession.ratio = (float)total_in/total_out; break;
		case UNZIP: LastSession.ratio = (float)total_out/total_in; break;
		}
	}
	return (ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR);
}

int Compressor::Zip()	{return deflate(this, flush);}
int Compressor::UnZip()	{return inflate(this, Z_NO_FLUSH);}


