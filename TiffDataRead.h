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
	bool loadInfo(const char* _filename); 
	bool loadData(); 
	unsigned char* read(int _row, int _col, int _band);
	unsigned char* readL(int _row, int _col, int _band);

public:
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
	char* getFileName();

	void deleteImgData();

protected:
	template<class TT> bool readData();

protected: 
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
	double mdInvalidValue;
	int mnPerPixSize;			//=>

};

#endif