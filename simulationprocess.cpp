#include "simulationprocess.h"
#include "dynasimulation.h"
#include "TiffDataRead.h"
#include "geodpcasys.h"
#include "TiffDataWrite.h"
#include "CImg.h"
#include <QTableWidget>
#include <QMessageBox>
#include <iomanip>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <QtCore>

using namespace std;
using namespace cimg_library;

SimulationProcess::SimulationProcess( DynaSimulation* _dsn )
{
	goalNum=NULL;
	saveCount=NULL;
	min__dis2goal=NULL;


	val=NULL;
	ra=NULL;
	raSum=NULL;
	probability=NULL;
	sProbability=NULL;
	normalProbability=NULL;
	mdNeighIntensity=NULL;

	temp=NULL;
	restrict=NULL;


	m_dsn=_dsn;
	_rows=m_dsn->lau_poDataset2.at(0)->rows();
	_cols=m_dsn->lau_poDataset2.at(0)->cols();
	isbreak=0;
	if (!m_dsn->ui.rstlineEdit->text().isEmpty())
	{
		isRestrictExit=true;
	}
	else
	{
		isRestrictExit=false;
	}


	connect(this,SIGNAL(sendColsandRows(int,int,int)),_dsn,SLOT(getColsandRowsandshowDynamicOnUi(int,int,int)));
	connect(this,SIGNAL(sendparameter(QString)),_dsn,SLOT(getParameter(QString)));
	
}


SimulationProcess::~SimulationProcess()
{

}


///
/// <检查限制转换区域的格式>
///
template<class TT> bool SimulationProcess::testData()
{
	restrict=new unsigned char[_cols*_rows];
	for (int ii=0;ii<_cols*_rows;ii++)
	{
		TT temp=*(TT*)(m_dsn->rest_poDataset.at(0)->imgData()+ii*sizeof(TT));
		restrict[ii]=(unsigned char)temp;
	}
	return true;
}


void SimulationProcess::testRestrict()
{
	bool bRlt;

	switch(m_dsn->rest_poDataset.at(0)->datatype())
	{
	case GDT_Byte:
		bRlt = testData<unsigned char>();
		break;
	case GDT_UInt16:
		bRlt = testData<unsigned short>();
		break;
	case GDT_Int16:
		bRlt = testData<short>();
		break;
	case GDT_UInt32:
		bRlt = testData<unsigned int>();
		break;
	case GDT_Int32:
		bRlt = testData<int>();
		break;
	case GDT_Float32:
		bRlt = testData<float>();
		break;
	case GDT_Float64:
		bRlt = testData<double>();
		break;
	default:
		cout<<"CGDALRead::loadFrom : unknown data type!"<<endl;
		return ;
	}
}
///
/// <从界面获取参数>
///

// <巧妙的排序>
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

QString SimulationProcess::getUiparaMat()
{

	t_filerestrict=new double*[_classes];
	for(int i=0;i<_classes;i++)
	{
		t_filerestrict[i]=new double[_classes];
	}
	t_filecost=new double*[_classes];
	for(int i=0;i<_classes;i++)
	{
		t_filecost[i]=new double[_classes];
	}

	double temp;
	for (int i=0;i<_classes;i++)
	{
		priority_level.push_back(i+1);
		priority_record.push_back(0);

		for (int j=0;j<_classes;j++)
		{
			temp=m_dsn->restricttableWidget->item(i,j)->text().trimmed().toDouble();
			if (temp==1||temp==0)
			{
				t_filerestrict[i][j]=temp;
			}
			else
			{
				QString status = QString("Input wrong at Restricted Matrix table unit: %1 and %2") .arg(i).arg(j);
				return status;
			}
			temp=m_dsn->switchcost->item(i,j)->text().trimmed().toDouble();
			if (temp<=1&&temp>=0)
			{
				t_filecost[i][j]=temp;
			}
			else
			{
				QString status = QString("Input wrong at Cost Matrix table unit: %1 and %2") .arg(i).arg(j);
				return status;
			}
		}
	}

	for (int i=0;i<_classes;i++)
	{
		for (int j=0;j<_classes;j++)
		{
			if(t_filerestrict[i][j]==1)
			{
				priority_record[i]+=1;
			}
		}
	}

	vector<forArray> save_arr;
	for (int kk=0;kk<_classes;kk++)
	{
		myPushback(save_arr,priority_record.at(kk),priority_level.at(kk));
	}
	sort(save_arr.begin(),save_arr.end(),sortbymb1);

	for (int kk=0;kk<_classes;kk++)
	{
		priority_record[kk]=save_arr[kk].mb1;
		priority_level[kk]=save_arr[kk].mb2;
	}

	save_arr.clear();

	tmp.push_back(priority_record[0]);

	int stas;

	// 挑出独立元素
	for (int ii=0;ii<_classes;ii++)
	{
		stas=0;
		for (int jj=0;jj<tmp.size();jj++)
		{
			if (tmp[jj]==priority_record[ii])
			{
				break;
			}
			else
			{
				stas++;
			}
		}
		if (stas==tmp.size())
		{
			tmp.push_back(priority_record[ii]);
		}
	}

	// 赋值土地等级
	for (int ii=0;ii<_classes;ii++)
	{
		for (int jj=0;jj<tmp.size();jj++)
		{
			if (priority_record[ii]==tmp[jj])
			{
				priority_record[ii]=jj+1;
			}
		}
	}

	vector<forArray> save_arr1;
	for (int kk=0;kk<_classes;kk++)
	{
		myPushback(save_arr1,priority_level.at(kk),priority_record.at(kk));
	}
	sort(save_arr1.begin(),save_arr1.end(),sortbymb1);

	for (int kk=0;kk<_classes;kk++)
	{
		priority_record[kk]=save_arr1[kk].mb2;
		priority_level[kk]=save_arr1[kk].mb1;
	}

	save_arr1.clear();

	return tr("true");
}

void SimulationProcess::saveResult()
{

	m_dsn->lau_poDataset2.at(0)->deleteImgData();
	if(isRestrictExit==true)
	{
		m_dsn->rest_poDataset.at(0)->deleteImgData();
	}

	int i,j,k;
	TiffDataWrite pwrite;

	bool brlt ;

	if (m_dsn->nodatavalue2>255||m_dsn->nodatavalue2<1)
	{
		brlt = pwrite.init(m_dsn->ui.saveSimlineEdit->text().toStdString().c_str(), m_dsn->lau_poDataset2.at(0)->rows(), m_dsn->lau_poDataset2.at(0)->cols(), 1, \
			m_dsn->lau_poDataset2.at(0)->geotransform(), m_dsn->lau_poDataset2.at(0)->projectionRef(), GDT_Byte, 0);
	}
	else
	{
		brlt = pwrite.init(m_dsn->ui.saveSimlineEdit->text().toStdString().c_str(), m_dsn->lau_poDataset2.at(0)->rows(), m_dsn->lau_poDataset2.at(0)->cols(), 1, \
			m_dsn->lau_poDataset2.at(0)->geotransform(), m_dsn->lau_poDataset2.at(0)->projectionRef(), GDT_Byte, m_dsn->nodatavalue2);
	}
	    
	if (!brlt)
	{
		//cout<<"write init error!"<<endl;
		sendparameter(tr("保存路径错误： ")+m_dsn->ui.saveSimlineEdit->text().toStdString().c_str());
		return ;
	}
	unsigned char _val = 0;
	//#pragma omp parallel for private(j, k, _val), num_threads(omp_get_max_threads())
	if (m_dsn->_landuse2!=NULL)
	{
		for (i=0; i<pwrite.rows(); i++)
		{
			for (j=0; j<pwrite.cols(); j++)
			{
				for (k=0; k<pwrite.bandnum(); k++)
				{
					_val = m_dsn->_landuse2[k*pwrite.rows()*pwrite.cols()+i*pwrite.cols()+j];
					pwrite.write(i, j, k, &_val);
				}
			}
		}
		//cout<<"write success!"<<endl;
		pwrite.close();
		delete[] m_dsn->_landuse2;
		m_dsn->_landuse2=NULL;
	}
	m_dsn->lau_poDataset2.at(0)->close();
	if (m_dsn->rest_poDataset.size()>0)
	{
		m_dsn->rest_poDataset.at(0)->close();
		delete[] restrict;
		restrict=NULL;
	}
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

int** ScanWindow(int sizeWindows)
{
	//defining a two-dimensions window
	int numWindows=sizeWindows*sizeWindows-1;
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

void SimulationProcess::startloop()
{

	time_t t = time(NULL); 
	srand(t);

	temp = new unsigned char[_cols*_rows];



	/// <参数>
	sizeWindows=m_dsn->ui.CAneigh->value();
	looptime=m_dsn->ui.itelineEdit->text().trimmed().toInt();/// <迭代次数>


// 	if (m_dsn->ui.radioMonteCarlo->isChecked()!=true)
// 	{
		degree=m_dsn->ui.deglineEdit->text().toDouble();
/*	}*/

	_classes=m_dsn->rgbLanduseType2.size();

	int _Sum=0;
	numWindows=sizeWindows*sizeWindows-1;

	//---------------------------define a scanning window--------------------------- 
	//
	if (numWindows!=0)
	{
		direction=ScanWindow(sizeWindows);
	}
	//
	/// <读入限制转换矩阵>
	QString _status=getUiparaMat();

	if (_status!="true")
	{
		sendparameter(_status);
		return;
	}

	/// <初始化一些中间变量>
	raSum=new double[_classes+1];
	ra=new double[_classes];
	probability=new double[_classes];

	min__dis2goal=new int[_classes];

	saveCount=new int[_classes];
	val=new int[_classes];
	goalNum=new int[_classes];
	mdNeighIntensity=new double[_classes];
	/// <初次统计文本>
	raSum[0]=0;/// <单独赋值>
	for (int i=0;i<_classes;i++)/// <循环赋值>
	{
		ra[i]=0;
		val[i]=0;
		raSum[i+1]=0;
		probability[i]=0;
		saveCount[i]=m_dsn->futuretableWidget->item(0,i)->text().trimmed().toDouble();
		goalNum[i]=m_dsn->futuretableWidget->item(1,i)->text().trimmed().toDouble();
		mdNeighIntensity[i]=m_dsn->mqIntenofneigh->item(0,i)->text().trimmed().toDouble();
		_Sum+=saveCount[i];
	}



	/// <定义RGB数组>
	Colour=new short*[_classes+1];
	for(int i=0;i<(_classes+1);i++)
	{
		Colour[i]=new short[3];
	}

	for (int ii=0;ii<_classes;ii++)
	{

		Colour[0][0]=255;
		Colour[0][1]=255;
		Colour[0][2]=255;

		Colour[ii+1][0]=m_dsn->rgbType2.at(ii).red();
		Colour[ii+1][1]=m_dsn->rgbType2.at(ii).green();
		Colour[ii+1][2]=m_dsn->rgbType2.at(ii).blue();
	}

	/// <定义一个每个颜色8位(bit)的_cols x _rows的彩色图像>
	m_dsn->u_rgb=new unsigned char[_cols*_rows*3];
	int bytePerLine = (_cols* 24 + 31)/8;
	m_dsn->u_rgbshow= new unsigned char[bytePerLine * _rows * 3];

	/// <首次显示>
	for (int jj=0;jj<_rows*_cols;jj++)
	{
		for (int ii=0;ii<3;ii++)
		{
			for (int hh=0;hh<_classes+1;hh++)
			{
				if (m_dsn->_landuse2[jj]==0)
				{
					m_dsn->u_rgb[ii*_rows*_cols+jj]=255;
				}
				if (m_dsn->_landuse2[jj]==hh)
				{
					m_dsn->u_rgb[ii*_rows*_cols+jj]=Colour[hh][ii];
				}
			}
		}
	}
	/// <首次显示>



	sendColsandRows(_cols,_rows,0);

	sendparameter(tr("设定迭代次数： ")+QString::number(looptime));

	sendparameter(tr("设定邻域： ")+QString::number(sizeWindows));

// 	if(m_dsn->ui.radioMonteCarlo->isChecked()!=true)
// 	{
		sendparameter(tr("设定后期加速： ")+QString::number(degree));
// 	}
// 	else
// 	{
// 		sendparameter(tr("设定蒙特卡罗自由度"));
// 	}

	if(m_dsn->ui.norstradioButton->isChecked()==true)
	{
		sendparameter(tr("不设定限制转换区域"));
	}
	else
	{
		testRestrict();
		sendparameter(tr("设定限制转换区域"));
	}

	QString _label(tr("\n-----------------开始迭代，输出像元数-----------------\n\n显示土地利用类型标签,"));
	for (int ii=0;ii<_classes;ii++)
	{
		_label = _label + m_dsn->lauTypeName2.at(ii)+  tr(","); // m_dsn->lauTypeName2.at(ii)+
	}
	sendparameter(_label.left(_label.length() - 1));

	QString __send(tr("第 0 次初始各类像元个数,"));
	for (int ii=0;ii<_classes;ii++)
	{
		__send = __send + QString::number(saveCount[ii])+  tr(","); // m_dsn->lauTypeName2.at(ii)+
	}
	sendparameter(__send.left(__send.length() - 1));  //__send.chop(1);

}



void SimulationProcess::runloop()
{
	double timestart;
	double timeend;
	double timeused;
	double start,end ;
	double passingThrehold;
	double _max=0;
	int k=0;
	bool isRestrictPix;
	double* initialDist;
	double* dynaDist;
	double* adjustment;
	double* adjustment_effect;
	double* adjustmentNorma;
	double passProb1;
	double _tmp_min;



	passingThrehold=1.0/double(looptime)*2;
	time_t t = time(NULL); 
	srand(t);
	start=GetTickCount();  
	adjustment=new double[_classes];
	initialDist=new double[_classes];
	dynaDist=new double[_classes];
	adjustment_effect=new double[_classes];
	adjustmentNorma=new double[_classes];
	for (int ii=0;ii<_classes;ii++)
	{
		adjustment_effect[ii]=1;    // 初始化动态适应参数
		adjustmentNorma[ii]=1;
	}

	QFile file("Inheritance.csv");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);

	QString iniInher;
	QString iniheader;
	for (int ii=0;ii<_classes;ii++)
	{
		iniheader = iniheader +  m_dsn->lauTypeName2.at(ii)+  tr(",");
		iniInher = iniInher + QString::number(adjustment_effect[ii])+  tr(",");
	}
	out<<iniheader.left(iniheader.length() - 1)<<endl;

	vector<double> inherant;
	

	for(;;)
	{

		if (m_dsn->ui.pro_dynagraphicsView->seeIsFinish()==k)
		{

			/// <每次迭代前参数调整>

			_tmp_min=1;

			for (int ii=0;ii<_classes;ii++)
			{
				min__dis2goal[ii]=goalNum[ii]-saveCount[ii];

				if (k==0) // 确定第一次的方向
				{
					initialDist[ii]=min__dis2goal[ii]; // 初始化距离
					dynaDist[ii]=initialDist[ii];   // 初始化动态距离
				}

				adjustment[ii]=min__dis2goal[ii]/dynaDist[ii];

				if (adjustment[ii]<1&&adjustment[ii]>0)  // 方向对了，更新dynadist，用于下次检查方向是否对
				{
					dynaDist[ii]=min__dis2goal[ii];

					if (initialDist[ii]>0&&adjustment[ii]>(1-degree))  // 速度太慢，加速
					{
						adjustment_effect[ii]=adjustment_effect[ii]*(adjustment[ii]+degree);
					}

					if (initialDist[ii]<0&&adjustment[ii]>(1-degree))  // 速度太慢，加速
					{
						adjustment_effect[ii]=adjustment_effect[ii]*(1/(adjustment[ii]+degree));
					}

				}


				if (initialDist[ii]>0&&adjustment[ii]>1)   // 要增加的反而少了，加强惯性
				{
					adjustment_effect[ii]=adjustment_effect[ii]*adjustment[ii];
				}

				if (initialDist[ii]<0&&adjustment[ii]>1)   // 要减少的反而多了，减弱惯性
				{
					adjustment_effect[ii]=adjustment_effect[ii]*(1.0/adjustment[ii]);
				}

				inherant.push_back(adjustment_effect[ii]);

			}

	        qSort(inherant.begin(), inherant.end());

			double max_in_effect=inherant[_classes-1];
			double min_in_effect=inherant[0];

			for (int ii=0;ii<_classes;ii++)
			{
				adjustmentNorma[ii]=adjustment_effect[ii]/max_in_effect;
			}

			inherant.clear();

			QString sendInher;
			for (int ii=0;ii<_classes;ii++)
			{
				sendInher = sendInher + QString::number(adjustment_effect[ii])+  tr(",");
			}
			out<<sendInher.left(sendInher.length() - 1)<<endl;


			/// <二维迭代开始>
			for (int i=0;i<_rows;i++)
			{
				for (int j=0;j<_cols;j++)
				{
					if ((m_dsn->_landuse2[i*_cols+j]<m_dsn->rgbLanduseType2[_classes-_classes]||m_dsn->_landuse2[i*_cols+j]>m_dsn->rgbLanduseType2[_classes-1]))/// <掩模、范围>
					{
						temp[i*_cols+j]=m_dsn->_landuse2[i*_cols+j];
					}
					else/// <否则，计算该元胞分布概率>
					{
						if (isRestrictExit==true)
						{
							if (restrict[i*_cols+j]==0)
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
							isRestrictPix=false; /// <判断是否是限制像元>
						}

						if (isRestrictPix==false)
						{
							/// <统计窗口内各类个数>
							if (numWindows!=0)
							{
							
								for (int ii=0;ii<_classes;ii++)
								{
									val[ii]=0;
								}
								for (int m =0; m<numWindows; m++)
								{
									int _x = i+direction[m][0];
									int _y = j+direction[m][1];
									if (_x<0 || _y<0 || _x>=_rows || _y>=_cols)
										continue;
									for (int _ii=1;_ii<=_classes;_ii++)
									{
										if (m_dsn->_landuse2[_x*_cols+_y] == _ii)
										{
											val[_ii-1]+=1;
										}
									}
								}
							}
							else
							{
								for (int _ii=1;_ii<=_classes;_ii++)
								{
									val[_ii-1]=0;
								}
							}

							///
							/// <阈值计算>
							///

							/// <取出地类类别标签>
							int __label=m_dsn->_landuse2[i*_cols+j]-1;

							double Inheritance =0;

							double stochastic_perturbation=(double)rand()/(double)RAND_MAX;

							/// <求出基本概率>
							switch(m_dsn->prob_poDataset2.at(0)->datatype())
							{
							case GDT_Float32:

								for (int _ii=0;_ii<_classes;_ii++)
								{
									double _tmp= val[_ii]/(double)numWindows;

									if (numWindows!=0)
									{
										ra[_ii]=_tmp;
									}
									else
									{
										ra[_ii]=0;
									}

									float __tmp;
									__tmp=*(float*)(m_dsn->prob_poDataset2.at(0)->imgData()+(_cols*_rows*(_ii)+i*_cols+j)*sizeof(float));

									//double rdmData=(double)rand()/(double)RAND_MAX;

									if (numWindows!=0)
									{
										double _neigheffect=mdNeighIntensity[_ii];
										ra[_ii]=ra[_ii]*_neigheffect; 
										//结合高精度与论文写作
										//double rdmData=(double)rand()/((double)RAND_MAX*(1/__tmp));
										probability[_ii]=mypow(double(__tmp),1)*ra[_ii];// 直接阈值分割算了，还模拟个蛋蛋
										//probability[_ii]=__tmp*ra[_ii]*rdmData;// 正常模型


// 										if (__label==_ii)
// 										{
// 											Inheritance=1;
// 											probability[_ii]+=Inheritance*adjustment_effect[_ii]; 
// 										}


 										if (__label==_ii)
 										{
											Inheritance=1;
											probability[_ii]*=Inheritance*adjustment_effect[_ii]; 
										}


									}
									else
									{
										probability[_ii]=mdNeighIntensity[_ii]+__tmp+adjustmentNorma[_ii];

										if (__label==_ii)
										{
											Inheritance=1;
											probability[_ii]+=Inheritance*adjustment_effect[_ii]; 
										}
									}

								}

								break;

							case GDT_Float64: // <保留之前的算法>

								for (int _ii=0;_ii<_classes;_ii++)
								{
									double _tmp= val[_ii]/(double)numWindows;
									if (numWindows!=0)
									{
										ra[_ii]=_tmp;
									}
									else
									{
										ra[_ii]=0;
									}

									double __tmp;
									__tmp=*(double*)(m_dsn->prob_poDataset2.at(0)->imgData()+(_cols*_rows*(_ii)+i*_cols+j)*sizeof(double));

									if (numWindows!=0)
									{
										double _neigheffect=mdNeighIntensity[_ii];
										ra[_ii]=ra[_ii]*_neigheffect; // global 版本专用

										//结合高精度与论文写作
										double rdmData=(double)rand()/((double)RAND_MAX*(1/__tmp));
										probability[_ii]=__tmp*ra[_ii]*rdmData;// 论文版本

										if (__label==_ii)
										{
											Inheritance=1;
											probability[_ii]+=Inheritance*adjustment_effect[_ii]; 
										}

									}
									else
									{
										probability[_ii]=mdNeighIntensity[_ii]+__tmp+adjustmentNorma[_ii];

										if (__label==_ii)
										{
											Inheritance=1;
											probability[_ii]+=Inheritance*adjustment_effect[_ii]; 
										}
									}

								}

								break;

							default:
								return ;
							}


							/// <基本概率乘以限制、转换代价>
							for (int ii=0;ii<_classes;ii++)
							{
								if (ii==(m_dsn->_landuse2[i*_cols+j]-1))
								{
									for (int jj=0;jj<_classes;jj++)
									{
										probability[jj]=probability[jj]*t_filerestrict[ii][jj]*t_filecost[ii][jj];
									}
									break;/// <break很重要>
								}
							}
							/// <归一化>
							double SumProbability=0;
							for (int ii=0;ii<_classes;ii++)
							{
								SumProbability+=probability[ii];
							}
							for (int _ii=0;_ii<_classes;_ii++)
							{
								if (SumProbability!=0)
								{
									double ___tmp= probability[_ii]/SumProbability;
									ra[_ii]=___tmp;
								}
								else
								{
									ra[_ii]=0;
								}
							}
							/// <构成转盘>
							raSum[0]=0;
							for (int ii=0;ii<_classes;ii++)
							{
								raSum[ii+1]=raSum[ii]+ra[ii];
							}



							///
							/// <模型关键> --------------------------------------------------------------------------------------------------------------------------------------------
							///

							// <通过标签>
							bool isPassing=true;

							double rdmData=(double)rand()/(double)RAND_MAX;

							// 转盘
// 							for (int _kk=1;_kk<=_classes;_kk++)
// 							{
// 								if (rdmData<=raSum[_kk]&&rdmData>raSum[_kk-1])
// 								{
// 								
// 									/// <正常情况，限制少，速度快>
// 									if (_kk!=m_dsn->_landuse2[i*_cols+j]&&isPassing==true) 
// 									{
// 
// 										double _disChangeFrom;
// 										_disChangeFrom=min__dis2goal[__label];
// 
// 										double _disChangeTo;
// 										_disChangeTo=min__dis2goal[_kk-1];
// 
// 										if (initialDist[_kk-1]>=0&&_disChangeTo==0) // 目标多于初始，达到目标后封顶，不让别的转成它，恢复最大惯性系数
// 										{
// 											temp[i*_cols+j]=m_dsn->_landuse2[i*_cols+j];
// 											adjustment_effect[_kk-1]=1;
// 											break;
// 										}
// 
// 										if (initialDist[__label]<=0&&_disChangeFrom==0) // 目标少于初始，达到目标后封底，不让它转成别人，恢复最小惯性系数
// 										{
// 											temp[i*_cols+j]=m_dsn->_landuse2[i*_cols+j];
// 											adjustment_effect[__label]=1;
// 											break;
// 										}
// 
// 										temp[i*_cols+j]=(unsigned char)_kk;
// 										saveCount[_kk-1]+=1;
// 										saveCount[__label]-=1;
// 										min__dis2goal[_kk-1]=goalNum[_kk-1]-saveCount[_kk-1];
// 										min__dis2goal[__label]=goalNum[__label]-saveCount[__label];
// 										break;
// 
// 									}
// 									else
// 									{
// 										temp[i*_cols+j]=m_dsn->_landuse2[i*_cols+j];
// 									}
// 								}
// 								else
// 								{
// 									temp[i*_cols+j]=m_dsn->_landuse2[i*_cols+j];
// 								}
// 							}

							double maxProb=probability[0];

							int outputlabel=1;

							for (int _kk=1;_kk<_classes;_kk++)
							{
								if (probability[_kk]>maxProb)
								{
									maxProb=probability[_kk];
									outputlabel=_kk+1;
								}
							}

							bool label1=false;
							bool label2=false;

							if (outputlabel!=m_dsn->_landuse2[i*_cols+j]&&isPassing==true) 
							{

								double _disChangeFrom;
								_disChangeFrom=min__dis2goal[__label];

								double _disChangeTo;
								_disChangeTo=min__dis2goal[outputlabel-1];

								if (initialDist[outputlabel-1]>=0&&_disChangeTo==0) // 目标多于初始，达到目标后封顶，不让别的转成它，恢复最大惯性系数
								{
									temp[i*_cols+j]=m_dsn->_landuse2[i*_cols+j];
									adjustment_effect[outputlabel-1]=1;
									label1=true;
								}

								if (initialDist[__label]<=0&&_disChangeFrom==0) // 目标少于初始，达到目标后封底，不让它转成别人，恢复最小惯性系数
								{
									temp[i*_cols+j]=m_dsn->_landuse2[i*_cols+j];
									adjustment_effect[__label]=1;
									label2=true;
								}

								if (label1==false&&label2==false)
								{
									temp[i*_cols+j]=(unsigned char)outputlabel;
									saveCount[outputlabel-1]+=1;
									saveCount[__label]-=1;
									min__dis2goal[outputlabel-1]=goalNum[outputlabel-1]-saveCount[outputlabel-1];
									min__dis2goal[__label]=goalNum[__label]-saveCount[__label];
								}

							}
							else
							{
								temp[i*_cols+j]=m_dsn->_landuse2[i*_cols+j];
							}
							

						}
						else
						{
							temp[i*_cols+j]=m_dsn->_landuse2[i*_cols+j];
						}
					}
				}
			}

			for (int ii=0;ii<_classes;ii++)
			{
				saveCount[ii]=0;
			}

			for (int jj=0;jj<_rows*_cols;jj++)
			{
				m_dsn->_landuse2[jj]=temp[jj];
				for (int kk=0;kk<_classes;kk++)
				{
					if (temp[jj]==m_dsn->rgbLanduseType2[kk])
					{
						saveCount[kk]+=1;
						break;
					}
				}
				for (int ii=0;ii<3;ii++)
				{
					for (int hh=0;hh<_classes+1;hh++)
					{
						if (temp[jj]==0)
						{
							m_dsn->u_rgb[ii*_rows*_cols+jj]=255;
						}
						if (temp[jj]==hh)
						{
							m_dsn->u_rgb[ii*_rows*_cols+jj]=Colour[hh][ii];
						}
					}
				}
			}

			sendColsandRows(_cols,_rows,1+k);

			QString __send(tr("第 %1 次迭代各类像元个数,").arg(k+1));
			for (int ii=0;ii<_classes;ii++)
			{
				__send = __send + QString::number(saveCount[ii])+  tr(",");
			}
			sendparameter(__send.left(__send.length() - 1));

 			// 停止
			if (isbreak==1||k>=looptime-1)
			{
				break;
			}

			k++;

		}
	}

	end=GetTickCount();   

	double timecost=end-start;  

// 	QString sendInher(tr("最终惯性系数："));
// 	for (int ii=0;ii<_classes;ii++)
// 	{
// 		sendInher = sendInher + QString::number(adjustment_effect[ii])+  tr(" : ");
// 	}
// 	sendparameter(sendInher.left(sendInher.length() - 2));
	
	QString sendtime(tr("用时：%1 s\n正在保存结果...").arg(timecost/1000.0,1,'f',4));
	sendparameter(sendtime);

	saveResult();

	/// <清除一维指针>
	delete[] adjustment;
	delete[] dynaDist;
	delete[] adjustment_effect;
	delete[] initialDist;
	delete[] saveCount;
	delete[] probability;
	delete[] min__dis2goal;
	delete[] ra;
	delete[] raSum;
	delete[] goalNum;
	delete[] val;
	delete[] temp;
	/// <附:清除二维指针的方法>
	if (numWindows!=0)
	{
		for(int i=0;i<numWindows;i++)
		{delete []direction[i];}
		delete []direction;
	}

	for(int i=0;i<_classes;i++)
	{delete []t_filerestrict[i];}
	delete []t_filerestrict;

	for(int i=0;i<_classes;i++)
	{delete []t_filecost[i];}
	delete []t_filecost;

	for(int i=0;i<(_classes+1);i++)
	{delete []Colour[i];}
	delete []Colour;
}

void SimulationProcess::stoploop()
{
	isbreak=1;
}


