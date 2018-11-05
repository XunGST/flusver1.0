#include "morpherodla.h"
#include "TiffDataRead.h"
#include "TiffDataWrite.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <time.h>
#include <cmath>
#include <omp.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

using namespace std;

MorphEroDla::MorphEroDla(QObject *parent)
	: QObject(parent)
{

}

MorphEroDla::~MorphEroDla()
{

}

void MorphEroDla::runMED()
{
	ifstream fileu("./FilesGenerate/ParameterFile.tmp");
	if (!fileu)
	{
		cout<<"read file 'ParameterFile.tmp' error!!!";
	}

	string sFilPath;
	vector<string> vsFilePath;
	while (getline(fileu,sFilPath)) 
	{
		vsFilePath.push_back(sFilPath);
	}

	string input_path=vsFilePath[0];
	string output_path=vsFilePath[1];
	string sWindowSize=vsFilePath[2];
	int iWindowSize;
	iWindowSize =atoi(sWindowSize.c_str());

	cout<<"input: "<<input_path<<endl;
	cout<<"output: "<<output_path<<endl;
	cout<<"Window Size: "<<iWindowSize<<endl;

	fileu.close();

	readImg2char( QString::fromStdString(input_path));
	MorphologicalErosionDilation(iWindowSize);
	saveImg( QString::fromStdString(output_path));
	imgList[0]->close();
}


void MorphEroDla::readImg2char(QString filename)
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	// <新建IO类>
	TiffDataRead* pread = new TiffDataRead;

	imgList.push_back(pread);

	std::string stdfilenamestr=filename.trimmed().toStdString(); // 不分开居然读不进来，醉了

	// <提取数据>
	if (!imgList.at(imgList.length()-1)->loadFrom(stdfilenamestr.c_str()))
	{
		qDebug()<<"load error!"<<endl;
		return;
	}
	else
	{
		imgList.at(imgList.length()-1)->convert2uchar();
		std::cout<<"convert success!"<<std::endl;
	}

	return;
}

int** MorphEroDla:: ScanWindow2(int sizeWindows,int& numWindows)
{
	//defining a two-dimensions window
	numWindows=sizeWindows*sizeWindows-1-4;
	int i,k,f;
	int** direction=new int*[numWindows];
	for(i=0;i<numWindows;i++)
	{
		direction[i]=new int[2];
	}
	//loop
	i=0;
	for (k=-(sizeWindows-1)/2;k<=(sizeWindows-1)/2;k++)
	{
		for (f=-(sizeWindows-1)/2;f<=(sizeWindows-1)/2;f++)
		{
			if ((!(k==0&&f==0))&&(abs(k)*abs(f)!=((sizeWindows-1)/2)*((sizeWindows-1)/2)))
			{
				direction[i][0]=k;
				direction[i][1]=f;
				i++;
			}
		}
	}


	for(i=0;i<numWindows;i++)
	{
		cout<<direction[i][0]<<","<<direction[i][1]<<endl;
	}


	return direction;
}


void MorphEroDla::saveImg(QString filename)
{
	//projection
	double coorval[6];
	imgList[0]->poDataset()->GetGeoTransform(coorval);
	cout<<imgList[0]->poDataset()->GetProjectionRef()<<endl;
	//data type
	GDALDataType dataType =imgList[0]->poDataset()->GetRasterBand(1)->GetRasterDataType();
	double nodata;
	nodata=imgList[0]->poDataset()->GetRasterBand(1)->GetNoDataValue();

	/// <生成tiff文件>
	TiffDataWrite pwrite;
	bool brlt = pwrite.init(filename.trimmed().toStdString().c_str(), imgList.at(0)->rows(), imgList.at(0)->cols(), imgList[0]->bandnum(), \
		imgList.at(0)->geotransform(), imgList.at(0)->projectionRef(), dataType, nodata);
	if (!brlt)
	{
		cout<<"write init error!"<<endl;
		return ;
	}
	unsigned char _val = 0;
	//#pragma omp parallel for private(j, k, _val), num_threads(omp_get_max_threads())
	if (imgList[0]->imgData()!=NULL)
	{
		for (size_t i=0; i<pwrite.rows(); i++)
		{
			for (size_t j=0; j<pwrite.cols(); j++)
			{
				for (size_t k=0; k<pwrite.bandnum(); k++)
				{
					size_t datalen;

					datalen=k*pwrite.rows()*pwrite.cols()+i*pwrite.cols()+j;

					_val = imgList[0]->imgData()[datalen];

					pwrite.write(i, j, k, &_val);
				}
			}
		}
		cout<<"write success!"<<endl;
		pwrite.close();
	}
}


void MorphEroDla::MorphologicalErosionDilation(int iSizeWin)
{
	//basic information
	cout<<imgList[0]->poDataset()->GetRasterYSize()<<endl;
	int rownum = imgList[0]->poDataset()->GetRasterYSize();
	cout<<imgList[0]->poDataset()->GetRasterXSize()<<endl;
	int colnum = imgList[0]->poDataset()->GetRasterXSize();
	cout<<imgList[0]->poDataset()->GetRasterCount()<<endl;
	int bandnum =imgList[0]->poDataset()->GetRasterCount();
	//projection
	double coorval[6];
	imgList[0]->poDataset()->GetGeoTransform(coorval);
	for(int i=0; i<6; i++)
	{
		cout<<coorval[i]<<endl;
	}
	cout<<imgList[0]->poDataset()->GetProjectionRef()<<endl;
	//data type
	cout<<GDALGetDataTypeName(imgList[0]->poDataset()->GetRasterBand(1)->GetRasterDataType())<<endl;
	GDALDataType datatype =imgList[0]->poDataset()->GetRasterBand(1)->GetRasterDataType();

	double nodata;
	nodata=imgList[0]->poDataset()->GetRasterBand(1)->GetNoDataValue();
	//

	unsigned char* ucTmpImg= new unsigned char[rownum*colnum*bandnum];

	int trytimes=4;// 迭代次数
	int numWindows;
	int** direction=ScanWindow2(iSizeWin,numWindows);

	for (int kk=0;kk<trytimes;kk++)
	{
		//#pragma omp parallel private(direction)
		//#pragma omp for schedule(dynamic)
		for (int ii=0;ii<rownum;ii++)
		{
			for (int jj=0;jj<colnum;jj++)
			{
				
				if (imgList[0]->imgData()[ii*colnum+jj]!=nodata)
				{
					//-----------------------------------膨胀----------------------------------------//
					if (kk==0||kk==3) 
					{
						// 点到目标值=2时
						if (imgList[0]->imgData()[ii*colnum+jj]==2)
						{
							//开始处理周围
							for (int m =0; m<numWindows; m++)
							{
								int _x = ii+direction[m][0];
								int _y = jj+direction[m][1];
								if (_x<0 || _y<0 || _x>=rownum || _y>=colnum)
								{
									continue;
								}
								//switch方法
								if (imgList[0]->imgData()[_x*colnum+_y]!=nodata)
								{
									ucTmpImg[_x*colnum+_y]=2;
								}
							}
						}
						// 不是目标值，不处理
						if (ucTmpImg[ii*colnum+jj]!=2)
						{
							ucTmpImg[ii*colnum+jj]=1;
						}
					}
					//-----------------------------------膨胀----------------------------------------//


					//-----------------------------------腐蚀----------------------------------------//
					if (kk==1||kk==2) // 腐蚀
					{
						// 不是目标值，不处理
						if (imgList[0]->imgData()[ii*colnum+jj]==1)
						{
							ucTmpImg[ii*colnum+jj]=1;
						}
						else
						{
							ucTmpImg[ii*colnum+jj]=2;
						}

						// 点到目标值=2时
						if (imgList[0]->imgData()[ii*colnum+jj]==2)
						{
							int val2;
							val2=0;
							//开始处理周围
							for (int m =0; m<numWindows; m++)
							{
								int _x = ii+direction[m][0];
								int _y = jj+direction[m][1];
								if (_x<0 || _y<0 || _x>=rownum || _y>=colnum)
								{
									continue;
								}
								//switch方法

								if (imgList[0]->imgData()[_x*colnum+_y]==1)
								{
									ucTmpImg[ii*colnum+jj]=1;
									break;
								}
							}
						}
					}
					//-----------------------------------腐蚀----------------------------------------//
				}
				else
				{
					ucTmpImg[ii*colnum+jj]=nodata;
				}
			}
		}
		//#pragma omp parallel 
		//#pragma omp for schedule(dynamic)
		for (int ii=0;ii<rownum;ii++)
		{
			for (int jj=0;jj<colnum;jj++)
			{
				imgList[0]->imgData()[ii*colnum+jj]=ucTmpImg[ii*colnum+jj];
			}
		}
	}
	for(int i=0;i<numWindows;i++)
	{
		delete[]direction[i]; 
		direction[i]=NULL;
	}
	delete []direction;
	direction=NULL;
	delete []ucTmpImg;
	ucTmpImg=NULL;


	return ;
}