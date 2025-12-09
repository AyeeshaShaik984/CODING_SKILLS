#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog() override = default;

    QString currentUser() const { return m_user; }
    QString currentRole() const { return m_role; }

private slots:
    void onLogin();
    void onCancel();

private:
    QLineEdit   *m_userEdit;
    QLineEdit   *m_passEdit;
    QPushButton *m_loginButton;
    QPushButton *m_cancelButton;

    QString m_user;
    QString m_role;

    bool tryLogin(const QString &user, const QString &pass);
};

#endif // LOGINDIALOG_H
