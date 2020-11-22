#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) //,
//      sharedMemory("RUZ_schedule_app")
{
    ui->setupUi(this);
    manager = new QNetworkAccessManager();
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::openSettings);
    connect(manager, &QNetworkAccessManager::finished, this, &MainWindow::processResponse);

    closing = false;
    connect(ui->actionExit, &QAction::triggered, this, [=]() {
        closing = true;
        close();
    });

    // Make system tray menu
    auto trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(ui->actionExit);

    sysTrayIcon = new QSystemTrayIcon;
    sysTrayIcon->setContextMenu(trayIconMenu);
    sysTrayIcon->setIcon(QIcon(":/icons/icon.ico"));
    sysTrayIcon->show();

    connect(sysTrayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason)
    {
        if(reason == QSystemTrayIcon::Trigger)
        {
            if(isVisible())
            {
                hide();
            }
            else
            {
                refreshSchedule();
                show();
                activateWindow();
            }
        }
    });

    this->redraw();
    this->refreshSchedule();

    // Initiate background tasks
    refreshTimer = new QTimer(this);
    notificationTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &MainWindow::refreshSchedule);
    connect(notificationTimer, &QTimer::timeout, this, &MainWindow::checkClassesAndNotify);

    refreshTimer->start(5 * 60 * 1000); // 5 minutes
    notificationTimer->start(10 * 1000); // 10 seconds

    notificationSettings.append(NotificationSetting(10, "10 минут до пары!"));
    notificationSettings.append(NotificationSetting(5, "5 минут до пары!"));
    notificationSettings.append(NotificationSetting(5, "1 минута до пары!"));
    notificationSettings.append(NotificationSetting(0, "Пара начинается!"));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete sysTrayIcon;
    delete manager;
}

void MainWindow::openSettings() {
    auto dialog = new SettingsDialog();
    connect(dialog, &SettingsDialog::accepted, this, &MainWindow::redraw);
    connect(dialog, &SettingsDialog::accepted, this, &MainWindow::refreshSchedule);
    dialog->open();
}

void MainWindow::refreshSchedule() {
    // Get the schedule for TODAY and TOMORROW
    qDebug() << "Refreshing schedule...";
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
    QString today = QDateTime::currentDateTime().toString("yyyy.MM.dd");
    ui->usernameLabel->setText(username);

    auto *layout = ui->scrollAreaWidgetContents->layout();
    if (layout != nullptr || !layout->children().empty()) {
        qDeleteAll(layout->children());
        delete layout;
    }

    layout = new QVBoxLayout;

    // clear the system tray menu
    QMenu *trayMenu = sysTrayIcon->contextMenu();
    trayMenu->clear();

    QString prevDate;
    for (auto itemref : schedule) {
        auto item = itemref.toObject();

        // Add a date label if needed
        if (prevDate != item["date"].toString()) {
            if (prevDate != "") {
                QFrame *line = new QFrame();
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Sunken);
                layout->addWidget(line);
            }
            prevDate = item["date"].toString();
            QLabel *dateLabel = new QLabel;
            auto date = QDateTime::fromString(prevDate, "yyyy.MM.dd");
            dateLabel->setText(date.toString("yyyy.MM.dd - dddd"));
            auto font = dateLabel->font();
            font.setBold(true);
//            font.setPixelSize((int)((double)font.pixelSize() * 1.25));
            dateLabel->setFont(font);
            layout->addWidget(dateLabel);
        }

        // Make a button in the window
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

        QString discipline = item["discipline"].toString();
        if (discipline.length() > 15) {
            discipline.truncate(15);
            discipline.append("...");
        }

        // Make an entry in the system tray menu
        if (prevDate == today) {
            QAction *action = new QAction;
            action->setText(item["beginLesson"].toString() + "-" +
                    item["endLesson"].toString() + ". " + discipline);
            if (!item["url1"].isNull() || !item["url2"].isNull()) {
                QString url = (item["url1"].isNull() ? item["url2"] : item["url1"]).toString();
                connect(action, &QAction::triggered, this, [=]() {
                    QDesktopServices::openUrl(url);
                });
            } else {
                action->setDisabled(true);
            }
            trayMenu->addAction(action);
        }
    }

    trayMenu->addAction(ui->actionExit);
    auto verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Maximum, QSizePolicy::Expanding);
    layout->addItem(verticalSpacer);
    ui->scrollAreaWidgetContents->setLayout(layout);
}


void MainWindow::processResponse(QNetworkReply *reply) {
    if (reply->error()) return;
    QString answer = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(answer.toUtf8());
    schedule = document.array();
    redraw();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (closing)
    {
        event->accept();
        exit(0);
    }
    else
    {
        this->hide();
        event->ignore();
    }
}

void MainWindow::checkClassesAndNotify() {
    QDateTime now = QDateTime::currentDateTime();
    for (auto itemref : schedule) {
        auto item = itemref.toObject();
        if (now.toString("yyyy.MM.dd") == item["date"].toString()) {
            auto beginTime = QTime::fromString(item["beginLesson"].toString(), "HH:mm");
            auto timeLeft = now.time().secsTo(beginTime) / 60;
            int oid = item["lessonOid"].toInt();

            qDebug() << "Time left till " << item["discipline"].toString() << ": " << timeLeft << " minutes";

            if (timeLeft < 0 || timeLeft > 10) {
                notificationFlags.remove(oid);
                continue; // No more processing if the class is not relevant
            }

            QString discipline = item["discipline"].toString();
            if (discipline.length() > 30) {
                discipline.truncate(30);
                discipline.append("...");
            }

            for (auto setting : notificationSettings) {
                if (timeLeft <= setting.timeLeft && !notificationFlags[oid].contains(setting.timeLeft)) {
                    notificationFlags[oid].insert(setting.timeLeft);
                    sysTrayIcon->showMessage(setting.notificationTitle,
                                             QString("%1-%2: %3")
                                             .arg(item["beginLesson"].toString(),
                                             item["endLesson"].toString(), discipline));
                }
            }
        }
    }
}
