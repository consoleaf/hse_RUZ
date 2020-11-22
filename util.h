#ifndef UTIL_H
#define UTIL_H

#include <QString>

struct NotificationSetting {
    int timeLeft;
    QString notificationTitle;

    NotificationSetting(int timeLeft, QString title) {
        this->timeLeft = timeLeft;
        notificationTitle = title;
    }
};

#endif // UTIL_H
