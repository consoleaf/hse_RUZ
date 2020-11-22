#include "mainwindow.h"
#include <QApplication>
#include <QSystemSemaphore>
#include <QSharedMemory>

int main(int argc, char * argv[]) {
    QApplication a(argc, argv);

    // Acquire the lock to prevent race condition with shared memory.
    QSystemSemaphore sema("9217c45a-7473-4f21-963b-57a5c883871a", 1);
    sema.acquire();

    #ifndef Q_OS_WIN32
    // on linux/unix shared memory is not freed upon crash
    // so if there is any trash from previous instance, clean it
    QSharedMemory nix_fix_shmem("927e9652-5fab-426d-9971-3c3c8b6437da");
    if (nix_fix_shmem.attach()) {
        nix_fix_shmem.detach();
    }
    #endif

    QSharedMemory shmem("927e9652-5fab-426d-9971-3c3c8b6437da");
    bool is_running;
    if (shmem.attach()) {
        is_running = true;
    } else {
        shmem.create(1);
        is_running = false;
    }
    sema.release(); // Release the lock, the check is complete

    if (is_running) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("You already have this app running."
            "\r\nOnly one instance is allowed.");
        msgBox.exec();
        return 1;
    }

    MainWindow w;
    w.show();
    return a.exec();
}
