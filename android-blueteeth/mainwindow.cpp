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
    /* ????????????????????? */
    if(!is_blueteethConnect)
        return;

    /* ??????????????????????????? */
    /* ????????????????????????, ??????????????????????????????????????????********************************************************************************************/
    receiveMsg();
    ++m_timerCounter;
    // m_currentSpeed = (1.5 + rand() % 10);

    // ui->m_currentSpeed->setNum(m_currentSpeed);
    // ??????
    if (m_timerCounter==5)
    {
        ui->m_currentSpeedText->setText(QString::number(m_currentSpeed, 'f', 2));
        if_out_of_range();
        m_timerCounter = 0;
    }
}

void MainWindow::setPercent()
{
    /* ????????????????????? */
    m_times= static_cast<double>(ui->m_slider->value()-50)/100+1.0;
    ui->m_sliderTimes->setText(QString::number(static_cast<int>(m_times * 100))+"%");
}

void MainWindow::if_out_of_range()
{
    /* ?????????????????????????????? */
    double maxSuggestSpeed = ui->m_maxSuggestSpeed->text().toDouble();
    double minSuggestSpeed = ui->m_minSuggestSpeed->text().toDouble();
    // qDebug()<<ui->m_minSuggestSpeed->text().toDouble();
    // qDebug()<<ui->m_maxSuggestSpeed->text().toDouble();
    if(m_currentSpeed>maxSuggestSpeed)
    {
        /* ???????????????????????? */
        // qDebug()<<"????????????";
        speechSomething("??????????????????");
    }else if (m_currentSpeed<minSuggestSpeed&&m_currentSpeed != 0.0)
    {
        /* ???????????????????????? */
        // qDebug()<<"????????????";
        speechSomething("??????????????????");
    }
    else if (m_currentSpeed == 0.0) {
        speechSomething("????????????????????????");
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
        ui->m_openBluetoothButton->setText("????????????");
        ui->m_blueteethLabel->setText("???????????????");
        ui->m_freshButton->setEnabled(false);
    }else
    {
        ui->m_openBluetoothButton->setText("????????????");
        ui->m_blueteethLabel->setText("???????????????");
        ui->m_freshButton->setEnabled(true);
    }
}

void MainWindow::openBlueteethDevice()
{
    ui->m_openBluetoothButton->setEnabled(false);
    m_localBlueteeth->powerOn();
    ui->m_openBluetoothButton->setText("????????????");
    ui->m_blueteethLabel->setText("???????????????");
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
    ui->m_openBluetoothButton->setText("????????????");
    ui->m_blueteethLabel->setText("???????????????");
    on_m_breakButton_released();
    ui->m_connectButton->setEnabled(false);
    QTimer::singleShot(1500, this, [&](){
       this->ui->m_openBluetoothButton->setEnabled(true);
       this->ui->m_freshButton->setEnabled(false);
    });
}

void MainWindow::sendMsg()
{
    /* ??????????????????******************************************************************************************************************************/
    if (!m_socket)
    {
        return;
    }
    QString Msg = ui->m_sliderTimes->text();
    // Msg = Msg.left(Msg.size()-1); ????????????????????????
    int length = Msg.size();
    Msg[length-1] = '\n';
    m_socket->write(Msg.toUtf8());
    qDebug() << "????????????"<<ui->m_sliderTimes->text().toUtf8();

}

void MainWindow::receiveMsg()
{
    QByteArray line = m_socket->readAll();
    QString strData(line);
    m_currentSpeed = static_cast<double>(strData.toFloat());
    qDebug() << "????????????: "<<strData.toFloat();
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
    //?????????????????????????????????????????????????????????????????????
    connect(m_socket,
            SIGNAL(connected()),
            this,
            SLOT(connectOK()));
    //???????????????????????????????????????????????????
    connect(m_socket,
            SIGNAL(disconnected()),
            this,
            SLOT(connectNot()));
    //????????????????????????????????????????????????????????????????????????
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
    m_discoveryDevice->stop();  //??????????????????
    is_blueteethConnect = true;
    ui->m_freshButton->setEnabled(false);

    ui->m_blueteethLabel->setText("???????????????");
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
        /* ??????????????????????????? */
        ui->m_blueteethLabel->setText("???????????????");
    }else
    {
       ui->m_blueteethLabel->setText("???????????????");
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
    /* ??????slider?????? */
    setPercent();
}

void MainWindow::on_m_slider_sliderReleased()
{
    /* ??????????????????,??????m_times????????? */
    sendMsg();
    /* ????????????????????????????????????????????? */
#ifndef Android
    /* ??????????????? */
    getLightSpeed();
    m_currentSpeed*=m_times;
#else
    /* ???????????? */
    m_updateTimer->start();
    /* ????????????????????? */
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
        /* ????????????pairingStatus???Pairing????????????
         * 0:Unpaired?????????
         * 1:Paired??????????????????
         * 2:AuthorizedPaired??????????????? */
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
