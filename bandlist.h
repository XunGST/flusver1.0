#ifndef BANDLIST_H
#define BANDLIST_H

#include <QTreeView>
#include <QStandardItemModel>
#include "TiffDataRead.h"

class GeoDpCAsys;
class DynaSimulation;

class BandList : public QTreeView
{
	Q_OBJECT

public:
	BandList(QWidget *parent);
	~BandList();

public slots:
	bool getBandList(QList<TiffDataRead*> _inUi_poDataset, int _serialnum);
	bool getOtherListModel(QList<TiffDataRead*> _inUi_poDataset, int _serialnum);
	bool showlabel(DynaSimulation* _dsn);
	/// <summary>
	/// <返回文件列表数据模型>
	/// </summary>
	/// <returns>文件列表数据模型.</returns>
	QStandardItemModel* FileListModel()
	{
		return fileListModel;
	};
	/// <summary>
	/// <设置fileListModel图像文件列表数据模型>
	/// </summary>
	/// <paramname="model">文件列表数据模型.</param>
	void SetFileListModel( QStandardItemModel* model)
	{
		this->fileListModel = model;
	};

	QStandardItem* getRootItem(); 

private:
	QStandardItemModel *fileListModel;
	QStandardItem *rootItem;
	QStandardItem *childItem;
};

#endif // BANDLIST_H
