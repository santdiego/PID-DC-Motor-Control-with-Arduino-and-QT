//    Distributed under MIT License.
//    Author: Dr. Ing. Diego D. Santiago
//    Institution: INAUT - UNSJ - CONICET
//    Email: dsantiago@inaut.unsj.edu.ar
//    Based in Qt. Terminal example

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"
#include "motorcontrol.h"
#include <QLabel>
#include <QMessageBox>
#include <QtDebug>
//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_status(new QLabel),
    m_console(new Console),
    m_settings(new SettingsDialog),
    m_control(new MotorControl),
    m_serial(new QSerialPort(this)),
    mPlot(0),
    mTag1(0),
    mTag2(0)
//! [1]
{
    //! [0]
    mPlot = new QCustomPlot(this);
    m_ui->setupUi(this);
    m_console->setEnabled(false);
    mPlot->setMinimumSize(500,300);
    m_console->setMinimumSize(200,300);
    m_ui->mainLayout->addWidget(m_control);

    m_ui->mainLayout->addWidget(mPlot);
    m_ui->mainLayout->addWidget(m_console);


    //centralWidget()->setLayout();

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionQuit->setEnabled(true);
    m_ui->actionConfigure->setEnabled(true);

    m_ui->statusBar->addWidget(m_status);

    initActionsConnections();

    shm_motor_in.setNativeKey(SHM_MOTOR_IN);
    shm_motor_out.setNativeKey(SHM_MOTOR_OUT);
    shm_motor_in.create(sizeof(Motor_IN_Parameters));
    shm_motor_out.create(sizeof(Motor_OUT_Parameters));

    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(m_console, &Console::getData, this, &MainWindow::writeData);
    connect(&timer, &QTimer::timeout, this, &MainWindow::writeInstructions);
    connect(&timer, &QTimer::timeout, this, &MainWindow::timerSlot);

    mPlot->yAxis->setTickLabels(false);
 //   connect(mPlot->yAxis2, SIGNAL(rangeChanged(QCPRange)), mPlot->yAxis, SLOT(setRange(QCPRange))); // left axis only mirrors inner right axis
    mPlot->yAxis2->setVisible(true);
    //mPlot->axisRect()->addAxis(QCPAxis::atRight); //agrega un nuevo eje para plotear
    mPlot->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(30); // add some padding to have space for tags
    //mPlot->axisRect()->axis(QCPAxis::atRight, 1)->setPadding(30); // add some padding to have space for tags //no se va a usar el eje derecho adicional

    // create graphs:
    mGraph1 = mPlot->addGraph(mPlot->xAxis, mPlot->axisRect()->axis(QCPAxis::atRight, 0));
    mGraph2 = mPlot->addGraph(mPlot->xAxis, mPlot->axisRect()->axis(QCPAxis::atRight, 0)); //Si se quiere en un eje adicional colocar 1
    QPen pen1(QColor(250, 120, 0));
    QPen pen2(QColor(0, 180, 60));
    pen1.setWidth(2);
    pen2.setWidth(2);

    mGraph1->setPen(pen1);
    mGraph2->setPen(pen2);

    // create tags with newly introduced AxisTag class (see axistag.h/.cpp):
    mTag1 = new AxisTag(mGraph1->valueAxis());
    mTag1->setPen(mGraph1->pen());
    mTag2 = new AxisTag(mGraph2->valueAxis());
    mTag2->setPen(mGraph2->pen());
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    mPlot->yAxis2->setRange(0,16);
    mPlot->yAxis->rescale(true);
    mPlot->replot();
    //connect(&mDataTimer, SIGNAL(timeout()), this, SLOT(timerSlot()));

    //! [3]
}
//! [3]
//!
void MainWindow::timerSlot()
{
    // calculate and add a new data point to each graph:
    mGraph1->addData(mGraph1->dataCount(), double(reinterpret_cast<Motor_OUT_Parameters*>(shm_motor_out.data())->encoder_vel ) );
    mGraph2->addData(mGraph2->dataCount(), double(m_control->ref)/1000);
    // make key axis range scroll with the data:
    mPlot->xAxis->rescale();

    //mGraph1->rescaleValueAxis(false, true);
    //mGraph2->rescaleValueAxis(false, true);
    mPlot->xAxis->setRange(mPlot->xAxis->range().upper, 100, Qt::AlignRight);

    // update the vertical axis tag positions and texts to match the rightmost data point of the graphs:
    double graph1Value = mGraph1->dataMainValue(mGraph1->dataCount()-1);
    double graph2Value = mGraph2->dataMainValue(mGraph2->dataCount()-1);
    mTag1->updatePosition(graph1Value);
    mTag2->updatePosition(graph2Value);
    mTag1->setText(QString::number(graph1Value, 'f', 2));
    mTag2->setText(QString::number(graph2Value, 'f', 2));

    mPlot->replot();
}

MainWindow::~MainWindow()
{
    shm_motor_in.detach();
    shm_motor_out.detach();
    delete m_settings;
    delete m_ui;
}

//! [4]
void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = m_settings->settings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        m_console->setEnabled(true);
        m_console->setLocalEchoEnabled(p.localEchoEnabled);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                              .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                              .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));

    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());

        showStatusMessage(tr("Open error"));
    }
}
//! [4]
void MainWindow::run(){
    const SettingsDialog::Settings p = m_settings->settings();
    if(m_serial->isOpen())
    {
        //m_serial->clear();
        if(p.writeRate_ms.toInt()>0)
            timer.start(p.writeRate_ms.toInt());
        else {
            writeInstructions();
        }
    }
}
//! [5]
void MainWindow::closeSerialPort()
{
    timer.stop();
    if (m_serial->isOpen())
        m_serial->close();
    m_console->setEnabled(false);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
}
//! [5]

void MainWindow::about()
{
    QMessageBox::about(this, tr("Motor PID Tuning"),
                       tr("Program that allows to configure the PID parameters for DC Motor Velocity Control."
                          "\n Author: Dr. Ing. Diego D. Santiago. "));
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
    m_serial->write(data);
}

void MainWindow::writeInstructions()
{
    uint16_t Ki=uint16_t(m_control->Ki);
    uint16_t Kp=uint16_t(m_control->Kp);
    uint16_t Kd=uint16_t(m_control->Kd);
    int16_t  ref=int16_t(m_control->ref);
    uint8_t command[10];//2byte Addr+2byte Ki+2byte Kp+2byte Kd+2byte ref;

    union{
        uint8_t byte[2];
        uint16_t value;
    }uaux;
    union{
        uint8_t byte[2];
        int16_t value;
    }aux;

    uaux.value=0;//addr
    command[0]=uaux.byte[0];
    command[1]=uaux.byte[1];
    aux.value=ref;
    command[2]=aux.byte[0];
    command[3]=aux.byte[1];
    uaux.value=Ki;//Ki
    command[4]=uaux.byte[0];
    command[5]=uaux.byte[1];
    uaux.value=Kp;//Kp
    command[6]=uaux.byte[0];
    command[7]=uaux.byte[1];
    uaux.value=Kd;//Kd
    command[8]=uaux.byte[0];
    command[9]=uaux.byte[1];
    instructionFrame.buildInstructionPacket(1,Write,command,sizeof(command));
    byte temp[BUFFERSIZE];
    uint16_t size;
    instructionFrame.getLastFrame(temp,size);
    m_serial->write(reinterpret_cast<char*>(temp),int(size));
    m_serial->flush();
}

void MainWindow::readData()
{
    //
    //byte data[BUFFERSIZE];
    //int size=m_serial->read((char*)data,BUFFERSIZE);
    const QByteArray data = m_serial->readAll();
    //m_console->insertPlainText("\n From Device [HEX]:"+data.toHex());
    qDebug()<<"\n data size="<<data.size();
    qDebug()<<"\n data="<<data;
    Error_Number e;
    for (int i=0;i<data.size();i++) {
        e=frameDecode.appendFrameByte(data[i]);

        if(e==No_Error){

            //extract data
            byte Id;
            uint16_t len;
            Instruction intruct;
            Error_Number error;
            byte param[BUFFERSIZE];
            e=frameDecode.getStatusPacket(Id,intruct,error,param,len);
            if(e==No_Error){
                if(len==sizeof (SerialDataIn)){
                    SerialDataIn *in=reinterpret_cast<SerialDataIn*>(param);
                    m_console->clear();
                    m_console->insertPlainText(QString("\n goal_vel="+QString::number(in->goal_vel)));
                    m_console->insertPlainText(QString("\n Ki="+QString::number(in->Ki)));
                    m_console->insertPlainText(QString("\n Kp="+QString::number(in->Kp)));
                    m_console->insertPlainText(QString("\n Kd="+QString::number(in->Kd)));
                    m_console->insertPlainText(QString("\n Velocity="+QString::number(in->vel)));
                    m_console->insertPlainText(QString("\n Sampel Time(us)="+QString::number(in->ts)));
                    m_console->insertPlainText(QString("\n Control Action u="+QString::number(in->u)));
                    Motor_OUT_Parameters*shm_out=reinterpret_cast<Motor_OUT_Parameters*>(shm_motor_out.data());
                    shm_out->encoder_vel=float(in->vel)/1000;                   
                }
            }
            else {
                qDebug()<<"getStatusPacket Error: "<<e;
            }
        }
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::initActionsConnections()
{
    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::show);
    connect(m_ui->actionClear, &QAction::triggered, m_console, &Console::clear);
    connect(m_ui->actionClear, &QAction::triggered, this, &MainWindow::cleanPlots);
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(m_ui->actionPlay, &QAction::triggered, this, &MainWindow::run);
    connect(m_ui->actionStop, &QAction::triggered, this, &MainWindow::stop);

}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

void MainWindow::cleanPlots(){
    mPlot->graph(0)->data().data()->clear();
    mPlot->graph(1)->data().data()->clear();
    mPlot->xAxis->rescale();
    mPlot->yAxis2->setRange(0,16);
    mPlot->replot();
}

void MainWindow::stop(){
    timer.stop();
    Sleep(100);
    m_serial->clear();
    short          ref  =0;
    byte command[4];//2byte Addr+2byte ref;
    byte16 tmp;

    tmp.value           =uint16_t(0);//addr
    command[0]          =tmp.word[0];
    command[1]          =tmp.word[1];
    tmp.value           =ref;
    command[2]          =tmp.word[0];
    command[3]          =tmp.word[1];

    instructionFrame.buildInstructionPacket(1,Write,command,sizeof(command));
    byte temp[BUFFERSIZE];
    uint16_t size;
    instructionFrame.getLastFrame(temp,size);
    m_serial->write(reinterpret_cast<char*>(temp),int(size));
    m_serial->flush();
    mPlot->xAxis->rescale();
    mPlot->yAxis->rescale();
    mPlot->replot();
}
