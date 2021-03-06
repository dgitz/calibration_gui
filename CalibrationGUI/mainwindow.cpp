#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tRecvPort->setText("45454");
    ui->tRecvServer->setText("239.255.43.21");
    ui->tSendPort->setText("5678");
    ui->tSendServer->setText("127.0.0.1");
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
    udp_transmitter.set_debugmode(false);
    udp_receiver.set_debug_mode(false);
    udp_receiver.set_server_info(ui->tRecvServer->toPlainText(),(uint16_t)(ui->tRecvPort->toPlainText().toLong()));
    udp_transmitter.set_server_info(ui->tSendServer->toPlainText(),(uint16_t)(ui->tSendPort->toPlainText().toLong()));
    udp_transmitter.Start();
    udp_receiver.Start();

    connect(timer_100ms,SIGNAL(timeout()),this,SLOT(send_Heartbeat_message()));
    connect(&udp_receiver,SIGNAL(new_diagnosticmessage(Diagnostic)),this,SLOT(update_messageviewer(Diagnostic)));
    connect(&udp_receiver,SIGNAL(new_armedstatusmessage(ArmedState)),this,SLOT(update_messageviewer(ArmedState)));
}
void MainWindow::send_Heartbeat_message()
{
    //qDebug() << "Name: " << DeviceName;
    QDateTime currentdatetime = QDateTime::currentDateTime();
    quint64 unixtime = currentdatetime.toMSecsSinceEpoch();
    quint64 unixtime2 = unixtime + 100; //Should be 100 mS into the future
    new_udpmsgsent(UDP_Heartbeat_ID);
    udp_transmitter.send_Heartbeat_0xAB31(DeviceName.toStdString(),unixtime,unixtime2);
}
void MainWindow::update_messageviewer(const Diagnostic &diag)
{
    //new_udpmsgreceived(UDP_Diagnostic_ID);
    if(diag.Level > INFO)
    {
        QString tempstr = "[" + QTime::currentTime().toString() + " " + QString::fromStdString(diag.NodeName) + "] " + QString::fromStdString(diag.Description);
        ui->tInfo->append(tempstr);
    }
}
void MainWindow::update_messageviewer(const ArmedState &state)
{
    //new_udpmsgreceived(UDP_Diagnostic_ID);
        QString tempstr = "[" + QTime::currentTime().toString() + " Armed State: " + QString::number(state.state);
        ui->tInfo->append(tempstr);
}
bool MainWindow::new_udpmsgsent(std::string id)
{
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
        msg.id = UDP_Resource_ID;
        msg.name = "Resource Info";
        udp_messages.push_back(msg);
    }
    {
        UDPMessageInfo msg;
        msg.id = UDP_Diagnostic_ID;
        msg.name = "Diagnostic";
        udp_messages.push_back(msg);
    }
    {
        UDPMessageInfo msg;
        msg.id = UDP_Device_ID;
        msg.name = "Device Info";
        udp_messages.push_back(msg);
    }
    {
        UDPMessageInfo msg;
        msg.id = UDP_ArmControl_ID;
        msg.name = "Arm Control";
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
    {
        UDPMessageInfo msg;
        msg.id = UDP_FindTarget_ID;
        msg.name = "Find Target";
        udp_messages.push_back(msg);
    }
    {
        UDPMessageInfo msg;
        msg.id = UDP_Power_ID;
        msg.name = "Power";
        udp_messages.push_back(msg);
    }
    {
        UDPMessageInfo msg;
        msg.id = UDP_TuneControlGroup_ID;
        msg.name = "Tune Control Group";
        udp_messages.push_back(msg);
    }
    {
        UDPMessageInfo msg;
        msg.id = UDP_Firmware_ID;
        msg.name = "Firmware";
        udp_messages.push_back(msg);
    }


    for(std::size_t i = 0; i < udp_messages.size(); i++)
    {
        udp_messages.at(i).rx_count = 0;
        udp_messages.at(i).tx_count = 0;
    }
}
