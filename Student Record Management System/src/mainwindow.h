#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

class QLineEdit;
class QPushButton;
class QGroupBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // roleName: "admin", "student", "guest"
    // userName: login username (for student ID and tickets)
    void setRole(const QString &roleName, const QString &userName);

    // shows window, returns true if user pressed "Back to Main"
    bool execMain();

private slots:
    void addStudent();
    void editStudent();
    void deleteStudent();
    void raiseTicket();
    void viewTickets();
    void backToMain();

private:
    // widgets
    QLineEdit    *idInput;         // top "Enter Roll No" input
    QLineEdit    *nameEdit;
    QLineEdit    *rollnoEdit;
    QLineEdit    *courseEdit;
    QLineEdit    *gradeEdit;
    QPushButton  *addButton;
    QPushButton  *editButton;
    QPushButton  *deleteButton;
    QPushButton  *ticketButton;
    QPushButton  *viewTicketsButton;
    QPushButton  *backButton;
    QPushButton  *viewStudentBtn;  // for all roles
    QGroupBox    *formGroup;       // admin form group

    // db & state
    QSqlDatabase db;
    QString      m_role;
    QString      m_user;
    bool         m_backToMain = false;

    void setupUi();
    void setupDatabase();
};

#endif // MAINWINDOW_H
