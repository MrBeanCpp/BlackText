#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCheckBox>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFontDialog>
#include <QFontMetrics>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QPaintEvent>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QStyle>
#include <QTemporaryFile>
#include <QTextBlock>
#include <QTextCodec>
#include <QTextCursor>
#include <QTextStream>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QWheelEvent>
#include <QtConcurrent>
#include <pairkey.h>
#include <windows.h>
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << QTime::currentTime() << "MainWindow_s";
    //QTime startTime = QTime::currentTime();
    ui->setupUi(this);
    setAcceptDrops(true);
    setWindowTitle(Title);

    ui->textEdit->setLineBar(ui->lineBar); //
    ui->textEdit->updateLineBarWidth();
    //ui->textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth); //*

    label_codef = new QLabel("Noting", this); //addWidget在界面绘制后进行（因为过于耗时）
    label_line = new QLabel(this);

    edit_search = new QLineEdit(this); //为了在shotcutkey.h中控制 拓展作用域（已经稀碎了，不愧是第一个Qt作品，[2021/9/1]）
    QPushButton* btn_search = new QPushButton(this);
    btn_run = new QToolButton(this);
    check_tempCpp = new QCheckBox(this);
    edit_search->setFixedWidth(150);
    edit_search->setPlaceholderText("...");
    btn_search->setFixedSize(60, 25);
    btn_search->setText("Find");
    btn_run->setIcon(style()->standardIcon(QStyle::SP_MediaPlay)); //
    btn_run->setText("Run");
    btn_run->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    check_tempCpp->setText("TempCpp");
    ui->toolBar->addWidget(edit_search);
    ui->toolBar->addWidget(btn_search);
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(btn_run);
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(check_tempCpp);
    ui->toolBar->hide();

    //↓憨批，早就移交具体部件执行了，听了个寂寞
    //ui->textEdit->installEventFilter(this); //↓此言不得之，键盘事件还得监听
    //ui->textEdit->verticalScrollBar()->installEventFilter(this); //监控滚动条即可
    //edit_search->installEventFilter(this);
    //check_tempCpp->installEventFilter(this);

    QDir().mkdir(tempPath);
    if (QFile::exists(gccPathFile)) {
        QFile file(gccPathFile);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream text(&file);
            gccPath = text.readLine();
            file.close();
            if (!QFile::exists(gccPath))
                gccPath = "";
        }
    }
    if (gccPath == "") {
        btn_run->setEnabled(false);
        check_tempCpp->setEnabled(false);
    }
    connect(ui->actionGccPath, &QAction::triggered, [=](bool checked) { //文件管理类重构！！！！！！！！！！！！！！！！！
        Q_UNUSED(checked);
        QString path = QFileDialog::getOpenFileName(this, "Choose Gcc/G++ Path", gccPath == "" ? "" : gccPath, "(*.exe)");
        if (path != "")
            gccPath = path;
        else
            return;
        QFile file(gccPathFile);
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            QTextStream text(&file);
            text << gccPath;
            file.close();
        }
        btn_run->setEnabled(true);
        check_tempCpp->setEnabled(true);
    });

    QTimer::singleShot(0, [=]() { //等待界面绘制后执行
        runAfterWinDraw();
    });

    updateStatistic();
    connect(ui->textEdit, &QPlainTextEdit::blockCountChanged, this, &MainWindow::updateStatistic);
    connect(ui->textEdit, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatistic);

    connect(ui->textEdit, &CodeEditor::textChanged, [=]() { setTextChanged(); });

    connect(edit_search, &QLineEdit::returnPressed, [=]() {
        emit btn_search->clicked();
    });
    connect(btn_search, &QPushButton::clicked, [=]() {
        static QString text;
        if (edit_search->text() == "") {
            QMessageBox::information(this, "Info", "\"...\"Empty!");
            return;
        }
        if (text != edit_search->text())
            ui->textEdit->moveCursor(QTextCursor::Start);
        text = edit_search->text();
        if (!ui->textEdit->find(edit_search->text())) {
            QMessageBox::information(this, "Info", "Not Find Text");
            ui->textEdit->moveCursor(QTextCursor::Start);
        }
    });

    connect(btn_run, &QToolButton::clicked, [=]() {
        runText(check_tempCpp->isChecked());
    });

    connect(check_tempCpp, &QCheckBox::toggled, [=](bool checked) {
        if (fileName != "" && fileName != tempcpp) {
            check_tempCpp->setChecked(false);
            return;
        }
        fileName = checked ? tempcpp : "";
        setTextChanged(true); //fileName改变，文本对应地址改变，所有大概也算...吧
    });

    connect(ui->actionsearch, &QAction::triggered, [=](bool checked) {
        checked ? ui->toolBar->show() : ui->toolBar->hide();
    });

    connect(ui->action, &QAction::triggered, [=]() { //另存为
        QString temp = fileName;
        fileName = "";
        saveText(fileName);
        if (fileName == "")
            fileName = temp;
        else if (fileName != tempcpp)
            check_tempCpp->setChecked(false);
    });

    connect(ui->actionAutoReturn, &QAction::triggered, [=](bool checked) {
        ui->textEdit->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    });
    connect(ui->actionAlwaysTop, &QAction::triggered, [=](bool checked) { //setWindowFlag后窗口hide()需要重新show()
        SetWindowPos(HWND(winId()), checked ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    });
    connect(ui->actionFont, &QAction::triggered, [=]() {
        bool ok;
        QFont font = QFontDialog::getFont(&ok, ui->textEdit->font(), this); //添加parent，否则parent置顶，覆盖对话框
        if (ok) {
            ui->textEdit->setFont(font);
            writeIniFile("font", font);
        }
    });
    connect(ui->actionAbout, &QAction::triggered, [=]() {
        QMessageBox::about(this, "关于 BlackText", "BlackText\n"
                                                   "© MrBeanC。保留所有权利。\n"
                                                   "Bilibili @蔡姬MrBeanC\n"
                                                   "最后更新于：" __DATE__ "  [" __TIME__ "]\n\n"
                                                   "该产品的初衷是解决Windows记事本 纯白背景闪瞎眼的问题（希望官方可以加入Dark Mode）\n"
                                                   "相信大家注意到了 该产品综合中、日、英 三种语言 完美的解决了本地化问题[doge]\n\n"
                                                   "已ちょっと优化加载速度与大文件读取\n"
                                                   "但由于技术限制 本产品在性能方面\"稍逊\"Windows记事本 这是不争的事实 恳请谅解");
    });

    //QMessageBox::information(this, "", QString::number(startTime.msecsTo(QTime::currentTime())));
    qDebug() << QTime::currentTime() << "MainWindow_end";
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
    else
        event->ignore();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* data = event->mimeData();
    fileName = data->urls().at(0).toLocalFile();
    fileName.replace('\\', '/');
    readText(fileName);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event);
    bool canClose = ((ui->textEdit->toPlainText() == "" && fileName == "") || (textChanged == false && fileName != "") || fileName == tempcpp);
    if (!canClose) {
        showNormal();
        SwitchToThisWindow(HWND(winId()), true); //保证弹出窗口时，前置
        QMessageBox::StandardButton button = QMessageBox::question(this, "真面目な問題", "Save or Not?", QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel);
        switch (button) {
        case QMessageBox::Save:
            saveText(fileName);
            break;
        case QMessageBox::No:
            break;
        case QMessageBox::Cancel:
            event->ignore();
            return;
        default:
            return;
        }
    }
    hide(); //保证视觉上快速响应
    if (!this->isMaximized())
        writeIniFile("size", this->size());
}

void MainWindow::readText(QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;

    setWindowTitle(Title + " - " + path.mid(path.lastIndexOf('/') + 1));
    setTextChanged(false);
    //ui->textEdit->hide(); //避免更新界面时候的显示消耗CPU资源！！！！！！！！！！！！！//但是为了快速回显，采用分段processEvents
    ui->textEdit->disconnect(SIGNAL(textChanged())); //暂时解除信号连接 防止标题+'*' //blockSingals会阻塞所有信号 导致LineBar不能重绘
    ui->textEdit->clear();
    int UTF8invalid;
    codeFormat = "UTF-8";
    QString text;
    bool firstAppend = true;
    while (!file.atEnd()) {
        QByteArray bytes;
        for (int i = 0; i < 1024 && !file.atEnd(); i++) //读取成块数据，优化速度
            bytes.push_back(file.readLine()); //防止read造成中文字符截断
        text = getCurrentUnicode(bytes, UTF8invalid);
        if (text == ERROR_CODEC) {
            ui->textEdit->clear();
            break;
        }
        //ui->textEdit->insertPlainText(text); //append会换行 //insert会在指针处插入 用户点击后造成混乱
        if (text.endsWith("\r\n") && !file.atEnd()) text.chop(2); //去除回车
        ui->textEdit->appendPlainText(text); //自动换行
        if (firstAppend) {
            ui->textEdit->verticalScrollBar()->setSliderPosition(0); //第一次append会滚动到末尾 需要调整
            firstAppend = false;
        }
        ui->textEdit->updateLineBarWidth();
        updateStatistic();
        qApp->processEvents();
        if (UTF8invalid > 0) codeFormat = "GBK";
    }
    file.close();
    if (text == ERROR_CODEC) {
        QMessageBox::critical(this, "ErrorCodec", "Can not recognize the Text Code Format");
        setWindowTitle(Title);
        path = "";
        codeFormat = "Nothing";
    }
    label_codef->setText(codeFormat);
    connect(ui->textEdit, &CodeEditor::textChanged, [=]() { setTextChanged(); }); //恢复信号连接
}

void MainWindow::saveText(QString& path) //引用，直接改fileName
{
    if ((ui->textEdit->toPlainText() == "" && path == "") || (textChanged == false && path != "")) //画真值表，写逻辑表达式，极其精简，可读性为0
        return;
    if (ui->textEdit->toPlainText() != "" && path == "")
        path = QFileDialog::getSaveFileName(this, "Choose Save Path", "", "(*.txt *.cpp)");
    if (path == "")
        return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream text(&file);
    text.setCodec("UTF-8"); //输出统一UTF-8
    codeFormat = "UTF-8";
    text << ui->textEdit->toPlainText();
    label_codef->setText(codeFormat);
    ui->statusBar->showMessage("Saved!", 2500);
    file.close();
    if (path != tempcpp)
        setWindowTitle(Title + " - " + path.mid(path.lastIndexOf('/') + 1));
    setTextChanged(false);
}

QString MainWindow::getCurrentUnicode(const QByteArray& bytes, int& UTF8invalid)
{
    QTextCodec::ConverterState state_UTF8;
    QTextCodec::ConverterState state_GBK;
    QString text = QTextCodec::codecForName("UTF-8")->toUnicode(bytes.constData(), bytes.size(), &state_UTF8);
    if ((UTF8invalid = state_UTF8.invalidChars) > 0) { //测试有无无效字符出现
        text = QTextCodec::codecForName("GBK")->toUnicode(bytes.constData(), bytes.size(), &state_GBK);
        if (state_GBK.invalidChars > 0)
            return ERROR_CODEC; //无法识别编码
    }
    return text;
}

void MainWindow::runText(bool isTemp)
{
    saveText(fileName);
    if (fileName == "")
        return;
    if (gccPath == "") {
        QMessageBox::critical(this, "Error", "No Gcc/G++ founded");
        return;
    }
    //static const QString gccPath = "\"C:\\Program Files (x86)\\Dev-Cpp\\MinGW64\\bin\\g++\"";
    QString exePath = fileName.mid(0, fileName.lastIndexOf('.')) + ".exe";
    if (isTemp)
        exePath = tempexe;
    QProcess process(this);
    process.start("cmd.exe");
    process.write(QDir::toNativeSeparators('\"' + gccPath + '\"' + ' ' + '\"' + fileName + '\"' + " -o " + '\"' + exePath + '\"' + '\n').toLocal8Bit().constData()); //toLocal8Bit()!
    process.write("exit\n"); //\n模拟回车 触发指令
    process.waitForFinished();
    QString Error = QString::fromLocal8Bit(process.readAllStandardError()); //从本地字符集转Unicode
    process.close();
    if (Error != "")
        QMessageBox::critical(this, "Error", Error);
    else {
        QMessageBox::information(this, "Info", "Compile Successful!");
        //system(QDir::toNativeSeparators('\"' + exePath + '\"').toLocal8Bit().constData()); //toLocal8Bit()!
        //ShellExecuteA(0, "open", "cmd.exe", QDir::toNativeSeparators("cmd /c " + '\"' + exePath + '\"').toLocal8Bit().constData(), "", SW_SHOWNORMAL);
        ShellExecuteA(NULL, "open", QDir::toNativeSeparators('\"' + exePath + '\"').toLocal8Bit().constData(), "", "", SW_SHOWNORMAL);
        //指定父窗口，否则不显示(貌似没有父窗口也没事？)
    }
}

void MainWindow::setTextChanged(bool isChanged)
{
    const QString title = windowTitle();
    if (isChanged) {
        textChanged = true;
        if (title.at(0) != '*')
            setWindowTitle('*' + title);
    } else {
        textChanged = false;
        if (title.at(0) == '*')
            setWindowTitle(windowTitle().mid(1));
    }
}

void MainWindow::updateStatistic()
{
    const QTextCursor& textcursor = ui->textEdit->textCursor();
    label_line->setText(
        "SumLines:" + QString::number(ui->textEdit->document()->lineCount()) +
        " Line:" + QString::number(textcursor.blockNumber() + 1) +
        " Col:" + QString::number(textcursor.columnNumber())); //blockCount()是段落数（自动换行不算）
}

void MainWindow::writeIniFile(const QString& key, const QVariant& value)
{
    qDebug() << "writeIniFile";
    QSettings iniFile(iniFilePath, QSettings::IniFormat);
    iniFile.setValue(key, value);
}

void MainWindow::readIniFile()
{
    qDebug() << "readIniFile";
    QSettings iniFile(iniFilePath, QSettings::IniFormat);

    QSize size = iniFile.value("size", QSize(1280, 720)).toSize();
    this->resize(size);

    if (iniFile.contains("font")) {
        QFont font = iniFile.value("font").value<QFont>();
        ui->textEdit->setFont(font);
    }
}

void MainWindow::runAfterWinDraw()
{
    qDebug() << QTime::currentTime() << "runAfterWinDraw_s";
    //1 此时 界面绘制 但白屏
    ui->statusBar->addPermanentWidget(label_codef);
    ui->statusBar->addWidget(label_line); //↑很神奇 但这两句addWidget非常耗时(400ms)

    readIniFile(); //保证界面快速加载
    qApp->processEvents(); //保证绘制完全，防止闪白屏 //resize之后再processEvent 防止readText耗时 导致resize滞后
    //2 此时 由于processEvents 界面绘制完整 呈现准色
    QStringList arg = qApp->arguments();
    if (arg.count() > 1) {
        fileName = arg[1];
        fileName.replace('\\', '/');
        readText(fileName);
    }
}
