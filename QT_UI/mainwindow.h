//    Distributed under MIT License.
//    Author: Dr. Ing. Diego D. Santiago
//    Institution: INAUT - UNSJ - CONICET
//    Email: dsantiago@inaut.unsj.edu.ar
//    Based in Qt. Terminal example

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSharedMemory>
#include <QTimer>
#include "serialframedecoder.h"
#include "qcustomplot.h"
#include "axistag.h"

#define SHM_MOTOR_IN    "motor_in"
#define SHM_MOTOR_OUT   "motor_out"


QT_BEGIN_NAMESPACE

class QLabel;

struct SerialDataIn{
    int16_t goal_vel;
    uint16_t Ki;
    uint16_t Kp;
    uint16_t Kd;
    uint16_t vel;
    int16_t ts;
    int16_t u;

};

struct Motor_IN_Parameters{
    float Ki;
    float Kp;
    float Kd;
    float vel_ref;
};
struct Motor_OUT_Parameters{
    float encoder_vel;
};

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class Console;
class SettingsDialog;
class MotorControl;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void about();
    void writeData(const QByteArray &data);
    void writeInstructions();
    void readData();
    void run();
    void handleError(QSerialPort::SerialPortError error);
    void timerSlot();
    void cleanPlots();
    void stop();
private:
    void initActionsConnections();
    void showStatusMessage(const QString &message);

    Ui::MainWindow *m_ui = nullptr;
    QLabel *m_status = nullptr;
    Console *m_console = nullptr;
    SettingsDialog *m_settings = nullptr;
    MotorControl *m_control = nullptr;

    QSerialPort *m_serial = nullptr;
    QSharedMemory shm_motor_in;
    QSharedMemory shm_motor_out;
    SerialFrameDecoder frameDecode;
    SerialFrameDecoder instructionFrame;
    QTimer timer;

    //Plots
    QCustomPlot *mPlot;
    QPointer<QCPGraph> mGraph1;
    QPointer<QCPGraph> mGraph2;
    AxisTag *mTag1;
    AxisTag *mTag2;
    //QTimer mDataTimer;
};

#endif // MAINWINDOW_H
