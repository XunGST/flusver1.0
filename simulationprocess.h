#ifndef SIMULATIONPROCESS_H
#define SIMULATIONPROCESS_H

#include <QObject>
#include <QList>
#include <vector>

class GeoDpCAsys;
class DynaSimulation;

class SimulationProcess : public QObject
{
	Q_OBJECT

public:
	SimulationProcess(DynaSimulation* _dsn);
	~SimulationProcess();

	template<class TT> bool testData();
	void testRestrict();

signals:
	void sendColsandRows(int,int,int);
	void sendparameter(QString);

public slots:
	void startloop();
	void runloop();
	void stoploop();

	QString getUiparaMat();
	void saveResult();


private:
	int _rows;
	int _cols;
	int _classes;
	int numWindows;
	int sizeWindows;
	int looptime;
	double degree;

	bool isRestrictExit;
	int isbreak;
	
	int* goalNum;
	
	int* min__dis2goal;

protected:
	int* saveCount;


	int* val;
	double* ra;
	double* raSum;
	double* probability;
	double* sProbability;
	double* normalProbability;

	unsigned char* temp;
	unsigned char* restrict;

	double** t_filerestrict;
	double** t_filecost;
	int** direction;
	short** Colour;

	QList<int>  priority_level;    // 土地利用类型
	QList<int>  priority_record;   // 对应等级
	QList<int>  tmp;               // 包含等级


private:
	DynaSimulation* m_dsn;
	GeoDpCAsys* m_gds;
	
};

#endif // SIMULATIONPROCESS_H
