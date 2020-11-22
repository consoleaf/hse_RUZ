#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    manager = new QNetworkAccessManager();

    disconnect(ui->lineEdit, &QLineEdit::returnPressed, ui->buttonBox, &QDialogButtonBox::accepted);

    ui->comboBox->addItem(settings.value("username").toString());
    ui->lineEdit->setText(settings.value("username").toString());

    on_refreshListButton_clicked();

    connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
        if (reply->error())
            return;
        QString answer = reply->readAll();
        QJsonDocument response = QJsonDocument::fromJson(answer.toUtf8());
        QJsonArray array = response.array();

        ui->comboBox->clear();
        ids.clear();
        for (QJsonValueRef ref : array) {
            QJsonObject obj = ref.toObject();
            ui->comboBox->addItem(obj["label"].toString());
            ids << obj["id"].toString();
        }
    });
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
    delete manager;
}

void SettingsDialog::on_refreshListButton_clicked()
{
    QString name = ui->lineEdit->text();
    QUrl url = QUrl("https://ruz.hse.ru/api/search");
    url.setQuery("type=student&term=" + name);
    request.setUrl(url);
    manager->get(request);
}

void SettingsDialog::on_buttonBox_accepted()
{
    if (ui->comboBox->currentText() != settings.value("username").toString()) {
        QString new_id = ids[ui->comboBox->currentIndex()];
        settings.setValue("userid", new_id);
        settings.setValue("username", ui->comboBox->currentText());
    }
    accept();
}

void SettingsDialog::on_lineEdit_returnPressed()
{
    on_refreshListButton_clicked();
}
