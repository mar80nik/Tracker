#include "compressor.h"

int Compressor::ZipUnzip(CFile *src, CFile *dst, CodecCallback codec)
{
	int ret = Z_OK;
	do 
	{
		TRY
		{
			Read(src);
		}
		AND_CATCH(CFileException, pEx)
		{
			pEx->ReportError(); ret = Z_ERRNO; break;
		}
		END_CATCH
			do 
			{   
				ret = (this->*codec)();
				TRY 
				{
					Write(dst);
				}
				CATCH (CFileException, pEx)
				{
					pEx->ReportError(); ret = Z_ERRNO;
				}
				END_CATCH
			} 
			while (IsZipCompleted() == FALSE && ret == Z_OK);
	} 
	while (IsReadCompleted() == FALSE);
	return ret;
}

/////////////////////////////////////////////////////////////////////
Compressor::Compressor(modes _mode, int _level, size_t _ChunkSize):
	ChunkSize(_ChunkSize), in(NULL), out(NULL), flush(Z_NO_FLUSH), have(0), mode(_mode), level(_level),
	codec(NULL)
{
	in = (BYTE*)malloc(ChunkSize); out = (BYTE*)malloc(ChunkSize);

	zalloc = NULL; zfree = NULL; opaque = Z_NULL;
	avail_in = 0; avail_out = ChunkSize;
	next_in = in; next_out = out; Timer.Start();

	switch (mode)
	{
	case ZIP:
		status = deflateInit(this, level);
		codec = &Compressor::Zip;
		break;
	case UNZIP:
		status = inflateInit(this);
		codec = &Compressor::UnZip;
		break;
	}

}

Compressor::~Compressor()
{
	if (in != NULL) 
		free(in);
	if (out != NULL) 
		free(out);	
}

int Compressor::Process( CFile *src, CFile *dst )
{
	int ret = Z_OK; 
	if (status == Z_OK)
	{
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
	}
	Timer.Stop();
	return (ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR);
}

void Compressor::Read( CFile *src )
{
	avail_in = src->Read(in, ChunkSize); next_in = in;	
	flush = (avail_in != ChunkSize ? Z_FINISH : Z_NO_FLUSH);
}

int Compressor::Zip()
{
	avail_out = ChunkSize;	next_out = out;	
	return deflate(this, flush);
}

int Compressor::UnZip()
{
	avail_out = ChunkSize;	next_out = out;
	return inflate(this, Z_NO_FLUSH);
}

void Compressor::Write( CFile *dst )
{
	have = ChunkSize - avail_out;
	dst->Write(out, have);
}

BOOL Compressor::IsZipCompleted() const
{
	return (avail_out != 0);
}

BOOL Compressor::IsReadCompleted() const
{
	return (flush == Z_FINISH);
}

