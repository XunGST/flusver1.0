#ifndef MED_H
#define MED_H

#include <QWidget>
#include "ui_med.h"

class MorphEroDla;

class med : public QWidget
{
	Q_OBJECT

public:
	med(QWidget *parent = 0);
	~med();

public slots:
	void openInputImage();
	void saveOutputImage();
	void runDelineation();

private:
	Ui::med ui;
	MorphEroDla* mMed;
};

#endif // MED_H
