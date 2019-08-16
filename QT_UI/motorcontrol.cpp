//    Distributed under MIT License.
//    Author: Dr. Ing. Diego D. Santiago
//    Institution: INAUT - UNSJ - CONICET
//    Email: dsantiago@inaut.unsj.edu.ar
//    Based in Qt. Terminal example

#include "motorcontrol.h"
#include "ui_motorcontrol.h"

MotorControl::MotorControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MotorControl)
{
    Kp=7000;
    Ki=3000;
    Kd=0;
    ref=15000;

    ui->setupUi(this);
    ui->horizontalSlider_kp->setValue(Kp);
    ui->horizontalSlider_ki->setValue(Ki);
    ui->horizontalSlider_kd->setValue(Kd);
    ui->horizontalSlider_ref->setValue(ref);

}

MotorControl::~MotorControl()
{
    delete ui;
}
void MotorControl::updateParameters(){

}

void MotorControl::on_horizontalSlider_ref_valueChanged(int value)
{
    ref=value;
    ui->lineEdit_ref->setText(QString::number(float(value)/1000));
    emit newParameters();
}

void MotorControl::on_horizontalSlider_kd_valueChanged(int value)
{
    Kd=value;
    ui->lineEdit_Kd->setText(QString::number(float(value)/1000));
    emit newParameters();
}

void MotorControl::on_horizontalSlider_ki_valueChanged(int value)
{
    Ki=value;
    ui->lineEdit_Ki->setText(QString::number(float(value)/1000));
    emit newParameters();
}

void MotorControl::on_horizontalSlider_kp_valueChanged(int value)
{
    Kp=value;
    ui->lineEdit_Kp->setText(QString::number(float(value)/1000));
    emit newParameters();
}

void MotorControl::on_lineEdit_ref_textChanged(const QString &arg1)
{
    ui->horizontalSlider_ref->setValue(int(arg1.toFloat()*1000));
}

void MotorControl::on_lineEdit_Ki_textChanged(const QString &arg1)
{
    ui->horizontalSlider_ki->setValue(int(arg1.toFloat()*1000));
}

void MotorControl::on_lineEdit_Kp_textChanged(const QString &arg1)
{
    ui->horizontalSlider_kp->setValue(int(arg1.toFloat()*1000));
}

void MotorControl::on_lineEdit_Kd_textChanged(const QString &arg1)
{
    ui->horizontalSlider_kd->setValue(int(arg1.toFloat()*1000));
}
