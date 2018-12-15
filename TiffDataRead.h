/************************************************************************/
/* This class is wrote for supercomputer to read image .
/* Not support Block read & write
/* Author: Tim
/* E-mail: 1245764691@qq.com
/* Version: v1.0
/************************************************************************/
#ifndef TIFDATAREAD_H
#define TIFDATAREAD_H
#include <gdal_priv.h>

class TiffDataRead
{
public:
	TiffDataRead(void);
	~TiffDataRead(void);

public:
	void close();
	bool isValid();

public:
	bool loadFrom(const char* _filename); 
	bool convert2uchar(); 
	bool loadInfo(const char* _filename); 
	bool loadData(); 
	unsigned char* read(size_t _row, size_t _col, size_t _band);
	unsigned char* readL(size_t _row, size_t _col, size_t _band);

public:
	GDALDataset* poDataset();
	size_t rows();
	size_t cols();
	size_t bandnum();
	long datalength();
	double invalidValue();
	unsigned char* imgData();
	GDALDataType datatype();
	double* geotransform();
	char* projectionRef();
	size_t perPixelSize();
	char* getFileName();

	void deleteImgData();

protected:
	template<class TT> bool readData();
	template<class TT> bool convertData();

protected: 
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
	double mdInvalidValue;
	size_t mnPerPixSize;			//=>

};

#endif