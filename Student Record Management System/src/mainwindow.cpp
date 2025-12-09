#include "mainwindow.h"

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QDir>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QEventLoop>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    setupDatabase();
}

MainWindow::~MainWindow()
{
    if (db.isOpen())
        db.close();
}

bool MainWindow::execMain()
{
    show();
    m_backToMain = false;

    QEventLoop loop;
    connect(this, &QWidget::destroyed, &loop, &QEventLoop::quit);
    connect(backButton, &QPushButton::clicked, &loop, &QEventLoop::quit);
    loop.exec();
    return m_backToMain;
}

void MainWindow::setupUi()
{
    setWindowTitle("Student Record Manager");
    resize(900, 540);
    setMinimumSize(800, 500);                    // similar compact width[web:125]

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(14);

    // ---------------- Top controls (all roles) ----------------
    QVBoxLayout *studentLayout = new QVBoxLayout;
    studentLayout->setAlignment(Qt::AlignHCenter);
    studentLayout->setSpacing(10);

    // Line 1: label + input + view
    QHBoxLayout *line1 = new QHBoxLayout;
    line1->setSpacing(10);
    QLabel *idLabel = new QLabel("Enter Roll No:", this);
    idInput = new QLineEdit(this);
    idInput->setMinimumHeight(40);
    idInput->setAlignment(Qt::AlignCenter);
    viewStudentBtn = new QPushButton("View Details", this);
    line1->addWidget(idLabel);
    line1->addWidget(idInput);
    line1->addWidget(viewStudentBtn);
    studentLayout->addLayout(line1);

    // Line 2: raise ticket (for student role only)
    QHBoxLayout *line2 = new QHBoxLayout;
    line2->setAlignment(Qt::AlignHCenter);
    ticketButton = new QPushButton("Raise Ticket", this);
    line2->addWidget(ticketButton);
    studentLayout->addLayout(line2);

    // Line 3: view tickets (for student + admin)
    QHBoxLayout *line3 = new QHBoxLayout;
    line3->setAlignment(Qt::AlignHCenter);
    viewTicketsButton = new QPushButton("View Tickets", this);
    line3->addWidget(viewTicketsButton);
    studentLayout->addLayout(line3);

    mainLayout->addLayout(studentLayout);

    // ---------------- Admin-only form group ----------------
    formGroup = new QGroupBox("Admin: Edit Student Details", this);
    QGridLayout *formLayout = new QGridLayout(formGroup);
    formLayout->setHorizontalSpacing(12);
    formLayout->setVerticalSpacing(10);
    formLayout->setContentsMargins(10, 10, 10, 10);   // padding[web:100][web:138]

    QLabel *nameLabel   = new QLabel("Name:", formGroup);
    QLabel *rollLabel   = new QLabel("Roll No:", formGroup);
    QLabel *courseLabel = new QLabel("Course:", formGroup);
    QLabel *gradeLabel  = new QLabel("Grade:", formGroup);

    nameEdit   = new QLineEdit(formGroup);
    rollnoEdit = new QLineEdit(formGroup);
    courseEdit = new QLineEdit(formGroup);
    gradeEdit  = new QLineEdit(formGroup);

    // tall inputs so text is visible
    nameEdit->setMinimumHeight(60);
    rollnoEdit->setMinimumHeight(60);
    courseEdit->setMinimumHeight(60);
    gradeEdit->setMinimumHeight(60);
    nameEdit->setMinimumWidth(300);
    rollnoEdit->setMinimumWidth(300);
    courseEdit->setMinimumWidth(300);
    gradeEdit->setMinimumWidth(300);

    QFont f = nameEdit->font();
    f.setPointSize(11);
    nameEdit->setFont(f);
    rollnoEdit->setFont(f);
    courseEdit->setFont(f);
    gradeEdit->setFont(f);

    nameEdit->setAlignment(Qt::AlignCenter);
    rollnoEdit->setAlignment(Qt::AlignCenter);
    courseEdit->setAlignment(Qt::AlignCenter);
    gradeEdit->setAlignment(Qt::AlignCenter);

    formLayout->addWidget(nameLabel,   0, 0);
    formLayout->addWidget(nameEdit,    0, 1);
    formLayout->addWidget(rollLabel,   1, 0);
    formLayout->addWidget(rollnoEdit,  1, 1);
    formLayout->addWidget(courseLabel, 2, 0);
    formLayout->addWidget(courseEdit,  2, 1);
    formLayout->addWidget(gradeLabel,  3, 0);
    formLayout->addWidget(gradeEdit,   3, 1);

    mainLayout->addWidget(formGroup);

    // ---------------- Buttons row ----------------
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);

    addButton    = new QPushButton("Add", this);
    editButton   = new QPushButton("Update", this);
    deleteButton = new QPushButton("Delete", this);
    backButton   = new QPushButton("Back to Main", this);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(backButton);
    mainLayout->addLayout(buttonLayout);

    // Keyboard Enter moves through admin fields
    connect(nameEdit,   &QLineEdit::returnPressed, this, [this]() { rollnoEdit->setFocus(); });
    connect(rollnoEdit, &QLineEdit::returnPressed, this, [this]() { courseEdit->setFocus(); });
    connect(courseEdit, &QLineEdit::returnPressed, this, [this]() { gradeEdit->setFocus(); });
    connect(gradeEdit,  &QLineEdit::returnPressed, this, [this]() { addButton->setFocus(); });

    // common connections
    connect(addButton,         &QPushButton::clicked,   this, &MainWindow::addStudent);
    connect(editButton,        &QPushButton::clicked,   this, &MainWindow::editStudent);
    connect(deleteButton,      &QPushButton::clicked,   this, &MainWindow::deleteStudent);
    connect(ticketButton,      &QPushButton::clicked,   this, &MainWindow::raiseTicket);
    connect(viewTicketsButton, &QPushButton::clicked,   this, &MainWindow::viewTickets);
    connect(backButton,        &QPushButton::clicked,   this, &MainWindow::backToMain);

    // View Details: all roles use DB lookup, no table
    connect(viewStudentBtn, &QPushButton::clicked, this, [this]() {
        QString roll = idInput->text().trimmed();
        if (roll.isEmpty()) {
            QMessageBox::information(this, "Input", "Enter a roll number.");
            return;
        }

        if (!db.isOpen())
            return;

        QSqlQuery query;
        query.prepare("SELECT name, course, grade FROM students WHERE rollno = :r");
        query.bindValue(":r", roll);
        if (!query.exec()) {
            QMessageBox::warning(this, "Database Error",
                                 "Failed to load student:\n" + query.lastError().text());
            return;
        }

        if (!query.next()) {
            QMessageBox::information(this, "Not found",
                                     "No student found with that roll number.");
            return;
        }

        QString info = "Name: "   + query.value(0).toString() + "\n" +
                       "Course: " + query.value(1).toString() + "\n" +
                       "Grade: "  + query.value(2).toString();
        QMessageBox::information(this, "Student Details", info);
    });
}

void MainWindow::setRole(const QString &roleName, const QString &userName)
{
    m_role = roleName;
    m_user = userName;

    // default: disable everything
    addButton->setEnabled(false);
    editButton->setEnabled(false);
    deleteButton->setEnabled(false);
    ticketButton->setEnabled(false);
    viewTicketsButton->setEnabled(false);
    idInput->setEnabled(false);
    viewStudentBtn->setEnabled(false);

    // default: show widgets
    addButton->show();
    editButton->show();
    deleteButton->show();
    ticketButton->show();
    viewTicketsButton->show();
    formGroup->show();

    if (m_role == "admin") {
        // admin: form CRUD + ticket viewer; can also use roll-number view
        addButton->setEnabled(true);
        editButton->setEnabled(true);
        deleteButton->setEnabled(true);
        viewTicketsButton->setEnabled(true);

        ticketButton->hide();          // admin does not raise tickets

        idInput->setEnabled(true);
        viewStudentBtn->setEnabled(true);

        formGroup->show();
    } else if (m_role == "student") {
        // student: ID + View + Raise Ticket + View Tickets, no admin form or CRUD
        idInput->setEnabled(true);
        viewStudentBtn->setEnabled(true);
        ticketButton->setEnabled(true);
        viewTicketsButton->setEnabled(true);

        formGroup->hide();
        addButton->hide();
        editButton->hide();
        deleteButton->hide();
    } else if (m_role == "guest") {
        // guest: only ID + View + Back
        idInput->setEnabled(true);
        viewStudentBtn->setEnabled(true);

        formGroup->hide();
        addButton->hide();
        editButton->hide();
        deleteButton->hide();
        ticketButton->hide();
        viewTicketsButton->hide();
    }
}

void MainWindow::setupDatabase()
{
    QDir dir(QCoreApplication::applicationDirPath());
    if (!dir.exists("data")) {
        dir.mkdir("data");
    }
    const QString dbPath = dir.filePath("data/students.db");

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        QMessageBox::critical(this, "Database Error",
                              "Failed to open database:\n" + db.lastError().text());
        return;
    }

    QSqlQuery query;
    const QString createSql =
        "CREATE TABLE IF NOT EXISTS students ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " name   TEXT NOT NULL,"
        " rollno TEXT NOT NULL,"
        " course TEXT,"
        " grade  TEXT"
        ")";
    if (!query.exec(createSql)) {
        QMessageBox::critical(this, "Database Error",
                              "Failed to create table:\n" + query.lastError().text());
    }
}

void MainWindow::addStudent()
{
    if (m_role != "admin")
        return;

    const QString name   = nameEdit->text().trimmed();
    const QString rollno = rollnoEdit->text().trimmed();
    const QString course = courseEdit->text().trimmed();
    const QString grade  = gradeEdit->text().trimmed();

    if (name.isEmpty() || rollno.isEmpty() || course.isEmpty() || grade.isEmpty()) {
        QMessageBox::information(this, "Validation",
                                 "Please fill all fields before adding.");
        return;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO students (name, rollno, course, grade) "
        "VALUES (:name, :rollno, :course, :grade)");
    query.bindValue(":name",   name);
    query.bindValue(":rollno", rollno);
    query.bindValue(":course", course);
    query.bindValue(":grade",  grade);

    if (!query.exec()) {
        QMessageBox::warning(this, "Database Error",
                             "Failed to add student:\n" + query.lastError().text());
        return;
    }

    QMessageBox::information(this, "Success", "Student added to database.");

    nameEdit->clear();
    rollnoEdit->clear();
    courseEdit->clear();
    gradeEdit->clear();
}

void MainWindow::editStudent()
{
    if (m_role != "admin")
        return;

    const QString rollno = rollnoEdit->text().trimmed();
    const QString name   = nameEdit->text().trimmed();
    const QString course = courseEdit->text().trimmed();
    const QString grade  = gradeEdit->text().trimmed();

    if (rollno.isEmpty()) {
        QMessageBox::information(this, "Update Student",
                                 "Enter Roll No to update.");
        return;
    }

    QSqlQuery query;
    query.prepare(
        "UPDATE students "
        "SET name = :name, course = :course, grade = :grade "
        "WHERE rollno = :roll");
    query.bindValue(":name",   name);
    query.bindValue(":course", course);
    query.bindValue(":grade",  grade);
    query.bindValue(":roll",   rollno);

    if (!query.exec()) {
        QMessageBox::warning(this, "Database Error",
                             "Failed to update student:\n" + query.lastError().text());
        return;
    }

    if (query.numRowsAffected() == 0) {
        QMessageBox::information(this, "Update Student",
                                 "No student found with that Roll No.");
    } else {
        QMessageBox::information(this, "Update Student",
                                 "Student record updated.");
    }
}

void MainWindow::deleteStudent()
{
    if (m_role != "admin")
        return;

    const QString rollno = rollnoEdit->text().trimmed();

    if (rollno.isEmpty()) {
        QMessageBox::information(this, "Delete Student",
                                 "Enter the Roll No of the student to delete.");
        return;
    }

    const auto reply = QMessageBox::question(
        this,
        "Confirm Delete",
        "Delete all records with this Roll No?"
    );
    if (reply != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.prepare("DELETE FROM students WHERE rollno = :roll");
    query.bindValue(":roll", rollno);

    if (!query.exec()) {
        QMessageBox::warning(this, "Database Error",
                             "Failed to delete student:\n" + query.lastError().text());
        return;
    }

    if (query.numRowsAffected() == 0) {
        QMessageBox::information(this, "Delete Student",
                                 "No student found with that Roll No.");
    } else {
        QMessageBox::information(this, "Delete Student",
                                 "Student record(s) deleted.");
    }

    rollnoEdit->clear();
}

void MainWindow::raiseTicket()
{
    if (m_role != "student") {
        QMessageBox::information(this, "Tickets",
                                 "Only students can raise tickets.");
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle("Raise Ticket");
    dlg.resize(420, 260);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    QLabel *title = new QLabel("Describe the issue with your record", &dlg);
    QFont f = title->font();
    f.setPointSize(12);
    f.setBold(true);
    title->setFont(f);

    QLineEdit *subjectEdit = new QLineEdit(&dlg);
    subjectEdit->setPlaceholderText("Subject (e.g., Wrong marks, ID mismatch)");

    QLineEdit *messageEdit = new QLineEdit(&dlg);
    messageEdit->setPlaceholderText("Short description");

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *okBtn = new QPushButton("Submit Ticket", &dlg);
    QPushButton *cancelBtn = new QPushButton("Cancel", &dlg);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    layout->addWidget(title);
    layout->addWidget(subjectEdit);
    layout->addWidget(messageEdit);
    layout->addLayout(btnLayout);

    connect(okBtn,     &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted)
        return;

    const QString subject = subjectEdit->text().trimmed();
    const QString message = messageEdit->text().trimmed();
    if (subject.isEmpty() || message.isEmpty()) {
        QMessageBox::information(this, "Tickets",
                                 "Subject and description are required.");
        return;
    }

    QString path = QCoreApplication::applicationDirPath() + "/tickets.txt";
    QFile file(path);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        QMessageBox::warning(this, "Tickets", "Cannot open tickets.txt");
        return;
    }

    static int nextId = 1;
    QTextStream out(&file);
    out << nextId++ << ' ' << m_user << ' '
        << subject.simplified() << ' '
        << message.simplified() << " open\n";

    QMessageBox::information(this, "Tickets", "Ticket submitted.");
}

void MainWindow::viewTickets()
{
    QString path = QCoreApplication::applicationDirPath() + "/tickets.txt";

    if (m_role == "admin") {
        QFile file(path);
        if (!file.exists()) {
            QMessageBox::information(this, "Tickets", "No tickets found.");
            return;
        }
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Tickets", "Cannot open tickets.txt");
            return;
        }

        QTextStream in(&file);
        QStringList lines;
        while (!in.atEnd())
            lines << in.readLine();
        file.close();

        if (lines.isEmpty()) {
            QMessageBox::information(this, "Tickets", "No tickets found.");
            return;
        }

        // Show all tickets with indices so admin can type 0, 1, ...
        QString all;
        for (int i = 0; i < lines.size(); ++i)
            all += QString::number(i) + ": " + lines[i] + "\n";

        bool ok = false;
        int idx = QInputDialog::getInt(
            this,
            "Tickets",
            "Tickets:\n\n" + all +
            "\nEnter index (e.g. 0 or 1) to mark as resolved (-1 to cancel):",
            -1, -1, lines.size() - 1, 1, &ok
        );
        if (!ok || idx < 0)
            return;

        QString line = lines[idx];
        // expect format: id user subject issue status
        QStringList parts = line.split(' ');
        if (parts.size() >= 4) {
            if (parts.size() == 4) {
                parts << "cleared";
            } else {
                parts[4] = "cleared";
            }
            line = parts.join(' ');
            lines[idx] = line;

            if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                QMessageBox::warning(this, "Tickets", "Cannot update tickets.txt");
                return;
            }
            QTextStream out(&file);
            for (const QString &l : lines)
                out << l << '\n';
            file.close();
        }

        QMessageBox::information(this, "Tickets", "Ticket marked as issue resolved.");
        return;
    }

    if (m_role != "student") {
        QMessageBox::information(this, "Tickets",
                                 "Only students and admins can view tickets.");
        return;
    }

    QFile file(path);
    if (!file.exists()) {
        QMessageBox::information(this, "Tickets", "No ticket has been raised.");
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Tickets", "Cannot open tickets.txt");
        return;
    }

    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;
        const QStringList parts = line.split(' ');
        if (parts.size() < 4)
            continue;
        if (parts[1] == m_user)
            lines << line;
    }
    file.close();

    if (lines.isEmpty()) {
        QMessageBox::information(this, "Tickets", "No ticket has been raised.");
        return;
    }

    QString details;
    for (const QString &l : lines) {
        const QStringList p = l.split(' ');
        QString id    = p[0];
        QString subj  = p[2];
        QString issue = p[3];
        QString status = (p.size() > 4 ? p[4] : "open");

        details += "Ticket ID: " + id + "\n";
        details += "Subject: " + subj + "\n";
        details += "Issue: " + issue + "\n";
        details += "Status: " + status + "\n\n";
    }

    QMessageBox::information(this, "Your Tickets", details);
}

void MainWindow::backToMain()
{
    m_backToMain = true;
    close();
}
