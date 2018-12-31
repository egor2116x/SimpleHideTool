#ifndef SIMPLEHIDETOOLTESTQT_UI_H
#define SIMPLEHIDETOOLTESTQT_UI_H

#include <QtWidgets/QMainWindow>
#include "ui_simplehidetooltestqt_ui.h"

class SimpleHideToolTestQT_UI : public QMainWindow
{
    Q_OBJECT

public:
    SimpleHideToolTestQT_UI(QWidget *parent = 0);
    ~SimpleHideToolTestQT_UI();

private:
    Ui::SimpleHideToolTestQT_UIClass ui;
private slots:
    
void InjectSlot();
void InjectIntoUserSessionSlot();
void InstallLibs();
void UninstallLibs();
void SetProcessList();
void GetProcessList();

};

#endif // SIMPLEHIDETOOLTESTQT_UI_H
