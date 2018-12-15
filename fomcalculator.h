#ifndef FOMCALCULATOR_H
#define FOMCALCULATOR_H

#include "kappacalculate.h"
#include "TiffDataRead.h"
#include <QObject>

class TiffDataRead;

class FoMcalculator : public QObject
{
	Q_OBJECT

public:
	FoMcalculator(QObject *parent);
	FoMcalculator(QObject *parent,vector<fomReader>& _image,int _mode);
	void calculateFoMprocess();

	~FoMcalculator();

private:

	bool reRoadImages();
	bool measureFoM();
	bool measureKappaChangePart();
	template<class TT> unsigned char convert(TiffDataRead* tImage,int ii);
	unsigned char convert2uchar(TiffDataRead* tImage,int ii);

private:
	vector<fomReader> imageSerial;
	vector<unsigned char*> convertImage;
	size_t itrows;
	size_t itcols;
	double nodata;
	double minmum;
	double maxmum;

	int mode;
	
};

#endif // FOMCALCULATOR_H
