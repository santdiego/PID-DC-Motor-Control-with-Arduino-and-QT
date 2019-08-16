//    Distributed under MIT License.
//    Author: Dr. Ing. Diego D. Santiago
//    Institution: INAUT - UNSJ - CONICET
//    Email: dsantiago@inaut.unsj.edu.ar
//    Based in Qt. Terminal example

#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <QWidget>

namespace Ui {
class MotorControl;
}

class MotorControl : public QWidget
{
    Q_OBJECT

public:
    explicit MotorControl(QWidget *parent = nullptr);
    ~MotorControl();
    int Kp;
    int Ki;
    int Kd;
    int ref;
public slots:
    void updateParameters();
signals:
    void newParameters();

private slots:
    void on_horizontalSlider_ref_valueChanged(int value);

    void on_horizontalSlider_kd_valueChanged(int value);

    void on_horizontalSlider_ki_valueChanged(int value);

    void on_horizontalSlider_kp_valueChanged(int value);

    void on_lineEdit_ref_textChanged(const QString &arg1);

    void on_lineEdit_Ki_textChanged(const QString &arg1);

    void on_lineEdit_Kp_textChanged(const QString &arg1);

    void on_lineEdit_Kd_textChanged(const QString &arg1);

private:
    Ui::MotorControl *ui;
};

#endif // MOTORCONTROL_H
