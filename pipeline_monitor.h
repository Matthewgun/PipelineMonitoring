#ifndef PIPELINE_MONITOR_H
#define PIPELINE_MONITOR_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include <QVector>
#include <QProgressBar> // Добавляем include
#include <QLabel>

// Предварительное объявление класса
class PipelineWidget;

struct MonitoringPoint {
    QString name;
    int address;
    QProgressBar *valueBar;
    QLabel *valueLabel;
    QLabel *statusLabel;
    quint16 value;
};

class PipelineMonitor : public QMainWindow
{
    Q_OBJECT

public:
    PipelineMonitor(QWidget *parent = nullptr);
    ~PipelineMonitor();

private slots:
    void connectToModbus();
    void disconnectFromModbus();
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void readModbusData();
    void onReadyRead();

private:
    void setupUI();
    void setupMonitoringPoints();
    QByteArray createModbusRequest(int address);
    void parseModbusResponse(const QByteArray &data);
    void updatePointDisplay(int index, quint16 value);
    QString getStatusFromValue(quint16 value);
    QColor getColorFromStatus(const QString &status);

    QTcpSocket *tcpSocket;
    QTimer *pollTimer;
    QVector<MonitoringPoint> monitoringPoints;
    PipelineWidget *pipelineWidget; // Указатель на виджет трубопровода
};

#endif // PIPELINE_MONITOR_H
