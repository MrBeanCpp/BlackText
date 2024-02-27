#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>
#include <mainwindow.h>
class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget* parent = nullptr);
    void setLineBar(QWidget* lineBar);
    void drawLineBar(QPaintEvent* event);
    void updateLineBarWidth(void);
    void zoom(int delta);

private:
    QWidget* LineBar = nullptr;
signals:

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent* event) override;
};

#endif // CODEEDITOR_H
