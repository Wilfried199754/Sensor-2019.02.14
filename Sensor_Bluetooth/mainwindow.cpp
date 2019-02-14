#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <string>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    //timer->setInterval(10);
    //每次计数会有2次激励，需要/2
    ui->Number->setDecMode();//设置次数显示为十进制
    ui->Time->setDecMode();//设置倒计时显示为十进制
    ui->Number->setDigitCount(7);//设置次数最多显示7位，可调整
    ui->Time->setDigitCount(7);//设置时间最多显示7位，可调整
    ui->Number->display(QString::number(tempNumber/2));//显示次数
    ui->Time->display(QString::number(tempTime));//显示时间

    connect(ui->Number,SIGNAL(overflow()),this,SLOT(on_Number_overflow()));//槽函数连接，次数溢出报错
    connect(ui->Time,SIGNAL(overflow()),this,SLOT(on_Time_overflow()));//槽函数连接，时间溢出报错

    connect(timer,SIGNAL(timeout()),this,SLOT(updateUI()));


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Number_overflow()//计数溢出报错
{
    QMessageBox::warning(NULL,"warning","Number_Data_Overflow!",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
}

void MainWindow::on_Time_overflow()//时间溢出报错
{
    QMessageBox::warning(NULL,"warning","Time_Data_Overflow!",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
}

void MainWindow::on_SetTime_clicked()//设置倒计时时间
{
    TimeSet = ui->TimeSetNumber->toPlainText();
    ShowTimeStr = TimeSet.toFloat(&ok)*60;
    tempTime = ShowTimeStr;
    ui->Time->display(QString::number(tempTime));//显示时间
}

void MainWindow::updateUI()//刷新数据
{
    if(tempTime>0){
        ui->Number->display(QString::number(tempNumber/2));//显示次数
        ui->Time->display(QString::number(--tempTime));//显示时间
    }
    else{
        //QMessageBox::warning(NULL,"提示","时间到！",QMessageBox::Yes,QMessageBox::Yes);
        on_Stop_clicked();
    }
}

void MainWindow::on_Start_clicked()
{
    //on_Zero_clicked();
    if(ui->BlueTooth->text() == tr("BlueTooth")){
        on_BlueTooth_clicked();
    }
    if(ui->Start->text() == tr("开始计数"))
    {
        if(ui->BlueTooth->text() == tr("关闭蓝牙"))
        {
            ui->Start->setText(tr("暂停计数"));
            countSign=1;
        }
        else{
            //on_Stop_clicked();
            QMessageBox::warning(NULL,"警告","请检查通讯连接",QMessageBox::Yes,QMessageBox::Yes);

        }
    }
    else{
        countSign=0;
        ShowNumberStr = tempNumber/2;
        ui->Number->display(QString::number(ShowNumberStr));//显示次数
        ui->Start->setText(tr("开始计数"));
    }
}
//暂停计数
void MainWindow::on_Stop_clicked()
{
    if(ui->Timing->text() == tr("计时中")&&ui->Stop->text()=="暂停"){
        ui->Timing->setText("暂停中");
        ui->Stop->setText("继续");
        ui->Timing->setEnabled(false);
        ui->Number->display(QString::number(ShowNumberStr));//显示次数
        ui->Time->display(QString::number(tempTime));//显示时间
        timer->stop();
    }
    else if(ui->Timing->text() == tr("定时")&&ui->Stop->text()=="暂停"){
    }
    else {
        ui->Timing->setText("计时中");
        ui->Stop->setText("暂停");
        ui->Timing->setEnabled(true);
        timer->start();
    }
}
//清零数据，重置时间
void MainWindow::on_Zero_clicked()
{
    on_SetTime_clicked();
    on_CountTimes_clicked();
    tempTime = ShowTimeStr;
    tempNumber = 0;
    ShowNumberStr=0;
    ui->Number->display(QString::number(tempNumber/2));//显示次数
    ui->Time->display(QString::number(tempTime));//显示时间
    ui->Timing->setText("定时");

}
//开始计时，自动获取计时时间和计时次数，连接蓝牙，计数完成后输出每次计数次数和最大值平均值
void MainWindow::on_Timing_clicked()
{
    on_CountTimes_clicked();
    on_SetTime_clicked();
    if(ui->Timing->text() == tr("定时"))
    {

        ui->Timing->setText("计时中");
        ui->Zero->setEnabled(false);
        int temp[100];int i=0;int j=1;
        int max=0;
        QString str;int average=0;
        for(;value>0;value--)
        {
            timer->start(1000);
            on_Start_clicked();
            while(tempTime>0)
            {
                //loopSign=0;
                qApp->processEvents();
            }
            temp[i]=tempNumber/2;i++;
            average+=tempNumber/2;
            if(tempNumber/2>max){
                max=tempNumber/2;
            }
            QString ichar=QString::number(j);
            str+="第";
            str+=ichar;
            str+="次成绩：";
            str+=QString::number(temp[j-1]);
            str+='\n';
            j++;
            timer->stop();
            on_Zero_clicked();//清零
            on_Start_clicked();//复位按钮
            ui->Start->setText(tr("开始计数"));
            ui->Timing->setText("等待中");
            if(value>1)
            {
                QMessageBox *box = new QMessageBox(QMessageBox::Information,tr("提示"),tr("时间到!下一次计时将在30秒后开始！"));
                QTimer::singleShot(30000,box,SLOT(accept())); //也可将accept改为close
                box->exec();//box->show();都可以
                ui->Timing->setText("计时中");
            }
            else{
                str+="平均：";
                str+=QString::number(average/allValue);
                str+="次";
                str+='\n';
                str+="最大：";
                str+=QString::number(max);
                str+="次";
                ui->Result->setPlainText(str);
                getSystemTime();
                QString fileName = "Data.txt";//指定文件夹路径为D盘根目录
                QFile file(fileName);
                if(file.open(QIODevice::WriteOnly |QIODevice::Text|QIODevice::Append ))
                {
                    QTextStream in(&file);
                    in << systemTime<<'\r\n'<<str<<'\r\n';
                    //file.flush();
                    file.close();

                }
                else
                {
                    QMessageBox(QMessageBox::Information,tr("警告"),tr("文件打开失败！请在D盘新建名为Data的文本文档"));
                }
            }
        }
        ui->Timing->setText("定时");

    }
    else {
        timer->stop();
        ui->Zero->setEnabled(true);
        on_Start_clicked();
        ui->Timing->setText("定时");

    }
}
//获取计数次数
void MainWindow::on_CountTimes_clicked()
{
    value=ui->Times->value();//获取定时次数
    allValue=value;
}
//软件延迟，以后可能用到
void MainWindow::Delay_MSec(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}
//关闭程序时确认一次，然后杀死进程
void MainWindow::closeEvent(QCloseEvent *event)
{
    switch( QMessageBox::information(this,tr("提示"),tr("你确定退出该软件?"),tr("确定"), tr("取消"),0,1))
    {
    case 0:
        event->accept();
        exit (0);
        break;
    case 1:
    default:
        event->ignore();
        break;
    }
}
//自动查找可连接串口
void MainWindow::on_ServiceNumber_clicked()
{
    //查找可用的串口
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            int temp=ui->PortBox->count();
            for(;temp>0;temp--){
                ui->PortBox->removeItem (temp);
            }
            ui->PortBox->addItem(serial.portName());
            serial.close();
        }
    }
    qDebug() << tr("界面设定成功!");
}
//读取数据，如果数据符合，计数+1
void MainWindow::Read_Data()
{
    QString buf;
    if(serial->bytesAvailable() )
    {
        buf = serial->readAll();
        serial->flush();
    }
    if(!buf.isEmpty())
    {
        QString str;//测试用
        if(buf=='0'){
            tempNumber++;
            ui->Number->display(QString::number(tempNumber/2));//显示次数
        }
        str+=buf;
        //ui->textEdit_Recv->clear();
        //ui->Result->append(str);//测试用，需要建立测试框
    }
    buf.clear();
}
//连接串口，设置数据位为8，停止位1，校验位0
void MainWindow::on_BlueTooth_clicked()
{
    if(ui->PortBox->currentText()!=NULL)
    {
        if(ui->BlueTooth->text()==tr("BlueTooth"))
        {
            serial = new QSerialPort;
            //设置串口名
            serial->setPortName(ui->PortBox->currentText());
            //打开串口
            serial->open(QIODevice::ReadWrite);
            //设置波特率
            serial->setBaudRate(ui->PortBox->currentText().toInt());
            //设置数据位数
            serial->setDataBits(QSerialPort::Data8);
            //设置奇偶校验
            serial->setParity(QSerialPort::NoParity);
            //设置停止位
            serial->setStopBits(QSerialPort::OneStop);
            //设置流控制
            serial->setFlowControl(QSerialPort::NoFlowControl);
            //关闭设置菜单使能
            ui->BlueTooth->setText(tr("关闭蓝牙"));
            //连接信号槽
            QObject::connect(serial, &QSerialPort::readyRead, this, &MainWindow::Read_Data);
        }
        else
        {
            //关闭串口
            serial->clear();
            serial->close();
            serial->deleteLater();
            //恢复设置使能
            ui->PortBox->setEnabled(true);
            ui->BlueTooth->setText(tr("BlueTooth"));

        }
    }
    else{
        timer->stop();
        ui->Timing->setText("定时");
        QMessageBox::warning(NULL,"警告","请选择串口",QMessageBox::Yes,QMessageBox::Yes);
    }
}
//获取当前时间戳
void MainWindow::getSystemTime(){
    QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
    systemTime = time.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
}

