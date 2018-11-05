#ifndef GLOBAL_DEFINE_H
#define GLOBAL_DEFINE_H

#include <QWidget>
#include "ui_geosimulator.h"
#include <QtCore>
#include <QList>
#include "TiffDataRead.h"

extern QList<TiffDataRead*> lau_poDataset;

extern QList<TiffDataRead*> msk_poDataset;

extern QList<TiffDataRead*> div_poDataset;

extern QList<TiffDataRead*> con_poDataset;

extern QList<TiffDataRead*> ajt_poDataset;

extern QList<short> rgbLanduseType;


#endif // MYFIRSTLIB_GLOBAL_H