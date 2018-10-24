//	cout << "/************************************************************************/" << endl;
//	cout << "/* Author: X. Liang" << endl;
//	cout << "/* E-mail: liangxunnice@163.com" << endl;
//	cout << "/* Version: v2.12" << endl;
//	cout << "/************************************************************************/" << endl;

#include "markovplayer.h"
#include "TiffDataRead.h"
#include "TiffDataWrite.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

markovPlayer::markovPlayer(QObject *parent)
	: QObject(parent)
{

}

markovPlayer::markovPlayer(QVector<TiffDataRead*> _start,QVector<TiffDataRead*> _end, int _startyear,int _endyear, int _predictyear)
{
	startImg=_start;
	endImg=_end;
	startyear=_startyear;
	endyear=_endyear;
	predictyear=_predictyear;

}

markovPlayer::~markovPlayer()
{

}

void markovPlayer::runPredict()
{
	checkInput();

	getConvMatrix();

	markovRun();
}

void markovPlayer::checkInput()
{
	if (startImg.size()>1&&endImg.size()>1)
	{
		startImg[1]->close();
		endImg[1]->close();
		startImg.remove(0);
		endImg.remove(0);
	}

	nodataS=startImg[0]->poDataset()->GetRasterBand(1)->GetNoDataValue();
	nodataE=endImg[0]->poDataset()->GetRasterBand(1)->GetNoDataValue();

	//maxmum=startImg[0]->poDataset()->GetRasterBand(1)->GetMaximum();
	//minmum=endImg[0]->poDataset()->GetRasterBand(1)->GetMinimum(); // ²»×¼

	double minmax[2];
	startImg[0]->poDataset()->GetRasterBand(1)->ComputeRasterMinMax(false,minmax);
	minmum=minmax[0];
	maxmum=minmax[1];


	iSrows=startImg[0]->rows();
	iScols=startImg[0]->cols();

	iErows=endImg[0]->rows();
	iEcols=endImg[0]->cols();

	if (iSrows!=iErows||iScols!=iEcols||nodataS==maxmum||nodataS==minmum)
	{
		qDebug()<<"false";
		return;
	}

	uImgStart=new unsigned char[iSrows*iScols];
	uImgEnd=new unsigned char[iEcols*iErows];

	size_t ii;
	for (ii=0;ii<iSrows*iScols;ii++)
	{
		uImgStart[ii]=convert2uchar(startImg,ii);
		uImgEnd[ii]=convert2uchar(endImg,ii);
	}

}


unsigned char markovPlayer::convert2uchar(QVector<TiffDataRead*> vImage,int ii)
{
	unsigned char uRestrictValue;

	switch(vImage.at(0)->datatype())
	{
	case GDT_Byte:
		uRestrictValue = convert<unsigned char>(vImage,ii);
		break;
	case GDT_UInt16:
		uRestrictValue = convert<unsigned short>(vImage,ii);
		break;
	case GDT_Int16:
		uRestrictValue = convert<short>(vImage,ii);
		break;
	case GDT_UInt32:
		uRestrictValue = convert<unsigned int>(vImage,ii);
		break;
	case GDT_Int32:
		uRestrictValue = convert<int>(vImage,ii);
		break;
	case GDT_Float32:
		uRestrictValue = convert<float>(vImage,ii);
		break;
	case GDT_Float64:
		uRestrictValue = convert<double>(vImage,ii);
		break;
	default:
		return 0;
	}

	return uRestrictValue;
}

template<class TT>
unsigned char markovPlayer::convert(QVector<TiffDataRead*> vImage,int ii)
{
	TT temp=*(TT*)(vImage[0]->imgData()+ii*sizeof(TT));

	if (temp==vImage[0]->poDataset()->GetRasterBand(1)->GetNoDataValue())
	{
		return (unsigned char)0;
	}

	return (unsigned char)temp;
}


void markovPlayer::getConvMatrix()
{

	if (vExitValue.size()>0)
	{
		vExitValue.clear();
	}

	for (int ii=minmum;ii<=maxmum;ii++)
	{
		vExitValue.push_back(ii);
	}


	pixelstatistic=new double* [vExitValue.size()];
	for (int ii=0; ii<vExitValue.size(); ii++)
	{
		pixelstatistic[ii] = new double[vExitValue.size()];
	}

	for (int ii=0;ii<vExitValue.size(); ii++)
	{
		for (int jj=0;jj<vExitValue.size(); jj++)
		{
			pixelstatistic[ii][jj]=0;
		}
	}

	_sumValue=0;

	for (int ii=0;ii<iScols*iSrows;ii++)
	{
		if (uImgStart[ii]!=0)
		{
			if (uImgStart[ii]==uImgEnd[ii]) // ||uImgEnd[ii]==0
			{
				pixelstatistic[uImgStart[ii]-1][uImgStart[ii]-1]++;
			}
			else
			{
				pixelstatistic[uImgStart[ii]-1][uImgEnd[ii]-1]++;
			}
			_sumValue++;
		}
	}
}


void markovPlayer::markovRun()
{
	QFile file("./FilesGenerate/MakovChain.csv");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);

	out<<"[Conversion Matrix]"<<endl;

	QString head(tr("Land use types"));
	for(int ii=0;ii<vExitValue.size();ii++)
	{
		head+=",type"+QString::number(ii+1);
	}

	out<<head+",total"<<endl;


	for(int ii=0;ii<vExitValue.size();ii++)
	{
		QString lineheader="type";
		lineheader+=QString::number(ii+1);

		vSumStart.push_back(0);
		vSumEnd.push_back(0);

		for (int jj=0;jj<vExitValue.size();jj++)
		{
			QString str = QString("%1").arg(pixelstatistic[ii][jj], 0, 'f', 0);
			lineheader+=","+str;
			vSumStart[ii]+=pixelstatistic[ii][jj];
			vSumEnd[ii]+=pixelstatistic[jj][ii];
		}

		QString str = QString("%1").arg(vSumStart[ii], 0, 'f', 0);
		lineheader+=","+str;

		out<<lineheader<<endl;
	}

	QString linetotal="total";

	double sum=0;
	for (int jj=0;jj<vExitValue.size();jj++)
	{
		QString str = QString("%1").arg(vSumEnd[jj], 0, 'f', 0);
		linetotal+=","+str;
	}
	out<<linetotal<<endl;


	//=======================================================

	out<<"[Conversion Probability]"<<endl;

	QString head1(tr("Land use types"));
	for(int ii=0;ii<vExitValue.size();ii++)
	{
		head1+=",type"+QString::number(ii+1);
	}


	for(int ii=0;ii<vExitValue.size();ii++)
	{
		QString lineheader="type";
		lineheader+=QString::number(ii+1);

		vSumStart.push_back(0);
		vSumEnd.push_back(0);

		for (int jj=0;jj<vExitValue.size();jj++)
		{
			pixelstatistic[ii][jj]=pixelstatistic[ii][jj]/(double)vSumStart.at(ii);
			QString str = QString("%1").arg(pixelstatistic[ii][jj], 0, 'f', 6);
			lineheader+=","+str;
		}
		out<<lineheader<<endl;
	}

	//=======================================================

	out<<"[Predict amount]"<<endl;

	QString head2(tr("year"));
	for(int ii=0;ii<vExitValue.size();ii++)
	{
		head2+=",type"+QString::number(ii+1);
	}
	out<<head2<<endl;

	int lenth=vExitValue.size();
	int times=(predictyear-endyear)/(endyear-startyear);
	QVector<double> vSumProb;
	QVector<double> vSumTmp;
	double _sum=0;
	for(int ii=0;ii<vExitValue.size();ii++)
	{
		 //vSumProb.push_back(vSumEnd.at(ii));
		vSumProb.push_back(vSumStart.at(ii));
		 _sum+=vSumProb.at(ii);
	}

	for(int ii=0;ii<vExitValue.size();ii++)
	{
		//vSumProb[ii]=vSumEnd.at(ii)/_sum;
		vSumProb[ii]=vSumStart.at(ii)/_sum;
		vSumTmp.push_back(0);
	}


	for(int kk=0;kk<=times;kk++)
	{

		for (int jj =0;jj<lenth;jj++)
		{
			double _sum=0;
			for (int zz =0;zz<lenth;zz++)
			{
				double tmp1;
				tmp1=vSumProb[zz];
				double tmp2;
				tmp2=pixelstatistic[zz][jj];

				_sum+=tmp1*tmp2;
			}
			vSumTmp[jj]=_sum;
		}

		for (int ii =0;ii<lenth;ii++)
		{
			vSumProb[ii]=vSumTmp[ii];
		}

	
		//QString lineheader1=QString::number(endyear+(kk+1)*(endyear-startyear));
		QString lineheader1=QString::number(endyear+(kk)*(endyear-startyear));
		for (int kk =0;kk<lenth;kk++)
		{
			QString str = QString("%1").arg(vSumTmp[kk]*_sumValue, 0, 'f', 0);
			lineheader1+=","+str;
		}
		out<<lineheader1<<endl;
	}

}






