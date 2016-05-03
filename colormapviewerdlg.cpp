#include "colormapviewerdlg.h"
#include "ui_colormapviewerdlg.h"
#include <QFileDevice>
#include <QDir>
#include <QStringList>
#include "qwt_color_map.h"
#include <QPainter>
#include "qwt_interval.h"
#include <qsettings.h>
#include <QFileDialog>
#define BarLength 350
#define BarWidth 40
QString gpath;
colorMapViewerDlg::colorMapViewerDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::colorMapViewerDlg)
{
    QSettings set;
    gpath = set.value("colorMaps path", "./ColorMaps").toString();
    ui->setupUi(this);
    ui->path->setText(gpath);
    ui->listWidget->setViewMode(QListWidget::IconMode);

    ui->listWidget->setIconSize(QSize(BarLength,BarWidth));

    ui->listWidget->setResizeMode(QListWidget::Adjust);
    genList(gpath);

}

void colorMapViewerDlg::genList(QString path){
    QStringList nameFilter("*.cmp");
    QDir directory(path);
    QStringList cmpFiles = directory.entryList(nameFilter);

    foreach(QString s, cmpFiles){
        QPixmap p(BarLength,BarWidth);
        QwtLinearColorMap cm;
        QFile inputFile(path + "\\" +s);
        if (inputFile.open(QIODevice::ReadOnly))
        {
           QTextStream in(&inputFile);
           while (!in.atEnd())
           {
              QString line = in.readLine();
              QStringList parms = line.split(",");
              cm.addColorStop(parms[1].toDouble(), QColor(parms[2]));

           }
           inputFile.close();
           QPainter pp(&p);
           for (int i = 0; i < BarLength; ++i){
               pp.setPen(cm.color(QwtInterval(.001,BarLength+.001), i));
               pp.drawLine(QPointF(i,0), QPointF(i,BarWidth));
           }
           ui->listWidget->addItem(new QListWidgetItem(QIcon(p),s));
        }
    }

}

colorMapViewerDlg::~colorMapViewerDlg()
{
    delete ui;
}

void colorMapViewerDlg::on_browsePb_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(0, tr("Open Directory")
                                                    ,gpath,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (dir == "")
        return;
    gpath = dir;
    ui->path->setText(gpath);
    genList(gpath);
}

void colorMapViewerDlg::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    m_selection = ui->path->text() + "/" + item->text();
    on_buttonBox_accepted();
    done(1);
}

void colorMapViewerDlg::on_buttonBox_accepted()
{

}

void colorMapViewerDlg::on_aboutpb_clicked()
{

}