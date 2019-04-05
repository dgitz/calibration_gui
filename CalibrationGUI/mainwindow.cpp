#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init_udpmessageinfo();
    ui->tRecvPort->setText("45454");
    ui->tRecvServer->setText("239.255.43.21");
    ui->tSendPort->setText("5678");
    ui->tSendServer->setText("127.0.0.1");
    ui->tPanServoMax->setText("100");
    ui->tPanServoMin->setText("-100");
    ui->tPanServoTime->setText("1.0");
    ui->tRollServoMax->setText("100");
    ui->tRollServoMin->setText("-100");
    ui->tRollServoTime->setText("1.0");
    ui->tTiltServoMax->setText("100");
    ui->tTiltServoMin->setText("-100");
    ui->tTiltServoTime->setText("1.0");
    ui->tabWidget->setCurrentIndex(SETUP_TAB);
    udp_connection_active = false;
    cycle_count= 0;
    servo_to_update = 0;
    pan_servo_value = 0;
    pan_servo_direction = false;
    roll_servo_value = 0;
    roll_servo_direction = false;
    tilt_servo_value = 0;
    tilt_servo_direction = false;
    calibration_imu_preview_running = false;
    calibration_imu_manualcontrol_running = false;
    ui->bConnect->setStyleSheet("color: black;"
                                "background-color: gray;"
                                );
    ui->bManualControl->setStyleSheet("color: black;"
                                      "background-color: gray;"
                                      );
    ui->bPreview->setStyleSheet("color: black;"
                                "background-color: gray;"
                                );
    ui->bAutoControl->setStyleSheet("color: black;"
                                "background-color: gray;"
                                );
    ui->bConnect->setText("Connect");
    //udp_transmitter.set_server_info("127.0.0.1",5678);
    //udp_transmitter.set_debugmode(true);
    //udp_receiver.set_debug_mode(true);
    // udp_receiver.set_server_info("239.255.43.21",45454);
    //udp_receiver.Start();

    QList<QHostAddress> list = QNetworkInterface::allAddresses();

    for(int i = 0; i <list.count();i++)
    {
        if(!list[i].isLoopback())
        {
            if(list[i].protocol() == QAbstractSocket::IPv4Protocol)
            {
                //qDebug() << list[i].toString();
                DeviceName = "UI_" + list[i].toString();
                break;
            }
        }
    }

    timer_10ms = new QTimer(this);
    timer_50ms = new QTimer(this);
    timer_100ms = new QTimer(this);
    timer_1000ms = new QTimer(this);
    timer_5000ms = new QTimer(this);

    connect(ui->bConnect,SIGNAL(clicked(bool)),SLOT(start_udp()));
    connect(ui->bPreview,SIGNAL(clicked(bool)),SLOT(calibration_imu_preview()));
    connect(ui->bManualControl,SIGNAL(clicked(bool)),SLOT(calibration_imu_manualcontrol()));
    connect(ui->bAutoControl,SIGNAL(clicked(bool)),SLOT(calibration_imu_autocontrol()));
    connect(ui->bPanServoReset,SIGNAL(clicked(bool)),SLOT(pan_servo_reset()));
    connect(ui->bRollServoReset,SIGNAL(clicked(bool)),SLOT(roll_servo_reset()));
    connect(ui->bTiltServoReset,SIGNAL(clicked(bool)),SLOT(tilt_servo_reset()));
    connect(timer_100ms,SIGNAL(timeout()),this,SLOT(update_ui()));

    timer_10ms->start(10);
    timer_50ms->start(50);
    timer_100ms->start(100);
    timer_1000ms->start(1000);
    timer_5000ms->start(5000);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::start_udp()
{
    if(udp_connection_active == false)
    {
        udp_transmitter.set_debugmode(false);
        udp_receiver.set_debug_mode(false);
        udp_receiver.set_server_info(ui->tRecvServer->toPlainText(),uint16_t(ui->tRecvPort->toPlainText().toLong()));
        udp_transmitter.set_server_info(ui->tSendServer->toPlainText(),uint16_t(ui->tSendPort->toPlainText().toLong()));
        udp_transmitter.Start();
        udp_receiver.Start();
        udp_connection_active = true;
        connect(timer_100ms,SIGNAL(timeout()),this,SLOT(send_Heartbeat_message()));
        connect(&udp_receiver,SIGNAL(new_diagnosticmessage(Diagnostic)),this,SLOT(update_messageviewer(Diagnostic)));
        connect(&udp_receiver,SIGNAL(new_armedstatusmessage(ArmedState)),this,SLOT(update_messageviewer(ArmedState)));
        ui->bConnect->setStyleSheet("color: black;"
                                    "background-color: green;"
                                    );
        ui->bConnect->setText("Disconnect");
    }
    else
    {
        calibration_imu_manualcontrol_running = false;
        calibration_imu_autocontrol_running = false;
        udp_transmitter.Stop();
        udp_receiver.Stop();
        udp_connection_active = false;
        disconnect(timer_100ms,SIGNAL(timeout()),this,SLOT(send_Heartbeat_message()));
        disconnect(&udp_receiver,SIGNAL(new_diagnosticmessage(Diagnostic)),this,SLOT(update_messageviewer(Diagnostic)));
        disconnect(&udp_receiver,SIGNAL(new_armedstatusmessage(ArmedState)),this,SLOT(update_messageviewer(ArmedState)));
        ui->bConnect->setStyleSheet("color: black;"
                                    "background-color: gray;"
                                    );
        ui->bConnect->setText("Connect");
    }
}
void MainWindow::send_Heartbeat_message()
{
    //qDebug() << "Name: " << DeviceName;
    QDateTime currentdatetime = QDateTime::currentDateTime();
    quint64 unixtime = quint64(currentdatetime.toMSecsSinceEpoch());
    quint64 unixtime2 = unixtime + 100; //Should be 100 mS into the future
    new_udpmsgsent(UDP_Heartbeat_ID);
    udp_transmitter.send_Heartbeat_0xAB31(DeviceName.toStdString(),unixtime,unixtime2);
}
void MainWindow::update_messageviewer(const Diagnostic &diag)
{
    new_udpmsgreceived(UDP_Diagnostic_ID);
    if(diag.Level > INFO)
    {
        QString tempstr = "[" + QTime::currentTime().toString() + " " + QString::fromStdString(diag.NodeName) + "] " + QString::fromStdString(diag.Description);
        ui->tInfo->append(tempstr);
    }
}
void MainWindow::update_messageviewer(const ArmedState &state)
{
    new_udpmsgreceived(UDP_Arm_Status_ID);
    QString tempstr = "[" + QTime::currentTime().toString() + "] Armed State: " + QString::number(state.state);
    ui->tInfo->append(tempstr);
}
bool MainWindow::new_udpmsgsent(std::string id)
{
    if(udp_connection_active == false)
    {
        return true;
    }
    bool found = false;
    for(std::size_t i = 0; i < udp_messages.size(); i++)
    {
        if(udp_messages.at(i).id == id)
        {
            udp_messages.at(i).tx_count++;
            found = true;
        }
    }
    return found;
}

bool MainWindow::new_udpmsgreceived(std::string id)
{
    if(udp_connection_active == false)
    {
        return true;
    }
    bool found = false;
    for(std::size_t i = 0; i < udp_messages.size(); i++)
    {
        if(udp_messages.at(i).id == id)
        {
            udp_messages.at(i).rx_count++;
            found = true;
        }
    }
    return found;
}
void MainWindow::calibration_imu_autocontrol()
{
    if(udp_connection_active == false)
    {
        ui->bAutoControl->setStyleSheet("color: black;"
                                    "background-color: red;"
                                    );
        return;
    }
    pan_servo_reset();
    roll_servo_reset();
    tilt_servo_reset();
     cycle_count = 0;
    calibration_imu_autocontrol_running = !calibration_imu_autocontrol_running;
    calibration_imu_preview_running = false;
    calibration_imu_manualcontrol_running = false;
    pan_servo_min = ui->tPanServoMin->toPlainText().toInt();
    pan_servo_max = ui->tPanServoMax->toPlainText().toInt();
    pan_servo_duration = ui->tPanServoTime->toPlainText().toDouble();

    roll_servo_min = ui->tRollServoMin->toPlainText().toInt();
    roll_servo_max = ui->tRollServoMax->toPlainText().toInt();
    roll_servo_duration = ui->tRollServoTime->toPlainText().toDouble();

    tilt_servo_min = ui->tTiltServoMin->toPlainText().toInt();
    tilt_servo_max = ui->tTiltServoMax->toPlainText().toInt();
    tilt_servo_duration = ui->tTiltServoTime->toPlainText().toDouble();

    if(ui->cbParallelOperation->isChecked() == true)
    {
        servo_to_update =0;
    }
    else
    {
        servo_to_update=1;
    }
    if(calibration_imu_autocontrol_running == true)
    {
        calibration_imu_lasttimeran.start();
        connect(timer_50ms,SIGNAL(timeout()),this,SLOT(update_calibration_imu()));
        connect(timer_50ms,SIGNAL(timeout()),this,SLOT(send_RemoteControl_message()));
        ui->bManualControl->setStyleSheet("color: black;"
                                          "background-color: gray;"
                                          );
        ui->bPreview->setStyleSheet("color: black;"
                                    "background-color: gray;"
                                    );
        ui->bAutoControl->setStyleSheet("color: black;"
                                    "background-color: green;"
                                    );
    }
    else
    {
        disconnect(timer_50ms,SIGNAL(timeout()),this,SLOT(update_calibration_imu()));
        disconnect(timer_50ms,SIGNAL(timeout()),this,SLOT(send_RemoteControl_message()));
        ui->bManualControl->setStyleSheet("color: black;"
                                          "background-color: gray;"
                                          );
        ui->bPreview->setStyleSheet("color: black;"
                                    "background-color: gray;"
                                    );
        ui->bAutoControl->setStyleSheet("color: black;"
                                    "background-color: gray;"
                                    );
    }
}
void MainWindow::calibration_imu_preview()
{
    pan_servo_reset();
    roll_servo_reset();
    tilt_servo_reset();
     cycle_count = 0;
    calibration_imu_preview_running = !calibration_imu_preview_running;
    calibration_imu_manualcontrol_running = false;
    pan_servo_min = ui->tPanServoMin->toPlainText().toInt();
    pan_servo_max = ui->tPanServoMax->toPlainText().toInt();
    pan_servo_duration = ui->tPanServoTime->toPlainText().toDouble();

    roll_servo_min = ui->tRollServoMin->toPlainText().toInt();
    roll_servo_max = ui->tRollServoMax->toPlainText().toInt();
    roll_servo_duration = ui->tRollServoTime->toPlainText().toDouble();

    tilt_servo_min = ui->tTiltServoMin->toPlainText().toInt();
    tilt_servo_max = ui->tTiltServoMax->toPlainText().toInt();
    tilt_servo_duration = ui->tTiltServoTime->toPlainText().toDouble();

    if(ui->cbParallelOperation->isChecked() == true)
    {
        servo_to_update =0;
    }
    else
    {
        servo_to_update=1;
    }
    if(calibration_imu_preview_running == true)
    {
        calibration_imu_lasttimeran.start();
        connect(timer_50ms,SIGNAL(timeout()),this,SLOT(update_calibration_imu()));

        ui->bManualControl->setStyleSheet("color: black;"
                                          "background-color: gray;"
                                          );
        ui->bPreview->setStyleSheet("color: black;"
                                    "background-color: green;"
                                    );
        ui->bAutoControl->setStyleSheet("color: black;"
                                    "background-color: gray;"
                                    );
    }
    else
    {
        disconnect(timer_50ms,SIGNAL(timeout()),this,SLOT(update_calibration_imu()));

        ui->bManualControl->setStyleSheet("color: black;"
                                          "background-color: gray;"
                                          );
        ui->bPreview->setStyleSheet("color: black;"
                                    "background-color: gray;"
                                    );
        ui->bAutoControl->setStyleSheet("color: black;"
                                    "background-color: gray;"
                                    );
    }
}
void MainWindow::calibration_imu_manualcontrol()
{
    pan_servo_reset();
    roll_servo_reset();
    tilt_servo_reset();
    cycle_count = 0;
    calibration_imu_manualcontrol_running = !calibration_imu_manualcontrol_running;
    calibration_imu_preview_running = false;
    if(calibration_imu_manualcontrol_running == true)
    {
        connect(timer_50ms,SIGNAL(timeout()),this,SLOT(update_calibration_imu()));
        connect(timer_50ms,SIGNAL(timeout()),this,SLOT(send_RemoteControl_message()));
        ui->bManualControl->setStyleSheet("color: black;"
                                          "background-color: green;"
                                          );
        ui->bPreview->setStyleSheet("color: black;"
                                    "background-color: gray;"
                                    );
        ui->bAutoControl->setStyleSheet("color: black;"
                                    "background-color: gray;"
                                    );
    }
    else
    {
        disconnect(timer_50ms,SIGNAL(timeout()),this,SLOT(update_calibration_imu()));
        disconnect(timer_50ms,SIGNAL(timeout()),this,SLOT(send_RemoteControl_message()));
        ui->bManualControl->setStyleSheet("color: black;"
                                          "background-color: gray;"
                                          );
        ui->bPreview->setStyleSheet("color: black;"
                                    "background-color: gray;"
                                    );
        ui->bAutoControl->setStyleSheet("color: black;"
                                    "background-color: gray;"
                                    );
    }
}
void MainWindow::update_calibration_imu()
{
    if((calibration_imu_preview_running == true) or (calibration_imu_autocontrol_running == true))
    {

        double dt = double(calibration_imu_lasttimeran.elapsed()/1000.0);
        if((servo_to_update == 0) or (servo_to_update == 1))
        {
            double dv = double(pan_servo_max-pan_servo_min);
            double dE =double(1.0/pan_servo_duration)*double(dt*dv);
            if(pan_servo_direction == false) //Normal
            {

                pan_servo_value+=dE;
                if(pan_servo_value >= pan_servo_max)
                {
                    pan_servo_direction = true;
                    pan_servo_value = double(pan_servo_max);
                }
            }
            else
            {
                pan_servo_value-=dE;
                if(pan_servo_value <= pan_servo_min)
                {
                    pan_servo_direction = false;
                    pan_servo_value = double(pan_servo_min);
                    if(servo_to_update == 1) { servo_to_update++; }
                }
            }
        }
        if((servo_to_update == 0) or (servo_to_update == 2))
        {
            double dv = double(roll_servo_max-roll_servo_min);
            double dE =double(1.0/roll_servo_duration)*double(dt*dv);
            if(roll_servo_direction == false) //Normal
            {

                roll_servo_value+=dE;
                if(roll_servo_value >= roll_servo_max)
                {
                    roll_servo_direction = true;
                    roll_servo_value = double(roll_servo_max);
                }
            }
            else
            {
                roll_servo_value-=dE;
                if(roll_servo_value <= roll_servo_min)
                {
                    roll_servo_direction = false;
                    roll_servo_value = double(roll_servo_min);
                    if(servo_to_update == 2) { servo_to_update++; }
                }
            }
        }
        if((servo_to_update == 0) or (servo_to_update == 3))
        {
            double dv = double(tilt_servo_max-tilt_servo_min);
            double dE =double(1.0/tilt_servo_duration)*double(dt*dv);
            if(tilt_servo_direction == false) //Normal
            {

                tilt_servo_value+=dE;
                if(tilt_servo_value >= tilt_servo_max)
                {
                    tilt_servo_direction = true;
                    tilt_servo_value = double(tilt_servo_max);
                }
            }
            else
            {
                tilt_servo_value-=dE;
                if(tilt_servo_value <= tilt_servo_min)
                {
                    tilt_servo_direction = false;
                    tilt_servo_value = double(tilt_servo_min);
                    cycle_count++;
                    ui->tInfo->append("[" + QTime::currentTime().toString() + "] Cycle Count: " + QString::number(cycle_count));
                    if(servo_to_update == 3) { servo_to_update = 1; }
                }
            }
        }
    }


    else if(calibration_imu_manualcontrol_running == true)
    {
        pan_servo_value = ui->sPanServo->value();
        roll_servo_value = ui->sRollServo->value();
        tilt_servo_value = ui->sTiltServo->value();
    }

    ui->lPanServo->setText("PanServo: " + QString::number(int(pan_servo_value)));
    ui->sPanServo->setValue(int(pan_servo_value));
    ui->lRollServo->setText("RollServo: " + QString::number(int(roll_servo_value)));
    ui->sRollServo->setValue(int(roll_servo_value));
    ui->lTiltServo->setText("TiltServo: " + QString::number(int(tilt_servo_value)));
    ui->sTiltServo->setValue(int(tilt_servo_value));
    calibration_imu_lasttimeran.start();
}
void MainWindow::send_RemoteControl_message()
{
    if((calibration_imu_manualcontrol_running == true) or (calibration_imu_autocontrol_running == true))
    {
        QDateTime currentdatetime = QDateTime::currentDateTime();
        quint64 unixtime = quint64(currentdatetime.toMSecsSinceEpoch());
        new_udpmsgsent(UDP_RemoteControl_ID);
        udp_transmitter.send_RemoteControl_0xAB10(unixtime,
                                                  int(327.68*double(pan_servo_value)),
                                                  int(327.68*double(roll_servo_value)),
                                                  int(327.68*double(tilt_servo_value)),
                                                  0,0,0,0,0,0,0,0,0,0,0,0,0);
    }
}
void MainWindow::update_ui()
{
    ui->tMessageInfo->clear();
    for(std::size_t i = 0; i < udp_messages.size(); ++i)
    {
        ui->tMessageInfo->append(QString::fromStdString(udp_messages.at(i).name) + "(0x" + QString::fromStdString(udp_messages.at(i).id) + "): TX: " +
                                 QString::number(udp_messages.at(i).tx_count) + " RX: " + QString::number(udp_messages.at(i).rx_count));
    }
}
void MainWindow::init_udpmessageinfo()
{
    {
        UDPMessageInfo msg;
        msg.id = UDP_Command_ID;
        msg.name = "Command";
        udp_messages.push_back(msg);
    }
    {
        UDPMessageInfo msg;
        msg.id = UDP_RemoteControl_ID;
        msg.name = "Remote Control";
        udp_messages.push_back(msg);
    }


    {
        UDPMessageInfo msg;
        msg.id = UDP_Arm_Status_ID;
        msg.name = "Arm Status";
        udp_messages.push_back(msg);
    }
    {
        UDPMessageInfo msg;
        msg.id = UDP_Heartbeat_ID;
        msg.name = "Heartbeat";
        udp_messages.push_back(msg);
    }



    for(std::size_t i = 0; i < udp_messages.size(); i++)
    {
        udp_messages.at(i).rx_count = 0;
        udp_messages.at(i).tx_count = 0;
    }
}
void MainWindow::pan_servo_reset()
{
    pan_servo_value = 0.0;
}
void MainWindow::roll_servo_reset()
{
    roll_servo_value = 0.0;
}
void MainWindow::tilt_servo_reset()
{
    tilt_servo_value = 0.0;
}
