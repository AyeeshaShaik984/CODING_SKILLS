#include "startdialog.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFont>

StartDialog::StartDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Welcome");
    resize(600, 400);
    setModal(true);

    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(40, 40, 40, 40);

    outer->addStretch();

    QWidget *card = new QWidget(this);
    card->setObjectName("startCard");
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(30, 30, 30, 30);
    cardLayout->setSpacing(16);
    cardLayout->setAlignment(Qt::AlignCenter);

    QLabel *title = new QLabel("Student Management System", card);
    QFont f = title->font();
    f.setPointSize(20);
    f.setBold(true);
    title->setFont(f);
    title->setAlignment(Qt::AlignCenter);

    QLabel *question = new QLabel("Who are you?", card);
    question->setAlignment(Qt::AlignCenter);

    QPushButton *studentBtn = new QPushButton("I am a Student", card);
    QPushButton *adminBtn   = new QPushButton("I am an Admin", card);
    QPushButton *guestBtn   = new QPushButton("I am a Guest", card);

    studentBtn->setMinimumHeight(46);
    adminBtn->setMinimumHeight(46);
    guestBtn->setMinimumHeight(46);

    cardLayout->addWidget(title);
    cardLayout->addWidget(question);
    cardLayout->addSpacing(10);
    cardLayout->addWidget(studentBtn);
    cardLayout->addWidget(adminBtn);
    cardLayout->addWidget(guestBtn);

    outer->addWidget(card, 0, Qt::AlignHCenter);
    outer->addStretch();

    connect(studentBtn, &QPushButton::clicked, this, &StartDialog::onStudent);
    connect(adminBtn,   &QPushButton::clicked, this, &StartDialog::onAdmin);
    connect(guestBtn,   &QPushButton::clicked, this, &StartDialog::onGuest);
}

void StartDialog::onStudent()
{
    m_choice = 1;
    accept();
}

void StartDialog::onAdmin()
{
    m_choice = 2;
    accept();
}

void StartDialog::onGuest()
{
    m_choice = 3;
    accept();
}
