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

	void write(int _row, int _col, int _band, void* pVal);

public:
	void close();

public:
	GDALDriver* poDriver();
	GDALDataset* poDataset();
	int rows();
	int cols();
	int bandnum();
	long datalength();
	double invalidValue();
	unsigned char* imgData();
	GDALDataType datatype();
	double* geotransform();
	char* projectionRef();
	int perPixelSize();


protected:
	template<class TT> bool createData();

protected:
	GDALDriver* mpoDriver;		//can not release this, maybe cause some memory error!
	GDALDataset* mpoDataset;	//=>
	int mnRows;					//
	int mnCols;					//
	int mnBands;				//
	unsigned char* mpData;		//=>
	GDALDataType mgDataType;	//
	long mnDatalength;			//=>
	double mpGeoTransform[6];	//
	char msProjectionRef[2048];	//
	char msFilename[2048];		//
	double mdInvalidValue;		//
	int mnPerPixSize;			//=>
};

#endif