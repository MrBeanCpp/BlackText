#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QMessageLogContext>
#include <QMutex>
#include <shortcutkey.h>
void outputMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg);
int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    //qInstallMessageHandler(outputMessage); //安装消息处理程序
    qDebug() << QTime::currentTime() << "start";
    QFile qss(":/style.qss");
    if (qss.open(QFile::ReadOnly)) {
        a.setStyleSheet(qss.readAll());
        qss.close();
    }
    MainWindow w;
    ShortCutKey shortcut(&w);
    a.installEventFilter(&shortcut);
    qDebug() << QTime::currentTime() << "before_show";
    w.show();
    qDebug() << QTime::currentTime() << "show";
    return a.exec();
}

void outputMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    static QMutex mutex;
    mutex.lock();
    QByteArray localMsg = msg.toLocal8Bit();
    QString text;
    switch (type) {
    case QtDebugMsg:
        text = QString("Debug:");
        break;
    case QtWarningMsg:
        text = QString("Warning:");
        break;
    case QtCriticalMsg:
        text = QString("Critical:");
        break;
    case QtFatalMsg:
        text = QString("Fatal:");
        break;
    default:
        text = QString("Debug:");
    }

    // 设置输出信息格式
    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line); // F文件L行数
    QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    //QString strMessage = QString("%1 %2 \t%3 \t%4").arg(text).arg(context_info).arg(strDateTime).arg(msg);
    QString strMessage = QString("%1 \t%2 \t%3").arg(text).arg(strDateTime).arg(msg);
    // 输出信息至文件中（读写、追加形式）
    //QFile file(QApplication::applicationDirPath() + QDateTime::currentDateTime().toString("yyyy-MM-dd").append("-log.txt"));
    QFile file(QApplication::applicationDirPath() + "/log.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream stream(&file);
    stream << strMessage << "\r\n";
    file.flush();
    file.close();
    // 解锁
    mutex.unlock();
}
