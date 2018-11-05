#include "geodpcasys.h"
#include <QtGui/QApplication>
#include <QtCore> 

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	//set code for locale
	QTextCodec* codec =QTextCodec::codecForLocale();
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForTr(codec);

	GeoDpCAsys w;
	w.show();
	return a.exec();
}
