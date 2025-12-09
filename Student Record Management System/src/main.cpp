#include <QApplication>
#include <QFile>
#include "mainwindow.h"
#include "startdialog.h"
#include "logindialog.h"

static void loadStyleSheet(QApplication &app)
{
    QFile styleFile(":/styles/stylesheet.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        const QString style = QString::fromUtf8(styleFile.readAll());
        app.setStyleSheet(style);
        styleFile.close();
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    loadStyleSheet(app);

    while (true) {
        StartDialog start;
        if (start.exec() != QDialog::Accepted)
            break;

        if (start.isStudentChoice()) {
            LoginDialog login;
            if (login.exec() != QDialog::Accepted)
                continue;
            if (login.currentRole() != "student")
                continue;

            MainWindow w;
            w.setRole("student", login.currentUser());
            if (!w.execMain())
                break;
        } else if (start.isAdminChoice()) {
            LoginDialog login;
            if (login.exec() != QDialog::Accepted)
                continue;
            if (login.currentRole() != "admin")
                continue;

            MainWindow w;
            w.setRole("admin", login.currentUser());
            if (!w.execMain())
                break;
        } else { // guest
            LoginDialog login;
            if (login.exec() != QDialog::Accepted)
                continue;
            if (login.currentRole() != "guest")
                continue;

            MainWindow w;
            w.setRole("guest", login.currentUser());
            if (!w.execMain())
                break;
        }
    }

    return 0;
}
