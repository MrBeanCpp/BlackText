#include "codeeditor.h"
#include "pairkey.h"
#include <QDebug>
#include <QPainter>
#include <QTextBlock>
#include<QScrollBar>
CodeEditor::CodeEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    setFont(QFont("Consolas", 14));
    //setTabStopDistance(4 * fontMetrics().horizontalAdvance(' ')); //Tab宽度
    setAcceptDrops(false); //否则触发textEditor事件导致主窗口接收不到
    verticalScrollBar()->installEventFilter(this);
    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateLineBarWidth);
    connect(this, &QPlainTextEdit::updateRequest, [=]() {
        if (LineBar != nullptr)
            LineBar->update(); //repaint死循环
    });
}

void CodeEditor::setLineBar(QWidget* lineBar)
{
    LineBar = lineBar;
    LineBar->installEventFilter(this);
}

void CodeEditor::drawLineBar(QPaintEvent* event)
{
    QPainter painter(LineBar);
    painter.fillRect(event->rect(), QColor(60, 60, 60));

    QTextBlock block = firstVisibleBlock();
    int line = block.blockNumber() + 1;
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top() + 1); //蓝色边框1px偏差
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && block.isVisible() && top <= event->rect().bottom()) {
        painter.setPen(QColor(120, 120, 120));
        painter.setFont(font());
        painter.drawText(0, top, LineBar->width(), fontMetrics().height(), Qt::AlignRight, QString::number(line));

        line++;
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
    }
}

void CodeEditor::updateLineBarWidth()
{
    if (LineBar != nullptr)
        LineBar->setFixedWidth((QString::number(blockCount()).length() + 1) * fontMetrics().horizontalAdvance('9'));
}

void CodeEditor::zoom(int delta)
{
    delta > 0 ? zoomIn() : zoomOut();
    updateLineBarWidth();
}

bool CodeEditor::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == LineBar && event->type() == QEvent::Paint) //必须要在QPaintEvent里绘图!!!!!!!!!!!!!!!
        drawLineBar(static_cast<QPaintEvent*>(event));
    else if(watched==verticalScrollBar() && event->type()==QEvent::Wheel){
        QWheelEvent* wheelevent = static_cast<QWheelEvent*>(event);
        if (wheelevent->modifiers() == Qt::ControlModifier) {
            zoom(wheelevent->delta());
            return true; //阻止事件继续传递（给滚动条）
        }
    }
    return QPlainTextEdit::eventFilter(watched, event);
}

void CodeEditor::keyPressEvent(QKeyEvent* event)
{
    static int Last_Key;
    static bool isLeftPair = false;
    if (PairKey::find(Last_Key) && (PairKey::getRight(Last_Key) == event->key() || event->key() == Qt::Key_Backspace) && isLeftPair) {
        if (PairKey::getRight(Last_Key) == event->key()) {
            moveCursor(QTextCursor::NextCharacter);
        } else if (event->key() == Qt::Key_Backspace) {
            moveCursor(QTextCursor::NextCharacter);
            textCursor().deletePreviousChar();
            textCursor().deletePreviousChar();
        }
        isLeftPair = false;
    } else if (PairKey::find(event->key())) {
        insertPlainText(PairKey::getStr(event->key()));
        moveCursor(QTextCursor::PreviousCharacter);
        isLeftPair = true;
    } else if (event->key() == Qt::Key_Tab) {
        insertPlainText("    ");//4*' '
    } else
        QPlainTextEdit::keyPressEvent(event);
    Last_Key = event->key();
}
