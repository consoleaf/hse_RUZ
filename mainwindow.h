#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <settingsdialog.h>
#include <iostream>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QCommandLinkButton>
#include <QDesktopServices>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString userid;
    QString username;
    QSettings settings;
    QNetworkAccessManager *manager;
    QNetworkRequest request;
    QJsonArray schedule;

private slots:
    void openSettings();
    void redraw();
    void refreshSchedule();
    void processResponse(QNetworkReply *reply);
};
#endif // MAINWINDOW_H
