#ifndef SHORTCUTKEY_H
#define SHORTCUTKEY_H

#include <QObject>
#include <mainwindow.h>
class ShortCutKey : public QObject
{
    Q_OBJECT
public:
    explicit ShortCutKey(QObject *parent = nullptr);
    ShortCutKey(MainWindow* Win, QObject* parent = nullptr);
signals:

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    MainWindow* win;
    Ui::MainWindow* winUi;
};

#endif // SHORTCUTKEY_H
