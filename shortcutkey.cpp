#include "shortcutkey.h"
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <ui_mainwindow.h>
ShortCutKey::ShortCutKey(QObject *parent) : QObject(parent)
{

}

ShortCutKey::ShortCutKey(MainWindow* Win, QObject* parent)
    : QObject(parent)
    , win(Win)
    , winUi(Win->ui)
{
}

bool ShortCutKey::eventFilter(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched)
    if (event->type() != QEvent::KeyPress) return false;
    QKeyEvent* keyevent = static_cast<QKeyEvent*>(event);
    if (keyevent->isAutoRepeat()) return false;
    if (keyevent->modifiers() == Qt::ControlModifier && keyevent->key() == Qt::Key_S)
        win->saveText(win->fileName);
    else if (keyevent->modifiers() == Qt::ControlModifier && keyevent->key() == Qt::Key_F) {
        winUi->actionsearch->setChecked(!winUi->actionsearch->isChecked());
        winUi->actionsearch->triggered(winUi->actionsearch->isChecked());
        if (winUi->actionsearch->isChecked()) {
            win->edit_search->setFocus();
            win->edit_search->setText(winUi->textEdit->textCursor().selectedText());
        } else
            winUi->textEdit->setFocus();
    } else if (keyevent->modifiers() == Qt::ControlModifier && keyevent->key() == Qt::Key_R && win->btn_run->isEnabled())
        emit win->btn_run->clicked();
    else if (keyevent->modifiers() == Qt::ControlModifier && keyevent->key() == Qt::Key_T && win->check_tempCpp->isEnabled()) {
        win->check_tempCpp->setChecked(!win->check_tempCpp->isChecked());
        win->check_tempCpp->toggled(win->check_tempCpp->isChecked());
    } else
        return false;
    return true;//阻断传播，否则多次触发（收到多个控件的消息）
}
