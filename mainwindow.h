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
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTimer>
#include <QSet>
#include <util.h>
//#include <QLockFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    QString userid;
    QString username;
    QSettings settings;
    QNetworkAccessManager *manager;
    QNetworkRequest request;
    QJsonArray schedule;
    bool closing;
    QSystemTrayIcon *sysTrayIcon;
    QTimer *refreshTimer;
    QTimer *notificationTimer;
    QMap<int, QSet<int>> notificationFlags;
    QVector<NotificationSetting> notificationSettings;
//    QLockFile lockFile;

private slots:
    void openSettings();
    void redraw();
    void refreshSchedule();
    void processResponse(QNetworkReply *reply);
    void checkClassesAndNotify();
};
#endif // MAINWINDOW_H

