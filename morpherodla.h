#ifndef MORPHERODLA_H
#define MORPHERODLA_H

#include <QObject>

class TiffDataRead;

class MorphEroDla : public QObject
{
	Q_OBJECT

public:
	MorphEroDla(QObject *parent);
	~MorphEroDla();
	void runMED();

private:
	void readImg2char(QString filename);
	void MorphologicalErosionDilation(int iSizeWin);
	int** ScanWindow2(int sizeWindows,int& numWindows);
	void saveImg(QString filename);

private:
	QList<TiffDataRead*> imgList;
	
};

#endif // MORPHERODLA_H
