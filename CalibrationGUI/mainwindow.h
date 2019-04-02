#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QNetworkInterface>
#include "../../gui_common/include/helper.h"
#include "../../gui_common/include/udptransmitter.h"
#include "../../gui_common/include/udpreceiver.h"

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
    void update_messageviewer(const Diagnostic &diag);
    void update_messageviewer(const ArmedState &state);
    void start_udp();

private:
    bool new_udpmsgsent(std::string id);
    void init_udpmessageinfo();

    std::vector<UDPMessageInfo> udp_messages;
    Ui::MainWindow *ui;
    QString DeviceName;
    UDPTransmitter udp_transmitter;
    UDPReceiver udp_receiver;

    QTimer *timer_10ms;
    QTimer *timer_50ms;
    QTimer *timer_100ms;
    QTimer *timer_1000ms;
    QTimer *timer_5000ms;
};

#endif // MAINWINDOW_H
