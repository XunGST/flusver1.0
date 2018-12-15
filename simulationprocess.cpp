#include "simulationprocess.h"
#include "dynasimulation.h"
#include "TiffDataRead.h"
#include "geodpcasys.h"
#include "TiffDataWrite.h"
#include <QTableWidget>
#include <QMessageBox>
#include <iomanip>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <QtCore>
#include <Windows.h>
#include <QDebug>

//#define ONLYURBAN

using namespace std;

SimulationProcess::SimulationProcess(QObject* parent)
{
	goalNum=NULL;
	saveCount=NULL;
	mIminDis2goal=NULL;
	temp=NULL;
	val=NULL;
	mdNeiborhoodProbability=NULL;
	mdRoulette=NULL;
	probability=NULL;
	sProbability=NULL;
	normalProbability=NULL;
	mdNeighIntensity=NULL;
	finishedCode=0;
	isSave=false;
	

	if (!readImageData())
		qDebug() << "read image error"<<endl;

	_rows=imgList[0]->rows();
	_cols=imgList[0]->cols();
	isbreak=0;
	temp = new unsigned char[_cols*_rows];

	connect(this,SIGNAL(sendColsandRows(int,int,int)),parent,SLOT(getColsandRowsandshowDynamicOnUi(int,int,int)));
	connect(this,SIGNAL(sendparameter(QString)),parent,SLOT(getParameter(QString)));

}

SimulationProcess::~SimulationProcess()
{

}

///
/// <邻域生成窗口>
///
int** ScanWindow(int sizeWindows)
{
	//defining a two-dimensions window
	int _numWindows=sizeWindows*sizeWindows-1;
	int i,k,f;
	int** direction=new int*[_numWindows];
	for(i=0;i<_numWindows;i++)
	{
		direction[i]=new int[2];
	}
	//loop
	i=0;
	for (k=-(sizeWindows-1)/2;k<=(sizeWindows-1)/2;k++)
	{
		for (f=-(sizeWindows-1)/2;f<=(sizeWindows-1)/2;f++)
		{
			if (!(k==0&&f==0))
			{
				direction[i][0]=k;
				direction[i][1]=f;
				i++;
			}
		}
	}
	return direction;
}


struct forArray
{
	int mb1;
	int mb2;
};
bool sortbymb1(const forArray &v1,const forArray &v2)
{
	return v1.mb1<v2.mb1;
};
void myPushback(vector<forArray> &vecTest,const int &m1,const int &m2)
{
	forArray test;
	test.mb1=m1;
	test.mb2=m2;
	vecTest.push_back(test);
}

bool SimulationProcess::getUiparaMat()
{
	QFile file("./FilesGenerate/config_mp.log");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return "false";
	while (!file.atEnd()) 
	{

		QString str = file.readLine().trimmed();
		if (str==tr("[Number of types]").trimmed())
		{
			str=file.readLine();
			nType=str.toInt();

			/// <初始化一些中间变量>
			mdRoulette=new double[nType+1];
			mdNeiborhoodProbability=new double[nType];
			probability=new double[nType];
			mIminDis2goal=new int[nType];
			saveCount=new int[nType];
			val=new int[nType];
			goalNum=new int[nType];
			mdNeighIntensity=new double[nType];

			mdRoulette[0]=0;/// <单独赋值>
			for (int i=0;i<nType;i++)/// <循环赋值>
			{
				mdNeiborhoodProbability[i]=0;
				val[i]=0;
				mdRoulette[i+1]=0;
				probability[i]=0;
			}

			t_filecost=new double*[nType];
			for(int i=0;i<nType;i++)
			{
				t_filecost[i]=new double[nType];
			}
		}

		if (str==tr("[Future Pixels]").trimmed())
		{
			for (int ii=0;ii<nType;ii++)
			{
				str=file.readLine();

				QStringList strList = str.split(",", QString::SkipEmptyParts);

				for (int jj=0;jj<strList.length();jj++)
				{
					goalNum[ii]=strList[jj].toInt();
				}
			}
		}
		if (str==tr("[Cost Matrix]").trimmed())
		{

			for (int ii=0;ii<nType;ii++)
			{
				str=file.readLine();

				QStringList strList = str.split(",", QString::SkipEmptyParts);

				for (int jj=0;jj<strList.length();jj++)
				{
					t_filecost[ii][jj]=strList[jj].toDouble();
				}
			}
		}

		if (str==tr("[Intensity of neighborhood]").trimmed())
		{

			for (int ii=0;ii<nType;ii++)
			{

				str=file.readLine();

				QStringList strList = str.split(",", QString::SkipEmptyParts);

				for (int jj=0;jj<strList.length();jj++)
				{
					mdNeighIntensity[ii]=strList[jj].toDouble();
				}
			}
		}

		if (str==tr("[Maximum Number Of Iterations]").trimmed())
		{
			str=file.readLine();
			looptime=str.trimmed().toInt();/// <迭代次数>
		}
		if (str==tr("[Size of neighborhood]").trimmed())
		{
			str=file.readLine();
			sizeWindows=str.trimmed().toInt();			/// <参数>

			if (sizeWindows!=1)
			{
				numWindows=sizeWindows*sizeWindows-1;
			}
			else
			{
				numWindows=1;
			}
			//---------------------------define a scanning window--------------------------- 
			//
			if (sizeWindows!=1)
			{
				direction=ScanWindow(sizeWindows);
			}
			//
		}
		if (str==tr("[Accelerated factor]").trimmed())
		{
			str=file.readLine();
			degree=str.trimmed().toDouble();
		}
		if (str==tr("[Enclaves for land use type]").trimmed())
		{
			str=file.readLine();
		}
	}
	file.close();

	Colour=new short*[nType+1];
	for(int i=0;i<(nType+1);i++)
	{
		Colour[i]=new short[3];
	}

	QFile filec("./FilesGenerate/config_color.log");
	if (!filec.open(QIODevice::ReadOnly | QIODevice::Text))
		return "false";
	QString str=filec.readLine();
	int num=0;
	Colour[0][0]=255;
	Colour[0][1]=255;
	Colour[0][2]=255;
	while (!filec.atEnd()) 
	{
		QString str = filec.readLine();
		QStringList strList = str.split(",", QString::SkipEmptyParts);
		typeIndex<<strList[0].toInt();;
		typeInitialCount<<strList[1].toInt();
		typeName<<strList[2];
		Colour[num+1][0]=strList[3].toInt();
		Colour[num+1][1]=strList[4].toInt();
		Colour[num+1][2]=strList[5].toInt();
		num++;
	}



	/// <定义一个每个颜色8位(bit)的_cols x _rows的彩色图像>
	u_rgb=new unsigned char[(size_t)_cols*_rows*3];
	size_t bytePerLine = (_cols* 24 + 31)/8;
	u_rgbshow= new unsigned char[(size_t)bytePerLine * _rows * 3];

	/// <首次显示>
	size_t ii,jj,hh;
	for (jj=0;jj<_rows*_cols;jj++)
	{
		for (ii=0;ii<3;ii++)
		{
			for (hh=0;hh<nType+1;hh++)
			{
				if (imgList[0]->imgData()[jj]==0)
				{
					u_rgb[ii*_rows*_cols+jj]=255;
				}
				if (imgList[0]->imgData()[jj]==hh)
				{
					u_rgb[ii*_rows*_cols+jj]=Colour[hh][ii];
				}
			}
		}
	}
	/// <首次显示>

	sendColsandRows(_cols,_rows,0);

	sendparameter(tr("Maximum Number Of Iterations: ")+QString::number(looptime));

	sendparameter(tr("Neighborhood influence: ")+QString::number(sizeWindows));

	sendparameter(tr("Acceleration for iterate: ")+QString::number(degree));

	sendparameter(tr("Cost Matrix")); 

	for (int i=0;i<nType;i++)
	{
		QString line;
		for (int j=0;j<nType;j++)
		{
			line=line+QString::number(t_filecost[i][j])+" ";
		}
		saveCount[i]=typeInitialCount[i];// 顺便用这个循环带一下
		sendparameter(line);
	}

	sendparameter(tr("Future year"));

	return true;
}

void SimulationProcess::saveResult(string filename)
{

	size_t i,j,k;
	TiffDataWrite pwrite;

	bool brlt ;

	brlt = pwrite.init(filename.c_str(), imgList[0]->rows(), imgList[0]->cols(), 1, \
		imgList[0]->geotransform(), imgList[0]->projectionRef(), GDT_Byte, 0);


	if (!brlt)
	{
		qDebug()<<"write init error!"<<endl;
		sendparameter(tr("Save error: ")+savepath.c_str());
		return ;
	}


	unsigned char _val = 0;
	//#pragma omp parallel for private(j, k, _val), num_threads(omp_get_max_threads())
	if (imgList[0]->imgData()!=NULL)
	{
		for (i=0; i<pwrite.rows(); i++)
		{
			for (j=0; j<pwrite.cols(); j++)
			{
				for (k=0; k<pwrite.bandnum(); k++)
				{
					_val = imgList[0]->imgData()[k*pwrite.rows()*pwrite.cols()+i*pwrite.cols()+j];
					pwrite.write(i, j, k, &_val);
				}
			}
		}
		//cout<<"write success!"<<endl;
		pwrite.close();

	}

	isSave=true;
}

double SimulationProcess::mypow(double _num,int times)
{
	double dTempNum=_num;
	for (int ii=0;ii<times;ii++)
	{
		dTempNum=dTempNum*_num;
	}
	return dTempNum;
}


void SimulationProcess::acceptFinisheCode(int _finished)
{
	finishedCode=_finished;
}

bool SimulationProcess::readImageData()
{
	bool bRlt;
	QFile file("./FilesGenerate/logFileSimulation.log");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	while (!file.atEnd()) 
	{
		QString str = file.readLine();
		if (str.trimmed()==tr("[Path of land use data]").trimmed())
		{
			QString str = file.readLine().trimmed();
			string landPath=str.toStdString();
			bRlt=imageOpenConver2uchar(landPath.c_str());
		}
		if (str.trimmed()==tr("[Path of probability data]").trimmed())
		{
			QString str = file.readLine().trimmed();
			string probPath=str.toStdString();
			bRlt=imageOpen(probPath.c_str());
		}
		if (str.trimmed()==tr("[Path of simulation result]").trimmed())
		{
			QString str = file.readLine().trimmed();
			savepath=str.toStdString();
		}
		if (str.trimmed()==tr("[Path of restricted area]").trimmed())
		{
			QString str = file.readLine().trimmed();
			if (str.trimmed()==tr("No restrict data").trimmed())
			{
				isRestrictExit=false;
				sendparameter(tr("No restricted areas"));
			}
			else
			{
				isRestrictExit=true;
				string restPath=str.toStdString();
				bRlt=imageOpenConver2uchar(restPath.c_str());
				sendparameter(tr("Setting up restricted areas ").trimmed());
			}
		}

	}
	file.close();

	if (!bRlt)
	{
		qDebug()<<"Read image error";
		sendparameter(tr("Read image error"));
	}

	return bRlt;
}

bool SimulationProcess::imageOpen(QString filename)
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	// <新建IO类>
	TiffDataRead* pread = new TiffDataRead;

	imgList.push_back(pread);

	string stdfilenamestr=filename.trimmed().toStdString(); // 不分开居然读不进来，醉了

	// <提取数据>
	if (!imgList.at(imgList.length()-1)->loadFrom(stdfilenamestr.c_str()))
	{
		qDebug()<<"load error!"<<endl;
		return false;
	}
	else
	{
		cout<<"load success!"<<endl;
	}

	return true;
}

bool SimulationProcess::imageOpenConver2uchar(QString filename)
{
	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	TiffDataRead* pread = new TiffDataRead;

	imgList.push_back(pread);

	string stdfilenamestr=filename.trimmed().toStdString(); 

	if (!imgList.at(imgList.length()-1)->loadFrom(stdfilenamestr.c_str()))
	{
		qDebug()<<"load error!"<<endl;
		return false;
	}
	else
	{
		imgList.at(imgList.length()-1)->convert2uchar();
		cout<<"convert success!"<<endl;
	}

	return true;
}

void SimulationProcess::startloop()
{
	time_t t = time(NULL); 

	srand(t);

	/// <读入限制转换矩阵>
	bool _isStatus=getUiparaMat();

	if (_isStatus!=true)
	{
		qDebug()<<"Get Parameter Error";
		return;
	}

	QString _label(tr("\n-----------------Start to iterate-----------------\n\nLand use type,"));
	for (int ii=0;ii<nType;ii++)
	{
		_label = _label + typeName[ii]+  tr(","); 
	}
	sendparameter(_label.left(_label.length() - 1));

	QString sendInfo(tr("Number of pixels of each land use before iteration,"));
	for (int ii=0;ii<nType;ii++)
	{
		sendInfo = sendInfo + QString::number(saveCount[ii])+  tr(","); 
	}
	sendparameter(sendInfo.left(sendInfo.length() - 1));  

}


void SimulationProcess::stoploop()
{
	isbreak=2;
}


void SimulationProcess::runloop2()
{
	double timestart;
	double timeend;
	double timeused;
	double start,end ;
	double _max=0;
	int k=0;
	bool isRestrictPix;
	double* initialDist;
	double* dynaDist;
	double* adjustment;
	double* adjustment_effect;
	double* initialProb;
	int* opposite2reverse;
	double* bestdis;

	vector<double> inherant;
	long piexelsum=0;
	int numofYear=0;
	int historyDis=0;
	int sumDis=0;
	int stasticHistroyDis=0;
	bool isSwitchIniYear=false;

	time_t t = time(NULL); 
	srand(t);
	start=GetTickCount();  
	adjustment=new double[nType];
	initialDist=new double[nType];
	dynaDist=new double[nType];
	adjustment_effect=new double[nType];
	initialProb=new double[nType];
	opposite2reverse=new int[nType];
	bestdis=new double[nType];

	for (int ii=0;ii<nType;ii++)
	{
		adjustment_effect[ii]=1;    
		piexelsum+=typeInitialCount[ii];
		opposite2reverse[ii]=0;
	}

	QString iniheader;
	for (int ii=0;ii<nType;ii++)
	{
		iniheader = iniheader +  typeName[ii]+  tr(",");
	}
	//	out<<iniheader.left(iniheader.length() - 1)<<endl;


	// 已经迭代次数

	size_t tSumPixel=0;
	for (int ii=0;ii<nType;ii++)
	{
		tSumPixel+=goalNum[ii];
	}

	if (piexelsum!=tSumPixel)
	{
		QString sendInfo(tr("Warning: the sum of future pixel(%1) is not equal to the sum of initial pixel(%2), the model may not automatic stop...").arg(tSumPixel).arg(piexelsum));
		sendparameter(sendInfo);
	}


	for(;;)
	{


		if (finishedCode==k)
		{

			/// <每次迭代前参数调整>
			for (int ii=0;ii<nType;ii++)
			{
				mIminDis2goal[ii]=goalNum[ii]-saveCount[ii];

				if (k==0||isSwitchIniYear==true) 
				{
					initialDist[ii]=mIminDis2goal[ii]; 
					dynaDist[ii]=initialDist[ii]*1.01;   
					bestdis[ii]=initialDist[ii];
				}

				if (abs(bestdis[ii])>abs(mIminDis2goal[ii])) 
				{
					bestdis[ii]=mIminDis2goal[ii];
				}
				else 
				{
					if ((abs(mIminDis2goal[ii])-abs(bestdis[ii]))/abs(initialDist[ii])>0.05)
					{
						opposite2reverse[ii]=1;
					}
				}


				adjustment[ii]=mIminDis2goal[ii]/dynaDist[ii];


				if (adjustment[ii]<1&&adjustment[ii]>0)  
				{
					dynaDist[ii]=mIminDis2goal[ii];

					if (initialDist[ii]>0&&adjustment[ii]>(1-degree)) 
					{
						adjustment_effect[ii]=adjustment_effect[ii]*(adjustment[ii]+degree);
					}

					if (initialDist[ii]<0&&adjustment[ii]>(1-degree)) 
					{
						adjustment_effect[ii]=adjustment_effect[ii]*(1/(adjustment[ii]+degree));
					}

				}

				if ((initialDist[ii]>0&&adjustment[ii]>1))   
				{
					adjustment_effect[ii]=adjustment_effect[ii]*adjustment[ii]*adjustment[ii];
				}

				if ((initialDist[ii]<0&&adjustment[ii]>1))  
				{
					adjustment_effect[ii]=adjustment_effect[ii]*(1.0/adjustment[ii])*(1.0/adjustment[ii]);
				}

			}



			QString sendInher;
			for (int ii=0;ii<nType;ii++)
			{
				sendInher = sendInher + QString::number(adjustment_effect[ii])+  tr(",");
			}
			//			out<<sendInher.left(sendInher.length() - 1)<<endl;


			/// <二维迭代开始>

			size_t i;
			size_t j;
			for (i=0;i<_rows;i++)
			{
				for (j=0;j<_cols;j++)
				{
	
					if ((imgList[0]->imgData()[i*_cols+j]<typeIndex[0]||imgList[0]->imgData()[i*_cols+j]>typeIndex[nType-1]))//
					{
						temp[i*_cols+j]=imgList[0]->imgData()[i*_cols+j];
					}
					else/// <否则，计算该元胞分布概率>
					{
						if (isRestrictExit==true)
						{
							if (imgList[2]->imgData()[i*_cols+j]==0) 
							{
								isRestrictPix=true;
							}
							else
							{
								isRestrictPix=false;
							}
						}
						else
						{
							isRestrictPix=false; 
						}

						if (isRestrictPix==false)
						{
							if (sizeWindows!=1)
							{

								for (int ii=0;ii<nType;ii++)
								{
									val[ii]=0;
								}
								for (int m =0; m<numWindows; m++)
								{
									int _x = i+direction[m][0];
									int _y = j+direction[m][1];
									if (_x<0 || _y<0 || _x>=_rows || _y>=_cols)
										continue;
									for (int _ii=1;_ii<=nType;_ii++)
									{
										if (imgList[0]->imgData()[_x*_cols+_y] == _ii)
										{
											val[_ii-1]+=1;
										}
									}
								}
							}
							else
							{
								for (int _ii=1;_ii<=nType;_ii++)
								{
									val[_ii-1]=1; 
								}
								numWindows=1; 
							}

							int oldType=imgList[0]->imgData()[i*_cols+j]-1;

							double Inheritance =0;

							switch(imgList[1]->datatype())
							{
							case GDT_Float32:

								for (int _ii=0;_ii<nType;_ii++)
								{

									mdNeiborhoodProbability[_ii]= val[_ii]/(double)numWindows;

									double dSuitability;

									dSuitability=*(float*)(imgList[1]->imgData()+(_cols*_rows*(_ii)+i*_cols+j)*sizeof(float));

									double _neigheffect=mdNeighIntensity[_ii];

									mdNeiborhoodProbability[_ii]=mdNeiborhoodProbability[_ii]*_neigheffect; 

									probability[_ii]=dSuitability*mdNeiborhoodProbability[_ii];

									initialProb[_ii]=dSuitability;

									if (oldType==_ii)
									{
										Inheritance=10*nType;
										probability[_ii]=probability[_ii]*(adjustment_effect[_ii])*Inheritance;
									}


								}

								break;

							case GDT_Float64: 

								for (int _ii=0;_ii<nType;_ii++)
								{
									mdNeiborhoodProbability[_ii]= val[_ii]/(double)numWindows;

									double dSuitability;

									dSuitability=*(double*)(imgList[1]->imgData()+(_cols*_rows*(_ii)+i*_cols+j)*sizeof(double));

									double _neigheffect=mdNeighIntensity[_ii]+0.000001; 

									mdNeiborhoodProbability[_ii]=mdNeiborhoodProbability[_ii]*_neigheffect; 

									probability[_ii]=dSuitability*mdNeiborhoodProbability[_ii];

									initialProb[_ii]=dSuitability;

									if (oldType==_ii)
									{
										Inheritance=10*nType;
										probability[_ii]=probability[_ii]*(adjustment_effect[_ii])*Inheritance;
									}

								}

								break;

							default:
								return ;
							}


							for (int jj=0;jj<nType;jj++)
							{
								probability[jj]=probability[jj]*t_filecost[oldType][jj];
							}

							double SumProbability=0;
							for (int ii=0;ii<nType;ii++)
							{
								SumProbability+=probability[ii];		
							}

							mdRoulette[0]=0;
							for (int ii=0;ii<nType;ii++)
							{
								mdRoulette[ii+1]=mdRoulette[ii]+mdNeiborhoodProbability[ii];
							}

							bool isConvert;

							double rdmData=(double)rand()/(double)RAND_MAX;

							for (int _kk=0;_kk<nType;_kk++)
							{

								int newType=_kk;

								if (rdmData<=mdRoulette[newType+1]&&rdmData>mdRoulette[newType]) 
								{
									double rdmData=(double)rand()/(double)RAND_MAX;
									double rdmData2=(double)rand()/(double)RAND_MAX;
									double rdmData3=(double)rand()/(double)RAND_MAX;

									if((oldType!=newType)&&(t_filecost[oldType][newType]!=0))
									{
										isConvert=true;
									}
									else
									{
										isConvert=false;
									}

									double _disChangeFrom;
									_disChangeFrom=mIminDis2goal[oldType];

									double _disChangeTo;
									_disChangeTo=mIminDis2goal[newType];

									if (initialDist[newType]>=0&&_disChangeTo==0) 
									{
										adjustment_effect[newType]=1;
										isConvert=false;
									}

									if (initialDist[oldType]<=0&&_disChangeFrom==0) 
									{
										adjustment_effect[oldType]=1;
										isConvert=false;
									}


									if (initialDist[oldType]>=0&&opposite2reverse[oldType]==1) 
									{
										isConvert=false;
									}
									if (initialDist[newType]<=0&&opposite2reverse[newType]==1) 
									{
										isConvert=false;
									}


									if (isConvert==true)                   
									{
										if ((rdmData3+(1.0/nType))/((k+1))<initialProb[newType])
										{
											//isConvert=true;
										}
										else
										{
											isConvert=false;
										}
									}


									if (isConvert==true)
									{
										temp[i*_cols+j]=(unsigned char)(newType+1);
										saveCount[newType]+=1;
										saveCount[oldType]-=1;
										mIminDis2goal[newType]=goalNum[newType]-saveCount[newType];
										mIminDis2goal[oldType]=goalNum[oldType]-saveCount[oldType];
										break; 
									}
									else
									{
										temp[i*_cols+j]=imgList[0]->imgData()[i*_cols+j]; 
										break; 
									}

									opposite2reverse[oldType]=0;
									opposite2reverse[newType]=0; 

								}
								else
								{
									temp[i*_cols+j]=imgList[0]->imgData()[i*_cols+j];
								}

							}
						}
						else
						{
							temp[i*_cols+j]=imgList[0]->imgData()[i*_cols+j];
						}
					}
				}
			}

			for (int ii=0;ii<nType;ii++)
			{
				saveCount[ii]=0;
			}

			for (int jj=0;jj<_rows*_cols;jj++)
			{
				imgList[0]->imgData()[jj]=temp[jj];
				for (int kk=0;kk<nType;kk++)
				{
					if (temp[jj]==typeIndex[kk])
					{
						saveCount[kk]+=1;
						break;
					}
				}
				for (int ii=0;ii<3;ii++)
				{
					for (int hh=0;hh<nType+1;hh++)
					{
						if (temp[jj]==0)
						{
							u_rgb[ii*_rows*_cols+jj]=255;
						}
						if (temp[jj]==hh)
						{
							u_rgb[ii*_rows*_cols+jj]=Colour[hh][ii];
						}
					}
				}
			}

			sendColsandRows(_cols,_rows,1+k);

			QString sendInfo(tr("Number of pixels of each land use at %1 iteration,").arg(k+1));
			for (int ii=0;ii<nType;ii++)
			{
				sendInfo = sendInfo + QString::number(saveCount[ii])+  tr(",");
			}
			sendparameter(sendInfo.left(sendInfo.length() - 1));

			k++;


			sumDis=0;

				for (int ii=0;ii<nType;ii++)
				{
					sumDis=sumDis+abs(mIminDis2goal[ii]);
				}

				if (sumDis==0||(stasticHistroyDis>5)&&(sumDis<(piexelsum*0.0001)))
				{
					sendparameter(tr("\nSave image at: ")+QString::fromStdString(savepath+"\n"));

					QString str=QString::fromStdString(savepath);

					saveResult(str.trimmed().toStdString());

					isbreak=1;
				}
		}

		if (isbreak==1||isbreak==2||k>=looptime-1)
		{
			break;
		}


	}

	if (isbreak==2||isSave==false) 
	{
		sendparameter(tr("\nSave image at: ")+QString::fromStdString(savepath+"\n"));

		QString str=QString::fromStdString(savepath);

		saveResult(str.trimmed().toStdString());
	}

	end=GetTickCount();   

	double timecost=end-start;  

	QString sendtime(tr("Time used: %1 s\n").arg(timecost/1000.0,1,'f',4));
	sendparameter(sendtime);

	delete[] initialProb;
	delete[] adjustment;
	delete[] dynaDist;
	delete[] adjustment_effect;
	delete[] initialDist;
	delete[] saveCount;
	delete[] probability;
	delete[] mIminDis2goal;
	delete[] mdNeiborhoodProbability;
	delete[] mdRoulette;
	delete[] goalNum;
	delete[] val;
	delete[] bestdis;
	delete[] opposite2reverse;

	if (sizeWindows!=1)
	{
		for(int i=0;i<numWindows;i++)
		{delete []direction[i];}
		delete []direction;
	}

	for(int i=0;i<nType;i++)
	{delete []t_filecost[i];}
	delete []t_filecost;

	for(int i=0;i<(nType+1);i++)
	{delete []Colour[i];}
	delete []Colour;

	for (;;)
	{
		if (finishedCode>=k)
		{
			break;
		}
	}

	delete[] u_rgbshow;
	delete[] u_rgb;

}

