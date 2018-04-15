#include "TiffDataWrite.h"
#include "ogrsf_frmts.h"
#include <iostream>
using namespace std;

TiffDataWrite::TiffDataWrite(void)
{
	mpoDriver = NULL;
	mpoDataset = NULL;
	mnRows = mnCols = mnBands = -1;
	mpData = NULL;
	mgDataType = GDT_Byte;
	mnDatalength = 0;
	memset(mpGeoTransform, 0, 6*sizeof(double));
	strcpy(msProjectionRef, "");
	strcpy(msFilename, "");
	mdInvalidValue = 0.0f;
	mnPerPixSize = 1;

}


TiffDataWrite::~TiffDataWrite(void)
{
	close();
}

void TiffDataWrite::close()
{
	//write into data
	if (mpoDataset!=NULL && mpData!=NULL)
	{
		mpoDataset->RasterIO(GF_Write, 0, 0, mnCols, mnRows, \
			mpData, mnCols, mnRows, mgDataType, mnBands, 0, 0, 0, 0);
		mpoDataset->FlushCache();
	}


	////release memory
	if (mpoDataset!=NULL)
	{
		GDALClose(mpoDataset);
		mpoDataset = NULL;
	}

	mnRows = mnCols = mnBands = -1;

	if (mpData!=NULL)
	{
		delete []mpData;
		mpData = NULL;
	}



	mgDataType = GDT_Byte;
	mnDatalength = 0;
	memset(mpGeoTransform, 0, 6*sizeof(double));
	strcpy(msProjectionRef, "");
	strcpy(msFilename, "");
	mdInvalidValue = 0.0f;
	mnPerPixSize = 1;
}

bool TiffDataWrite::init( const char* _filename, int _rows, int _cols, int _bandnum,double _pGeoTransform[6], const char* _sProjectionRef, GDALDataType _datatype /*= GDT_Byte*/, double _dInvalidVal /*= 0.0f*/ )
{
	close();

	//register
	if(GDALGetDriverCount() == 0)
	{
		GDALAllRegister();
		OGRRegisterAll();
		CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	}

	//load
	mpoDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (mpoDriver == NULL)
	{
		cout<<"CGDALWrite::init : Create poDriver Failed."<<endl;
		close();
		return false;
	}

	//
	strcpy(msFilename, _filename);
	mnRows = _rows;
	mnCols = _cols;
	mnBands = _bandnum;

	for (int i=0; i<6; i++)
		mpGeoTransform[i] = _pGeoTransform[i];

	strcpy(msProjectionRef, _sProjectionRef);
	mgDataType = _datatype;
	mdInvalidValue = _dInvalidVal;

	//create podataset
	char** papseMetadata = mpoDriver->GetMetadata();
	mpoDataset = mpoDriver->Create(msFilename, mnCols, mnRows, mnBands, mgDataType, papseMetadata);
	if (mpoDataset == NULL)
	{
		cout<<"CGDALWrite::init : Create poDataset Failed."<<endl;
		close();
		return false;		
	}

	//add projection and coordinate
	poDataset()->SetGeoTransform(mpGeoTransform);
	poDataset()->SetProjection(msProjectionRef);
	for (int i =0; i<mnBands; i++)
	{
		poDataset()->GetRasterBand(i+1)->SetNoDataValue(mdInvalidValue);
	}

	//create data
	bool bRlt = false;
	switch(mgDataType)
	{
	case GDT_Byte:
		bRlt = createData<unsigned char>();
		break;
	case GDT_UInt16:
		bRlt = createData<unsigned short>();
		break;
	case GDT_Int16:
		bRlt = createData<short>();
		break;
	case GDT_UInt32:
		bRlt = createData<unsigned int>();
		break;
	case GDT_Int32:
		bRlt = createData<int>();
		break;
	case GDT_Float32:
		bRlt = createData<float>();
		break;
	case GDT_Float64:
		bRlt = createData<double>();
		break;
	default:
		cout<<"CGDALWrite::init : unknown data type!"<<endl;
		close();
		return false;
	}

	if (bRlt == false)
	{
		cout<<"CGDALWrite::init : Create data error!"<<endl;
		close();
		return false;
	}

	return true;
}

bool TiffDataWrite::init( const char* _filename, TiffDataRead* pRead )
{
	if (pRead == NULL)
	{
		cout<<"CGDALWrite::init : CGDALRead Point is Null."<<endl;
		return false;
	}

	return init(_filename, pRead->rows(), pRead->cols(), pRead->bandnum(), \
		pRead->geotransform(), pRead->projectionRef(), pRead->datatype(), 
		pRead->invalidValue());
}

bool TiffDataWrite::init( const char* _filename, TiffDataRead* pRead, int bandnum,  GDALDataType _datatype /*= GDT_Byte*/, double _dInvalidVal /*= 0.0f*/ )
{
	if (pRead == NULL)
	{
		cout<<"CGDALWrite::init : CGDALRead Point is Null."<<endl;
		return false;
	}

	return init(_filename, pRead->rows(), pRead->cols(), bandnum, \
		pRead->geotransform(), pRead->projectionRef(), _datatype, 
		_dInvalidVal);
}

void TiffDataWrite::write( int _row, int _col, int _band, void* pVal )
{
	long nloc = (_band*mnRows*mnCols + _row*mnCols + _col)*mnPerPixSize;
	memcpy(mpData+nloc, pVal, mnPerPixSize);
}

template<class TT> bool TiffDataWrite::createData()
{
	if (mpoDataset == NULL)
		return false;

	if (mpData!=NULL)
		delete mpData;
	mpData = NULL;

	mnPerPixSize = sizeof(TT);
	mnDatalength = mnRows*mnCols*mnBands*mnPerPixSize;
	mpData = new unsigned char[mnDatalength];
	memset(mpData, 0, mnDatalength);
	return true;
}

GDALDriver* TiffDataWrite::poDriver()
{
	return mpoDriver;
}


GDALDataset* TiffDataWrite::poDataset()
{
	return mpoDataset;
}

int TiffDataWrite::rows()
{
	return mnRows;
}

int TiffDataWrite::cols()
{
	return mnCols;
}

int TiffDataWrite::bandnum()
{
	return mnBands;
}

long TiffDataWrite::datalength()
{
	return mnDatalength;
}

double TiffDataWrite::invalidValue()
{
	return mdInvalidValue;
}

unsigned char* TiffDataWrite::imgData()
{
	return mpData;
}

GDALDataType TiffDataWrite::datatype()
{
	return mgDataType;
}

double* TiffDataWrite::geotransform()
{
	return mpGeoTransform;
}

char* TiffDataWrite::projectionRef()
{
	return msProjectionRef;
}

int TiffDataWrite::perPixelSize()
{
	return mnPerPixSize;
}