#include "logindialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Login");
    resize(320, 200);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *userLabel = new QLabel("Username (ID):", this);
    m_userEdit = new QLineEdit(this);

    QLabel *passLabel = new QLabel("Password:", this);
    m_passEdit = new QLineEdit(this);
    m_passEdit->setEchoMode(QLineEdit::Password);

    mainLayout->addWidget(userLabel);
    mainLayout->addWidget(m_userEdit);
    mainLayout->addWidget(passLabel);
    mainLayout->addWidget(m_passEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    m_loginButton  = new QPushButton("Login", this);
    m_cancelButton = new QPushButton("Cancel", this);
    btnLayout->addStretch();
    btnLayout->addWidget(m_loginButton);
    btnLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(btnLayout);

    connect(m_loginButton,  &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(m_cancelButton, &QPushButton::clicked, this, &LoginDialog::onCancel);
}

bool LoginDialog::tryLogin(const QString &user, const QString &pass)
{
    QString path = QCoreApplication::applicationDirPath() + "/credentials.txt";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "credentials.txt missing!");
        return false;
    }

    QTextStream in(&file);
    QString u, p, r;
    while (!in.atEnd()) {
        in >> u >> p >> r;
        if (u.isEmpty())
            continue;
        if (u == user && p == pass) {
            m_user = u;
            m_role = r;
            return true;
        }
    }
    return false;
}

void LoginDialog::onLogin()
{
    QString user = m_userEdit->text().trimmed();
    QString pass = m_passEdit->text().trimmed();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::information(this, "Validation", "Enter username and password.");
        return;
    }
    if (!tryLogin(user, pass)) {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password.");
        return;
    }
    accept();
}

void LoginDialog::onCancel()
{
    reject();
}
