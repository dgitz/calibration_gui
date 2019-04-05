#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QNetworkInterface>
#include "../../gui_common/include/helper.h"
#include "../../gui_common/include/udptransmitter.h"
#include "../../gui_common/include/udpreceiver.h"
#define SETUP_TAB 0
#define CALIBRATION_IMU_TAB 1
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void send_Heartbeat_message();
    void send_RemoteControl_message();
    void update_messageviewer(const Diagnostic &diag);
    void update_messageviewer(const ArmedState &state);
    void start_udp();
    void update_ui();
    void calibration_imu_preview();
    void calibration_imu_manualcontrol();
    void calibration_imu_autocontrol();
    void pan_servo_reset();
    void roll_servo_reset();
    void tilt_servo_reset();
    void update_calibration_imu();

private:
    bool new_udpmsgsent(std::string id);
    bool new_udpmsgreceived(std::string id);
    void init_udpmessageinfo();


    std::vector<UDPMessageInfo> udp_messages;
    Ui::MainWindow *ui;
    QString DeviceName;
    UDPTransmitter udp_transmitter;
    UDPReceiver udp_receiver;
    bool udp_connection_active;

    QTimer *timer_10ms;
    QTimer *timer_50ms;
    QTimer *timer_100ms;
    QTimer *timer_1000ms;
    QTimer *timer_5000ms;

    QElapsedTimer calibration_imu_lasttimeran;

    //IMU Calibration Variables
    bool calibration_imu_preview_running;
    bool calibration_imu_manualcontrol_running;
    bool calibration_imu_autocontrol_running;
    uint16_t cycle_count;
    uint8_t servo_to_update;
    double pan_servo_value;
    bool pan_servo_direction;
    int pan_servo_min;
    int pan_servo_max;
    double pan_servo_duration;

    double roll_servo_value;
    bool roll_servo_direction;
    int roll_servo_min;
    int roll_servo_max;
    double roll_servo_duration;

    double tilt_servo_value;
    bool tilt_servo_direction;
    int tilt_servo_min;
    int tilt_servo_max;
    double tilt_servo_duration;

};

#endif // MAINWINDOW_H
