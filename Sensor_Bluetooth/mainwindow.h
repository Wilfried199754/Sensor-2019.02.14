#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma execution_character_set("utf-8")  //避免中文乱码


#include <QMainWindow>
#include <QString>
#include <QTime>
#include <QTimer>
#include <QSpinBox>
#include <QDebug>
#include <QCloseEvent>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QDateTime>
#include <QDir>




namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_Number_overflow();

    void on_Time_overflow();

    void on_SetTime_clicked();

    void updateUI();

    void on_Start_clicked();

    void on_Stop_clicked();

    void on_Zero_clicked();

    void on_Timing_clicked();

    void on_CountTimes_clicked();

    void Delay_MSec(unsigned int msec);

    void closeEvent(QCloseEvent *event);

    void Read_Data();

    void on_BlueTooth_clicked();

    void on_ServiceNumber_clicked();

    void getSystemTime();


private:
    Ui::MainWindow *ui;
    QTimer *timer;     //定时器对象指针
    QTimer *timer2;
    QTime baseTime;     //基准时间
    bool ok;
    int ShowTimeStr=60;    //要在LCD中显示的时间，默认1分钟
    int ShowNumberStr=0;    //要在LCD中显示的次数
    int TimeSetNumber;
    int tempTime=60;
    int tempNumber=0;
    QString TimeSet;       //用于储存倒计时时间的变量
    QString Echo="1";//返回值
    int countTimes;
    int countData[100];
    int value=1;
    int allValue=0;
    bool countSign=0;//计数标志位，0位停止，1为开始
    bool loopSign=0;//计数循环标志位，0为当前循环未结束，1为结束
    QSerialPort *serial;
    QString systemTime;

};

#endif // MAINWINDOW_H
