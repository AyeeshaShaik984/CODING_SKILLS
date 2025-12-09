#ifndef STARTDIALOG_H
#define STARTDIALOG_H

#include <QDialog>

class QLabel;
class QPushButton;

class StartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StartDialog(QWidget *parent = nullptr);
    ~StartDialog() override = default;

    bool isStudentChoice() const { return m_choice == 1; }
    bool isAdminChoice()   const { return m_choice == 2; }
    bool isGuestChoice()   const { return m_choice == 3; }

private slots:
    void onStudent();
    void onAdmin();
    void onGuest();

private:
    int m_choice = 0;
};

#endif // STARTDIALOG_H
