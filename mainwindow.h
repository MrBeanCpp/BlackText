#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QDragEnterEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QTime>
#include <QToolButton>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui;

    QString fileName;
    QString codeFormat = "UTF-8";
    QLabel* label_codef;
    QLabel* label_line;
    QToolButton* btn_run;
    QCheckBox* check_tempCpp;
    bool textChanged = false;
    const QString tempPath = QDir::tempPath() + "/Dark-Editor";
    //    const QString tempexe = QDir::toNativeSeparators(QDir::currentPath()) + "\\Temp-EXE.exe";
    //    const QString tempcpp = QDir::toNativeSeparators(QDir::currentPath()) + "\\Temp-CPP.cpp";
    //    const QString gccPathFile = QDir::toNativeSeparators(QDir::currentPath()) + "\\gccPath.txt";
    const QString tempexe = tempPath + "/Temp-EXE.exe";
    const QString tempcpp = tempPath + "/Temp-CPP.cpp";
    const QString gccPathFile = QApplication::applicationDirPath() + "/gccPath.txt";
    QString gccPath;

    const QString Title = "Dark TextEditor v2.26";

    const QString ERROR_CODEC = "ErrorCodec"; //无法识别编码

    QLineEdit* edit_search = nullptr;
    const QString iniFilePath = QApplication::applicationDirPath() + "/BlackText.ini";

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    //bool eventFilter(QObject* watched, QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:

private:
    void readText(QString& path); //重构单独类
    void saveText(QString& path);
    QString getCurrentUnicode(const QByteArray& bytes, int& UTF8invalid);
    void runText(bool isTemp = false);
    void setTextChanged(bool isChanged = true);
    void updateStatistic(void);
    void writeIniFile(const QString& key, const QVariant& value);
    void readIniFile(void);
    void runAfterWinDraw(void); //为了快速显示，延后某些耗时操作

public:
    friend class ShortCutKey;
};
#endif // MAINWINDOW_H
