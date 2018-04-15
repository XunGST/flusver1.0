#include "geodpcasys.h"
#include <QtGui/QApplication>
#include <QtCore> 

int main(int argc, char *argv[])
{
	//set code for locale
	QTextCodec* codec =QTextCodec::codecForLocale();
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForTr(codec);

	QApplication app(argc, argv);
	GeoDpCAsys w;
	w.show();

	return app.exec();


}
