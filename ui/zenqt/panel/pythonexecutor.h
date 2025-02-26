#pragma once

#ifndef __PYTHON_EXECUTOR_H__
#define __PYTHON_EXECUTOR_H__

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>
#include <QTextEdit>

class PythonExecutePane : public QWidget
{
    Q_OBJECT
public:
    PythonExecutePane(QWidget* parent = nullptr);

private slots:
    void run();

private:
    QListWidget* listWidget;
    QStackedWidget* stackedWidget;
};


#endif