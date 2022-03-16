#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QSlider>
#include <QTimer>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QListWidgetItem>
#include <QBluetoothSocket>
#include <QTextToSpeech>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/* 蓝牙串口uuid */
static const QLatin1String serviceUuid("00001101-0000-1000-8000-00805F9B34FB");

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void createTimer();
    void destoryTimer();
    void createTts();
    void destoryTts();
    void dataInit();                                /* 数据初始化 */

private:
    /* 蓝牙部分 */
    void initBlueteeth();
    void destoryBlueteeth();
    void openBlueteethDevice();
    void closeBlueteethDevice();
    void displayLocalBlueteethStatus();
    void setListWidgetStyle();
    void clearListWidgetStyle();
    void createSocket();
    void destorySocket();
    void connecttoSocket();

    void sendMsg();                                 /* 向蓝牙发送信息驱动电机 */
    void receiveMsg();                              /* 向蓝牙接受单片机/传感器信息 */

private:
    void if_out_of_range();                         /* 判断是否超过预设范围 */
    void setPercent();                              /* 设置百分比显示 */
    void setSuggestRange();                         /* 更改预设药品范围 */
    void setRangeval(double minVal, double maxVal); /* 更改数值 */
    void displaySuggestRange();                     /* 显示建议数值 */
    void setRestVal();                              /* 计算剩余点滴量 */
    void speechSomething(const char*);

private slots:
    void on_m_slider_sliderReleased();
    void on_m_drugBox_currentIndexChanged(int index);
    void on_m_slider_valueChanged(int value);
    void getLightSpeed();                           /* 更新滴速 */
    void on_m_openBluetoothButton_clicked();
    void on_m_freshButton_released();
    void addBlueteethDevicesToList(const QBluetoothDeviceInfo&);
    void enableConnectButton();
    void on_m_connectButton_released();
    void on_m_breakButton_released();
    void connectOK();
    void connectNot();
    void readBlueteethDataEvent();

private:
    Ui::MainWindow *ui;


private:
    double m_currentSpeed;
    double m_times;
    double m_minSuggest;
    double m_maxSuggest;
    int m_currentDrugIndex;
    int m_restVal;
    QTextToSpeech *m_tts;

private:
    bool is_blueteethConnect  = false;
    QTimer *m_updateTimer = nullptr;
    QBluetoothLocalDevice* m_localBlueteeth = nullptr;
    QBluetoothDeviceDiscoveryAgent* m_discoveryDevice = nullptr;
    QBluetoothSocket* m_socket = nullptr;

private:
    int m_timerCounter = 0;


};
#endif // MAINWINDOW_H
