#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QtNetwork/QNetworkAccessManager>
#include <QSettings>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void on_refreshListButton_clicked();
    void on_buttonBox_accepted();
    void on_lineEdit_returnPressed();

private:
    Ui::SettingsDialog *ui;
    QNetworkAccessManager *manager;
    QNetworkRequest request;
    QVector<QString> ids;
    QSettings settings;
};

#endif // SETTINGSDIALOG_H
