#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScroller>


#define Android
#define TIMEOUT (2000)


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setListWidgetStyle();
    dataInit();
    initBlueteeth();
    createTimer();
    createTts();
}

MainWindow::~MainWindow()
{
    delete ui;
    destoryBlueteeth();
    destoryTimer();
    destoryTts();
    destorySocket();
}

void MainWindow::setListWidgetStyle()
{
    // ui->listWidget->setHorizontalScrollMode(QListWidget::ScrollPerPixel);
    QScroller::grabGesture(ui->listWidget,QScroller::LeftMouseButtonGesture);
    // ui->listWidget->setWrapping(true);
    // ui->listWidget->setFlow(QListView::LeftToRight);
}

void MainWindow::clearListWidgetStyle()
{
    ui->listWidget->clear();
    m_discoveryDevice->stop();
}

void MainWindow::createTts()
{
    m_tts = new QTextToSpeech(this);
}

void MainWindow::destoryTts()
{
    delete m_tts;
}

void MainWindow::createTimer()
{
    m_updateTimer = new QTimer();
    m_updateTimer->setInterval(TIMEOUT);
    connect(m_updateTimer,
            SIGNAL(timeout()),
            this,
            SLOT(getLightSpeed()));
    m_updateTimer->start();
}

void MainWindow::destoryTimer()
{
    delete m_updateTimer;
}

void MainWindow::dataInit()
{
    setPercent();
    setSuggestRange();
    getLightSpeed();
}

void MainWindow::setRangeval(double minVal, double maxVal)
{
    m_minSuggest = minVal;
    m_maxSuggest = maxVal;
}

void MainWindow::displaySuggestRange()
{
    // ui->m_maxSuggestSpeed->setText(QString::number(m_maxSuggest, 'f', 2));
    // ui->m_minSuggestSpeed->setText(QString::number(m_minSuggest, 'f', 2));
    ui->m_maxSuggestSpeed->setNum(m_maxSuggest);
    ui->m_minSuggestSpeed->setNum(m_minSuggest);
}

void MainWindow::setSuggestRange()
{
    m_currentDrugIndex =  ui->m_drugBox->currentIndex();
    switch (m_currentDrugIndex) {
    case 0:
        setRangeval(1.0, 1.5);break;
    case 1:
        setRangeval(0.75, 1.25);break;
    case 2:
        setRangeval(0.5, 2.0); break;
    default:
        setRangeval(1.0, 2.0); break;
    }
    displaySuggestRange();
}

void MainWindow::getLightSpeed()
{
    /* 蓝牙未连接退出 */
    if(!is_blueteethConnect)
        return;

    /* 否则从蓝牙获取信息 */
    /* 获取光敏点滴速度, 每隔一段时间（定时器）再更新********************************************************************************************/
    receiveMsg();
    ++m_timerCounter;
    // m_currentSpeed = (1.5 + rand() % 10);

    // ui->m_currentSpeed->setNum(m_currentSpeed);
    // 显示
    if (m_timerCounter==5)
    {
        ui->m_currentSpeedText->setText(QString::number(m_currentSpeed, 'f', 2));
        if_out_of_range();
        m_timerCounter = 0;
    }
}

void MainWindow::setPercent()
{
    /* 设置速率百分比 */
    m_times= static_cast<double>(ui->m_slider->value()-50)/100+1.0;
    ui->m_sliderTimes->setText(QString::number(static_cast<int>(m_times * 100))+"%");
}

void MainWindow::if_out_of_range()
{
    /* 判断是否超过药品范围 */
    double maxSuggestSpeed = ui->m_maxSuggestSpeed->text().toDouble();
    double minSuggestSpeed = ui->m_minSuggestSpeed->text().toDouble();
    // qDebug()<<ui->m_minSuggestSpeed->text().toDouble();
    // qDebug()<<ui->m_maxSuggestSpeed->text().toDouble();
    if(m_currentSpeed>maxSuggestSpeed)
    {
        /* 语音提示速度过快 */
        // qDebug()<<"速度过快";
        speechSomething("点滴速度过快");
    }else if (m_currentSpeed<minSuggestSpeed&&m_currentSpeed != 0.0)
    {
        /* 语音提示速度过慢 */
        // qDebug()<<"速度过慢";
        speechSomething("点滴速度过慢");
    }
    else if (m_currentSpeed == 0.0) {
        speechSomething("注意，没有点滴了");
    }
}

void MainWindow::initBlueteeth()
{
    m_localBlueteeth = new QBluetoothLocalDevice();
    m_discoveryDevice = new QBluetoothDeviceDiscoveryAgent();
    connect(m_discoveryDevice,
            SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this,
            SLOT(addBlueteethDevicesToList(QBluetoothDeviceInfo))
            );
    displayLocalBlueteethStatus();

}

void MainWindow::destoryBlueteeth()
{
    delete m_localBlueteeth;
    delete m_discoveryDevice;
}

void MainWindow::displayLocalBlueteethStatus()
{
    if (m_localBlueteeth->hostMode()==QBluetoothLocalDevice::HostPoweredOff)
    {
        ui->m_openBluetoothButton->setText("打开蓝牙");
        ui->m_blueteethLabel->setText("蓝牙未连接");
        ui->m_freshButton->setEnabled(false);
    }else
    {
        ui->m_openBluetoothButton->setText("关闭蓝牙");
        ui->m_blueteethLabel->setText("蓝牙可连接");
        ui->m_freshButton->setEnabled(true);
    }
}

void MainWindow::openBlueteethDevice()
{
    ui->m_openBluetoothButton->setEnabled(false);
    m_localBlueteeth->powerOn();
    ui->m_openBluetoothButton->setText("关闭蓝牙");
    ui->m_blueteethLabel->setText("蓝牙可连接");
    QTimer::singleShot(1500, this, [&](){
       this->ui->m_openBluetoothButton->setEnabled(true);
       this->ui->m_freshButton->setEnabled(true);
    });
}

void MainWindow::closeBlueteethDevice()
{
    ui->m_openBluetoothButton->setEnabled(false);
    clearListWidgetStyle();
    m_localBlueteeth->setHostMode(QBluetoothLocalDevice::HostPoweredOff);
    ui->m_openBluetoothButton->setText("打开蓝牙");
    ui->m_blueteethLabel->setText("蓝牙未连接");
    on_m_breakButton_released();
    ui->m_connectButton->setEnabled(false);
    QTimer::singleShot(1500, this, [&](){
       this->ui->m_openBluetoothButton->setEnabled(true);
       this->ui->m_freshButton->setEnabled(false);
    });
}

void MainWindow::sendMsg()
{
    /* 驱动电机运动******************************************************************************************************************************/
    if (!m_socket)
    {
        return;
    }
    QString Msg = ui->m_sliderTimes->text();
    // Msg = Msg.left(Msg.size()-1); 去掉最后一个字符
    int length = Msg.size();
    Msg[length-1] = '\n';
    m_socket->write(Msg.toUtf8());
    qDebug() << "发送成功"<<ui->m_sliderTimes->text().toUtf8();

}

void MainWindow::receiveMsg()
{
    QByteArray line = m_socket->readAll();
    QString strData(line);
    m_currentSpeed = static_cast<double>(strData.toFloat());
    qDebug() << "接收成功: "<<strData.toFloat();
}

void MainWindow::setRestVal()
{
    m_restVal = 1500;
    ui->m_restNum->setNum(m_restVal);
}

void MainWindow::speechSomething(const char* str)
{
    m_tts ->setRate(0.5);
    m_tts ->setPitch(0.5);
    m_tts ->setVolume(1.5);
    if(m_tts->state()==QTextToSpeech::Ready)
    {
        m_tts->say(str);
    }
}

void MainWindow::createSocket()
{
    while(!m_socket)
        m_socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    //蓝牙连接设备成功后会停止搜索设备，显示连接成功
    connect(m_socket,
            SIGNAL(connected()),
            this,
            SLOT(connectOK()));
    //蓝牙连断开连接后，会显示已断开连接
    connect(m_socket,
            SIGNAL(disconnected()),
            this,
            SLOT(connectNot()));
    //接受到上位机传来的数据后显示，会触发接受数据函数
    connect(m_socket,
            SIGNAL(readyRead()),
            this,
            SLOT(readBlueteethDataEvent()));
}

void MainWindow::destorySocket()
{
    if (m_socket)
    {
        delete m_socket;
    }
}

void MainWindow::connecttoSocket()
{
    auto text = ui->listWidget->currentItem()->text();
    auto index = text.indexOf(' ');
    if (index == -1)
        return;
    QBluetoothAddress address(text.left(index));
    QString name(text.mid(index + 1));
    m_socket->connectToService(address, QBluetoothUuid(serviceUuid) ,QIODevice::ReadWrite);
}

void MainWindow::connectOK()
{
    m_discoveryDevice->stop();  //停止搜索设备
    is_blueteethConnect = true;
    ui->m_freshButton->setEnabled(false);

    ui->m_blueteethLabel->setText("蓝牙已连接");
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::green);
    ui->m_blueteethLabel->setPalette(pe);
    ui->m_slider->setEnabled(true);
}

void MainWindow::connectNot()
{
    is_blueteethConnect = false;
    ui->m_freshButton->setEnabled(true);

    if(m_localBlueteeth->hostMode()==QBluetoothLocalDevice::HostPoweredOff)
    {
        /* 如果是蓝牙关闭状态 */
        ui->m_blueteethLabel->setText("蓝牙未连接");
    }else
    {
       ui->m_blueteethLabel->setText("蓝牙可连接");
    }
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::black);
    ui->m_blueteethLabel->setPalette(pe);
    ui->m_slider->setEnabled(false);
}

void MainWindow::readBlueteethDataEvent()
{

}

void MainWindow::on_m_slider_valueChanged(int value)
{
    /* 更新slider数值 */
    setPercent();
}

void MainWindow::on_m_slider_sliderReleased()
{
    /* 驱动电机入口,发送m_times到蓝牙 */
    sendMsg();
    /* 这里可以替代成实际上的滴速检测 */
#ifndef Android
    /* 理论计算值 */
    getLightSpeed();
    m_currentSpeed*=m_times;
#else
    /* 重新计时 */
    m_updateTimer->start();
    /* 更新实际观测值 */
    getLightSpeed();
#endif
}

void MainWindow::on_m_drugBox_currentIndexChanged(int index)
{
    setSuggestRange();
    setRestVal();
}

void MainWindow::on_m_openBluetoothButton_clicked()
{
    if (m_localBlueteeth->hostMode()==QBluetoothLocalDevice::HostPoweredOff)
    {
        openBlueteethDevice();
    }
    else
    {
        closeBlueteethDevice();
    }
}

void MainWindow::on_m_freshButton_released()
{
    m_discoveryDevice->start();
}

void MainWindow::addBlueteethDevicesToList(const QBluetoothDeviceInfo& info)
{
    QString label = QString("%1 %2").arg(info.address().toString()).arg(info.name());
    QList<QListWidgetItem *> items = ui->listWidget->findItems(label, Qt::MatchExactly);

    if (items.empty())
    {
        QListWidgetItem *item = new QListWidgetItem(label);
        QBluetoothLocalDevice::Pairing pairingStatus = m_localBlueteeth->pairingStatus(info.address());
        /* 蓝牙状态pairingStatus，Pairing枚举类型
         * 0:Unpaired没配对
         * 1:Paired配对但没授权
         * 2:AuthorizedPaired配对且授权 */
        if (pairingStatus == QBluetoothLocalDevice::Paired || pairingStatus == QBluetoothLocalDevice::AuthorizedPaired )
        {
            item->setForeground(QColor(Qt::green));
        }
        else
        {
            item->setForeground(QColor(Qt::black));
        }
        ui->listWidget->addItem(item);
        connect(ui->listWidget,
                SIGNAL(itemClicked(QListWidgetItem*)),
                this,
                SLOT(enableConnectButton())
                );
    }
}

void MainWindow::enableConnectButton()
{
    ui->m_connectButton->setEnabled(true);
    if (!m_socket)
    {
        createSocket();
    }
}

void MainWindow::on_m_connectButton_released()
{
    ui->m_connectButton->setEnabled(false);
    connecttoSocket();
    ui->listWidget->setEnabled(false);
    ui->m_breakButton->setEnabled(true);
}

void MainWindow::on_m_breakButton_released()
{
    if (m_socket)
    {
        m_socket->disconnectFromService();
    }
    ui->m_breakButton->setEnabled(false);
    ui->m_connectButton->setEnabled(true);
    ui->listWidget->setEnabled(true);
    ui->m_currentSpeedText->setText("");
}
