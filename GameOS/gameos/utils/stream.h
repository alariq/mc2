/*!
	@file		stream.h
	@brief		Declaration of stream class
	@created	October 16, 2006
	@author		Author's name

	@copyright &copy; Magik Anvil 2006

	$Id$
*/
#pragma once

#include<sys/stat.h>
#include<sys/types.h>

//*****************************************
/// stream
//*****************************************
class stream
{
public:
	enum SEEK_TYPE { S_SET, S_CUR, S_END };
	stream(void) {};
	virtual ~stream(void) {};
	virtual int open(const char *, const char *) = 0;
	virtual int close() = 0;
	virtual size_t write(const void* pbuf, size_t size) = 0;
	virtual size_t write(const void* pbuf, size_t size, size_t count) = 0;
	virtual size_t write_byte(char b) = 0;
	virtual size_t write_word(short sh) = 0;
	virtual size_t write_dword(unsigned int i) = 0;
	virtual size_t write_float(float f) = 0;
	virtual size_t write_cstr(const char* str) = 0;

	virtual size_t read(void* pbuf, size_t size) const = 0;
	virtual size_t read(void* pbuf, size_t size, size_t count) const = 0;
	virtual char read_byte() = 0;
	virtual short read_word() = 0;
	virtual unsigned int read_dword() const = 0;
	virtual float read_float() const = 0;
	virtual size_t read_cstr(char* ) const = 0;

	virtual int seek(int bytes, SEEK_TYPE type) = 0;
    virtual int tell() = 0;
	virtual const char* get_path() const = 0;
    //virtual int get_fstat(struct _stat* pstat) = 0;

	static stream* makeFileStream();
	static void releaseStream(stream* pstream);
};
