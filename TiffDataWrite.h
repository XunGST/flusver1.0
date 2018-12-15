/************************************************************************/
/* This class is wrote for supercomputer to write image use parallel.
/* Not support Block read & write
/* But support multi-thread
/* Author: Y. Yao
/* E-mail: whuyao@foxmail.com
/* Version: v1.0
/************************************************************************/

#ifndef TIFDATAWRITE_H
#define TIFDATAWRITE_H

#include "gdal_priv.h"
#include "ogr_core.h"
#include "ogr_spatialref.h"
#include "TiffDataRead.h"

class TiffDataWrite
{
public:
	TiffDataWrite(void);
	~TiffDataWrite(void);
public:
	bool init(const char* _filename, int _rows, int _cols, int _bandnum,\
		double _pGeoTransform[6], const char* _sProjectionRef, \
		GDALDataType _datatype = GDT_Byte, \
		double _dInvalidVal = 0.0f);

	bool init(const char* _filename, TiffDataRead* pRead);

	bool init(const char* _filename, TiffDataRead* pRead, int bandnum, \
		GDALDataType _datatype = GDT_Byte, \
		double _dInvalidVal = 0.0f);

	void write(size_t _row, size_t _col, size_t _band, void* pVal);

public:
	void close();

public:
	GDALDriver* poDriver();
	GDALDataset* poDataset();
	size_t rows();
	size_t cols();
	size_t bandnum();
	size_t datalength();
	double invalidValue();
	unsigned char* imgData();
	GDALDataType datatype();
	double* geotransform();
	char* projectionRef();
	size_t perPixelSize();


protected:
	template<class TT> bool createData();

protected:
	GDALDriver* mpoDriver;		//can not release this, maybe cause some memory error!
	GDALDataset* mpoDataset;	//=>
	size_t mnRows;					//
	size_t mnCols;					//
	size_t mnBands;				//
	unsigned char* mpData;		//=>
	GDALDataType mgDataType;	//
	size_t mnDatalength;			//=>
	double mpGeoTransform[6];	//
	char msProjectionRef[2048];	//
	char msFilename[2048];		//
	double mdInvalidValue;		//
	size_t mnPerPixSize;			//=>
};

#endif