#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QWebEngineProfile>
#include <vector>
#include <QWebEngineView>
#include <QTimer>
#include <QMessageBox>
#include <QTextEdit>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QMenu>

class AIPage;

class CustomTextEdit : public QTextEdit {
    Q_OBJECT

public:
    CustomTextEdit(QWidget *parent = nullptr) : QTextEdit(parent) {

    }

signals:
    void SendMsg(const QString& msg);

protected:
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Return) {
            if (event->modifiers() & Qt::ShiftModifier) {
                // Shift + Enter: 换行
                QTextEdit::keyPressEvent(event);
            } else {
                // Enter: 弹出消息框
                //QMessageBox::information(this, "提示", "你按下了 Enter 键！");
                emit SendMsg(this->toPlainText());
                this->clear();
            }
        } else {
            // 处理其他按键
            QTextEdit::keyPressEvent(event);
        }
    }
};


class AIPage : public QWebEnginePage
{
    Q_OBJECT
public:
    explicit AIPage(QWebEngineProfile* profile = nullptr,QWebEngineView *parent = nullptr)
        :QWebEnginePage(profile,parent),
          mProfile(profile),
          mWebView(parent){

    }

    void LaunchPage(){
        //QWebEnginePage* pPage = new QWebEnginePage(mProfile,mWebView);
        mWebView->setPage(this);
        mWebView->load(QUrl(mUrl));
        mWebView->setFocus();

        QObject::connect(mWebView, &QWebEngineView::loadFinished, [this] {
            AddTimer();
        }); 
    }

    void DisableArkDebug(){
        QString disable_antidebug = R"( //禁止反调试
            let end = setTimeout(function () { }, 10000);
            for (let i = 1; i <= end; i++) {
                clearTimeout(i);
            })";

        this->runJavaScript(disable_antidebug);
    }

    void SendMsg(const QString &msg){
        if(!mReady){
            return;
        }

        if(!mPrejs.isEmpty()){
            runJavaScript(mPrejs);
        }

        QString send_msg = msg;
        QTimer::singleShot(100, [this,send_msg] {
             this->runJavaScript(mEditjs.arg(send_msg));
        });

        QTimer::singleShot(500, [this] {
             this->runJavaScript(mButtonjs);
        });
    }

    void AddTimer(){
        if(mTimer == nullptr){
            mTimer = new QTimer(this);
            mTimer->setSingleShot(false); // 设置为重复定时器
            mTimer->setInterval(1000);
            connect(mTimer, &QTimer::timeout, this, &AIPage::OnTime);
        }
        mTimer->start(); // 启动定时器
    }

protected slots:
    void OnTime(){
        runJavaScript(mCheckReadyjs);
    }

protected:
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                          const QString &message,
                                          int lineNumber,
                                          const QString &sourceID) override {
        if (message.contains("***page ready***") ) {
            mReady = true;
            if(mTimer != nullptr){
                mTimer->stop();
            }
        }
        return QWebEnginePage::javaScriptConsoleMessage(level,message,lineNumber,sourceID);
    }

public:
    QString mName;
    QString mUrl;
    QString mCheckReadyjs;
    QString mPrejs;
    QString mEditjs;
    QString mButtonjs;
    bool    mReady = false;
    QWebEngineProfile* mProfile = nullptr;
    QWebEngineView* mWebView = nullptr;
    QTimer* mTimer = nullptr;
    QDockWidget* mDockWidget = nullptr;
};

class MyWebView : public QWebEngineView
{
    Q_OBJECT
public:
    MyWebView(QWidget* parent = nullptr, QWebEngineProfile* profile = nullptr) :
        QWebEngineView(parent)
        , m_profile(profile) {

        connect(this->page(), &QWebEnginePage::linkHovered, this, &MyWebView::linkHovered);
    }

protected:
    QWebEngineView* createWindow(QWebEnginePage::WebWindowType type) override {
        QWebEnginePage* pPage2 = new AIPage(m_profile, this);
        this->setPage(pPage2);
        this->load(now_url);
        this->setFocus();

        return this;
    }

    void contextMenuEvent(QContextMenuEvent* event) override {
        QMenu contextMenu(this);

        // 添加自定义菜单项
        QAction* copyAction = contextMenu.addAction(tr("&Reload"));
        connect(copyAction, &QAction::triggered, this, &MyWebView::ReloadUrl);

        // 显示自定义的上下文菜单
        contextMenu.exec(event->globalPos());

        // 阻止默认的上下文菜单显示
        event->accept();
    }

public slots:
    void linkHovered(const QString& url)
    {
        now_url = url;
    }

    void ReloadUrl() {
        auto pAiPage = qobject_cast<AIPage*>(this->page());
        pAiPage->load(pAiPage->mUrl);
    }

protected:
    QUrl now_url;
    QWebEngineProfile* m_profile = nullptr;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum{
        kInputFloat_PageTable,
        kInputEmbed_PageEmed,
    };

protected:
    void InitPageData();
    void AddMenu();
    void AddToolBar();
    void InitProfile();
    void AddWebView();
    void SaveCurrentStatus();
    void ResizeDockWidget();

private slots:
    void OnExit();
    void OnSendMsg(const QString &msg);
    void OnToolBarShow();
    void OnResetView();
    void OnChangeDockStyle();

protected:
    QWebEngineProfile* mProfile = nullptr;
    std::vector<AIPage*> mPageList;
    QToolBar *mToolBar = nullptr;
    QAction *mToolbarAction = nullptr;
    QAction* mFloatAction = nullptr;
    QDockWidget* mInputDockWidget = nullptr;
    int mPageStyle = kInputEmbed_PageEmed;
};
#endif // MAINWINDOW_H
