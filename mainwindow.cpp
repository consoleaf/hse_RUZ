#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    manager = new QNetworkAccessManager();
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::openSettings);
    connect(manager, &QNetworkAccessManager::finished, this, &MainWindow::processResponse);
    this->redraw();
    this->refreshSchedule();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openSettings() {
    auto dialog = new SettingsDialog();
    connect(dialog, &SettingsDialog::accepted, this, &MainWindow::redraw);
    connect(dialog, &SettingsDialog::accepted, this, &MainWindow::refreshSchedule);
    dialog->open();
}

void MainWindow::refreshSchedule() {
    // Get the schedule for TODAY and TOMORROW
    QUrl url;
    url.setUrl("https://ruz.hse.ru/api/schedule/student/" + settings.value("userid").toString());
    QUrlQuery query;
    query.addQueryItem("lng", "1");
    QDateTime date = QDateTime::currentDateTime();
    query.addQueryItem("start", date.toString("yyyy.MM.dd"));
    query.addQueryItem("finish", date.addDays(7).toString("yyyy.MM.dd"));
    url.setQuery(query);
    request.setUrl(url);
    manager->get(request);
}

void MainWindow::redraw() {
    username = settings.value("username").toString();
    userid = settings.value("userid").toString();
    ui->usernameLabel->setText(username);

    auto *layout = ui->scrollAreaWidgetContents->layout();
    if (layout != nullptr || !layout->children().empty()) {
        qDeleteAll(layout->children());
        delete layout;
    }

    layout = new QVBoxLayout;

    QString prevDate;
    for (auto itemref : schedule) {
        auto item = itemref.toObject();
        if (prevDate != item["date"].toString()) {
            if (prevDate != "") {
                QFrame *line = new QFrame();
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Sunken);
                layout->addWidget(line);
            }
            prevDate = item["date"].toString();
            QLabel *dateLabel = new QLabel(prevDate);
            auto font = dateLabel->font();
            font.setBold(true);
            font.setPixelSize((int)(font.pixelSize() * 1.25));
            dateLabel->setFont(font);
            layout->addWidget(dateLabel);
        }

        QCommandLinkButton *entry = new QCommandLinkButton;
        entry->setFlat(true);
        entry->setText(item["discipline"].toString());
        entry->setDescription(item["beginLesson"].toString() + "-" + item["endLesson"].toString() + ". " +
                              item["kindOfWork"].toString() + "; " + item["auditorium"].toString());
        if (!item["url1"].isNull() || !item["url2"].isNull()) {
            QString url = (item["url1"].isNull() ? item["url2"] : item["url1"]).toString();
            connect(entry, &QCommandLinkButton::clicked, this, [=]() {
                QDesktopServices::openUrl(url);
            });
        } else {
            entry->setDisabled(true);
        }

        layout->addWidget(entry);
    }

    ui->scrollAreaWidgetContents->setLayout(layout);
}


void MainWindow::processResponse(QNetworkReply *reply) {
    if (reply->error()) return;
    QString answer = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(answer.toUtf8());
    schedule = document.array();
    redraw();
}
