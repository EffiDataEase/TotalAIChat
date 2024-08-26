#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTextBrowser>
#include <QMenuBar>
#include <QToolBar>
#include <QStyle>
#include <QDockWidget>
#include <QListWidget>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>
#include <QTimer>
#include <QSettings>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("AI Chat");
    AddMenu();
    AddToolBar();
    InitProfile();
    InitPageData();
    AddWebView();
    ResizeDockWidget();
    SaveCurrentStatus();
}


MainWindow::~MainWindow()
{
    QSettings settings("EffiDataEase", "AIChat");
    settings.setValue("LastState", this->saveState(1));
}

void MainWindow::SaveCurrentStatus(){
    QSettings settings("EffiDataEase", "AIChat");
    restoreState(settings.value("LastState").toByteArray(),1);
}

void MainWindow::AddToolBar(){
    // 创建一个工具栏
    mToolBar = new QToolBar("工具栏", this);
    mToolBar->setFloatable(true);
    mToolBar->setMovable(true);

    QIcon restoreIcon = style()->standardIcon(QStyle::SP_DialogResetButton);
    QIcon exitIcon = style()->standardIcon(QStyle::SP_DialogCloseButton);

    // 创建动作 (Action)
    QAction *exitAction = new QAction(exitIcon, "&Close", this);
    QAction *restoreAction = new QAction(restoreIcon, "&Restore", this);

    // 将动作连接到槽函数
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);
    connect(restoreAction, &QAction::triggered, this, &MainWindow::OnResetView);

    // 将动作添加到工具栏
    mToolBar->addAction(restoreAction);
    mToolBar->addAction(exitAction);

    // 将工具栏添加到主窗口
    addToolBar(mToolBar);

    mToolBar->setObjectName("toolbar");
    mToolBar->hide();
}

void MainWindow::AddMenu(){
    // 创建菜单栏
    QMenuBar *menuBar = new QMenuBar(this);

    // 创建 "File" 菜单
    QMenu *fileMenu = new QMenu("&文件", this);
    QAction *exitAction = new QAction("&退出", this);
    exitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    connect(exitAction, &QAction::triggered, this, &MainWindow::OnExit);
    fileMenu->addAction(exitAction);
    menuBar->addMenu(fileMenu);

    QMenu *viewMenu = new QMenu("&视图", this);

    mToolbarAction = new QAction("&显示工具栏", this);
    connect(mToolbarAction, &QAction::triggered, this, &MainWindow::OnToolBarShow);
    viewMenu->addAction(mToolbarAction);

    auto reset_action = new QAction("&重置视图", this);
    connect(reset_action, &QAction::triggered, this, &MainWindow::OnResetView);
    viewMenu->addAction(reset_action);


    mFloatAction = new QAction("&悬浮", this);
    connect(mFloatAction, &QAction::triggered, this, &MainWindow::OnChangeDockStyle);
    viewMenu->addAction(mFloatAction);

    menuBar->addMenu(viewMenu);

    // 设置窗口的菜单栏
    setMenuBar(menuBar);
}


void MainWindow::InitPageData(){
    auto page = new AIPage(mProfile,new MyWebView(this));
    page->mName = "天工AI";
    page->mUrl = "https://www.tiangong.cn/chat/universal/016";
    page->mCheckReadyjs = R"({ var visibleElement = document.querySelector('.el-textarea__inner');
                            if (visibleElement) {
                              console.log('***page ready***');
                            } })";
    page->mPrejs = "";
    page->mEditjs = R"({ var textarea = document.querySelector('.el-textarea__inner');
                     textarea.value="%1";
                     textarea.dispatchEvent(new Event('input', { bubbles: true }));})";
    page->mButtonjs = R"({ const button = document.querySelector('[class*="sendBtn"]');
                    button.click();})";
    mPageList.push_back(page);



    page = new AIPage(mProfile,new MyWebView(this));
    page->mName = "腾讯元宝";
    page->mUrl = "https://yuanbao.tencent.com/";
    page->mPrejs = "";
    page->mCheckReadyjs = R"({ var visibleElement = document.querySelector('[class="ql-editor ql-blank"]');
                            if (visibleElement) {
                              console.log('***page ready***');
                            }})";
    page->mEditjs = R"({ var visibleElement = document.querySelector('[class="ql-editor ql-blank"]');
                    var dT = new DataTransfer();
                    var evt = new ClipboardEvent('paste', {clipboardData: dT});
                    evt.clipboardData.setData('text/plain', '%1');
                    visibleElement.dispatchEvent(evt);})";
    page->mButtonjs = R"({ const button = document.querySelector('.style__send-btn___GVH0r');
                    button.click();})";
    mPageList.push_back(page);

    page = new AIPage(mProfile,new MyWebView(this));
    page->mName = "Kimi";
    page->mUrl = "https://kimi.moonshot.cn/";
    page->mPrejs = "";
    page->mCheckReadyjs = R"({var visibleElement = document.querySelector('div.editorContentEditable___FZJd9');
                            if (visibleElement) {
                              console.log('***page ready***');
                            }})";
    page->mEditjs = R"({var text_div = document.querySelector('div.editorContentEditable___FZJd9');
                    text_div.focus();

                    var dT = null;
                    try{
                        dT = new DataTransfer();
                    } catch(e){

                    }
                    var evt = new ClipboardEvent('paste', {clipboardData: dT});
                    evt.clipboardData.setData('text/plain', '%1');
                    text_div.dispatchEvent(evt);})";
    page->mButtonjs = R"({ const button = document.querySelector('#send-button');
                            if (button) {
                              button.click();
                            } else {
                              alert("not faund")}})";
    mPageList.push_back(page);

    page = new AIPage(mProfile,new MyWebView(this));
    page->mName = "通义千问";
    page->mUrl = "https://tongyi.aliyun.com/qianwen/";
    page->mPrejs = "";
    page->mCheckReadyjs = R"({ var visibleElement = document.querySelector('.ant-input.css-1r287do.ant-input-outlined.textarea--g7EUvnQR.fadeIn--_9l6unkO');
                            if (visibleElement) {
                              console.log('***page ready***');
                            }})";
    page->mEditjs = R"({ var textarea = document.querySelector('.ant-input.css-1r287do.ant-input-outlined.textarea--g7EUvnQR.fadeIn--_9l6unkO');
                     function setNativeValue(element, value) {
                       const valueSetter = Object.getOwnPropertyDescriptor(element, 'value').set;
                       const prototype = Object.getPrototypeOf(element);
                       const prototypeValueSetter = Object.getOwnPropertyDescriptor(prototype, 'value').set;

                       if (valueSetter && valueSetter !== prototypeValueSetter) {
                         prototypeValueSetter.call(element, value);
                       } else {
                         valueSetter.call(element, value);
                       }
                     }
                     setNativeValue(textarea,"%1");
                     textarea.dispatchEvent(new Event('input', { bubbles: true }));})";
    page->mButtonjs = R"({const button = document.querySelector('.operateBtn--zFx6rSR0');
                    button.click();})";
    mPageList.push_back(page);

    page = new AIPage(mProfile,new MyWebView(this));
    page->mName = "豆包";
    page->mUrl = "https://www.doubao.com/chat/";
    page->mPrejs = "";
    page->mCheckReadyjs = R"({ var visibleElement = document.querySelector('textarea[data-testid="chat_input_input"]');
                            if (visibleElement) {
                              console.log('***page ready***');
                            }})";
    page->mEditjs = R"({ var textarea = document.querySelector('textarea[data-testid="chat_input_input"]');
                     function setNativeValue(element, value) {
                       const valueSetter = Object.getOwnPropertyDescriptor(element, 'value').set;
                       const prototype = Object.getPrototypeOf(element);
                       const prototypeValueSetter = Object.getOwnPropertyDescriptor(prototype, 'value').set;

                       if (valueSetter && valueSetter !== prototypeValueSetter) {
                         prototypeValueSetter.call(element, value);
                       } else {
                         valueSetter.call(element, value);
                       }
                     }
                     setNativeValue(textarea,"%1");
                     textarea.dispatchEvent(new Event('input', { bubbles: true })); })";

    page->mButtonjs = R"({ var button = document.getElementById('flow-end-msg-send');
                     button.click();})";
    mPageList.push_back(page);

    page = new AIPage(mProfile,new MyWebView(this));
    page->mName = "360";
    page->mUrl = "https://chat.360.com";
    page->mPrejs = "";
    page->mCheckReadyjs = R"({ var visibleElement = document.querySelector('div[contenteditable="true"]');
                            if (visibleElement) {
                              console.log('***page ready***');
                            } })";
    page->mEditjs = R"({ var visibleElement = document.querySelector('div[contenteditable="true"]');
                    var dT = new DataTransfer();
                    var evt = new ClipboardEvent('paste', {clipboardData: dT});
                    evt.clipboardData.setData('text/plain', '%1');
                    visibleElement.dispatchEvent(evt);})";

    page->mButtonjs = R"({ const buttons = document.getElementsByClassName('p-8px');
                    const buttonElement = Array.from(buttons).find(btn => btn.classList.contains('rounded-6px') && btn.classList.contains('bg-[#0E6CF2]'));
                    buttonElement.click();})";
    mPageList.push_back(page);

    page = new AIPage(mProfile,new MyWebView(this));
    page->mName = "讯飞";
    page->mUrl = "https://xinghuo.xfyun.cn/desk";
    page->mCheckReadyjs = R"({ var visibleElement = document.querySelector('#ask-window > div.ask-window_small_size_flag__4zwJQ');
                            if (visibleElement) {
                                console.log('***page ready***');
                            }})";
    page->mPrejs = R"({ var visibleElement = document.querySelector('#ask-window > div.ask-window_small_size_flag__4zwJQ');
                    visibleElement.click();})";
    page->mEditjs = R"({ var visibleElement = document.querySelector('.ask-window_ask_window__tgBcr textarea');
                        visibleElement.value = "%1";})";
    page->mButtonjs = R"({ var visibleElement = document.querySelector('#ask_window_send_btn');
                    visibleElement.click();})";
    mPageList.push_back(page);

    page = new AIPage(mProfile,new MyWebView(this));
    page->mName = "文心一言";
    page->mUrl = "https://yiyan.baidu.com/";
    page->mPrejs = "";
    page->mCheckReadyjs = R"({
                                var visibleElement = document.querySelector('div.yc-editor');
                                if (visibleElement) {
                                    console.log('***page ready***');
                                }
                            })";
    page->mEditjs = R"({
                            var text_div = document.querySelector('div.yc-editor');
                            text_div.focus();
                            var dT = null;
                            try{
                                dT = new DataTransfer();
                            } catch(e){

                            }
                            var evt = new ClipboardEvent('paste', {clipboardData: dT});
                            evt.clipboardData.setData('text/plain', '%1');
                            text_div.dispatchEvent(evt);
                        })";
    page->mButtonjs =  R"(
                        {
                            var spanElement = document.querySelector('span.VAtmtpqL');
                            if (spanElement){
                                spanElement.click();
                            }
                        })";
    mPageList.push_back(page);

}

void MainWindow::ResizeDockWidget() {
    for (int i = 0; i < mPageList.size(); i++) {
        auto dock = mPageList[i]->mDockWidget;
        if (mPageStyle == kInputEmbed_PageEmed) {
            if (i % 3 == 0) {
                addDockWidget(Qt::LeftDockWidgetArea, dock);
            }
            else {
                splitDockWidget(mPageList[i - 1]->mDockWidget, dock, Qt::Horizontal);
            }
        }
        else {
            if (i == 0) {
                addDockWidget(Qt::LeftDockWidgetArea, dock);
            }
            else {
                tabifyDockWidget(mPageList[i - 1]->mDockWidget, dock);
            }
        }
    }

    if (mPageStyle == kInputEmbed_PageEmed) {
        mInputDockWidget->setFloating(false);
        splitDockWidget(mPageList[6]->mDockWidget, mInputDockWidget, Qt::Horizontal);

        QList<QDockWidget*> docks;
        docks << mPageList[0]->mDockWidget << mPageList[3]->mDockWidget << mPageList[6]->mDockWidget;
        QList<int> sizes;
        sizes << height() / 3 << height() / 3 << height() / 3;
        resizeDocks(docks, sizes, Qt::Vertical);

        docks.clear();
        sizes.clear();
        docks << mPageList[6]->mDockWidget << mInputDockWidget << mPageList[7]->mDockWidget;
        sizes << width() * 2 / 5 << width() / 5 << width() * 2 / 5;
        resizeDocks(docks, sizes, Qt::Horizontal);

    }
    else {
        mInputDockWidget->setFloating(true);
    }
}

void MainWindow::AddWebView(){

     for(int i=0;i<mPageList.size();i++){
         QDockWidget *dock = new QDockWidget(mPageList[i]->mName, this);
         dock->setObjectName(mPageList[i]->mName);
         mPageList[i]->mDockWidget = dock;
         mPageList[i]->LaunchPage();
         dock->setWidget(mPageList[i]->mWebView);
     }

     QString input_name = "输入";
     mInputDockWidget = new QDockWidget(input_name, this);
     auto pEdit = new CustomTextEdit(mInputDockWidget);
     mInputDockWidget->setWidget(pEdit);
     mInputDockWidget->setObjectName(input_name);
     QObject::connect(pEdit,&CustomTextEdit::SendMsg,this,&MainWindow::OnSendMsg);
}

void MainWindow::OnExit(){
    this->close();
}
void MainWindow::OnToolBarShow(){
    if(mToolBar->isHidden()){
        mToolBar->show();
        mToolbarAction->setText("&隐藏工具栏");
    }else{
        mToolBar->hide();
        mToolbarAction->setText("&显示工具栏");
    }
}
void MainWindow::OnResetView(){
    QSettings settings("EffiDataEase", "AIChat");
    restoreState(settings.value("InitState").toByteArray(),0);
}

void MainWindow::OnChangeDockStyle() {
    if (mPageStyle == kInputEmbed_PageEmed) {
        mPageStyle = kInputFloat_PageTable;
        mFloatAction->setText("&嵌入");
    }
    else {
        mPageStyle = kInputEmbed_PageEmed;
        mFloatAction->setText("&浮动");
    }
    ResizeDockWidget();
}

void MainWindow::InitProfile(){
    //qputenv("QTWEBENGINE_REMOTE_DEBUGGING", QByteArray::number(9000));
    QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);

    QString name = QString("AIChat.") + QString(qWebEngineChromiumVersion());
    mProfile = new QWebEngineProfile(name);
    mProfile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    mProfile->settings()->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    mProfile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    mProfile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    mProfile->settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
}


void MainWindow::OnSendMsg(const QString &msg){
    for(auto it:mPageList){
        it->SendMsg(msg);
    }
}
