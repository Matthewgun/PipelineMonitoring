#include "pipeline_monitor.h"
#include "pipeline_widget.h" // Добавляем include для кастомного виджета

#include <QTcpSocket>
#include <QMessageBox>
#include <QApplication>
#include <QDataStream>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QStatusBar>

PipelineMonitor::PipelineMonitor(QWidget *parent)
    : QMainWindow(parent)
    , tcpSocket(nullptr)
    , pollTimer(new QTimer(this))
    , pipelineWidget(nullptr) // Инициализируем указатель
{
    setWindowTitle("Мониторинг трубопровода");
    resize(1000, 700);

    setupUI();
    setupMonitoringPoints();

    pollTimer->setInterval(2000);
    connect(pollTimer, &QTimer::timeout, this, &PipelineMonitor::readModbusData);
}

PipelineMonitor::~PipelineMonitor()
{
    disconnectFromModbus();
    delete pipelineWidget; // Не забываем освободить память
}
void PipelineMonitor::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Левая панель - точки мониторинга
    QGroupBox *pointsGroup = new QGroupBox("Точки мониторинга", this);
    QVBoxLayout *pointsLayout = new QVBoxLayout(pointsGroup);

    // Панель управления
    QHBoxLayout *controlLayout = new QHBoxLayout();
    QPushButton *connectButton = new QPushButton("Подключиться", this);
    QPushButton *disconnectButton = new QPushButton("Отключиться", this);

    connect(connectButton, &QPushButton::clicked, this, &PipelineMonitor::connectToModbus);
    connect(disconnectButton, &QPushButton::clicked, this, &PipelineMonitor::disconnectFromModbus);

    controlLayout->addWidget(connectButton);
    controlLayout->addWidget(disconnectButton);
    controlLayout->addStretch();

    pointsLayout->addLayout(controlLayout);

    // Таблица точек мониторинга
    QGridLayout *gridLayout = new QGridLayout();

    // Заголовки
    gridLayout->addWidget(new QLabel("Участок"), 0, 0);
    gridLayout->addWidget(new QLabel("Индекс"), 0, 1);
    gridLayout->addWidget(new QLabel("% от НДД"), 0, 2);
    gridLayout->addWidget(new QLabel("Статус"), 0, 3);

    pointsLayout->addLayout(gridLayout);
    pointsGroup->setLayout(pointsLayout);

    // Правая панель - визуализация трубопровода
    QGroupBox *pipelineGroup = new QGroupBox("Визуализация трубопровода", this);
    QVBoxLayout *pipelineLayout = new QVBoxLayout(pipelineGroup);

    // Создаем кастомный виджет
    pipelineWidget = new PipelineWidget();
    pipelineLayout->addWidget(pipelineWidget);
    pipelineGroup->setLayout(pipelineLayout);

    // Добавляем обе панели в главный layout
    mainLayout->addWidget(pointsGroup, 1);
    mainLayout->addWidget(pipelineGroup, 2);

    setCentralWidget(centralWidget);
    statusBar()->showMessage("Не подключено");
}

void PipelineMonitor::setupMonitoringPoints()
{
    monitoringPoints = {
        {"20 км", 0, nullptr, nullptr, nullptr, 0},
        {"50 км", 1, nullptr, nullptr, nullptr, 0},
        {"82 км", 2, nullptr, nullptr, nullptr, 0},
        {"110 км", 3, nullptr, nullptr, nullptr, 0},
        {"141 км", 4, nullptr, nullptr, nullptr, 0},
        {"170 км", 5, nullptr, nullptr, nullptr, 0}
    };

    QGroupBox *pointsGroup = findChild<QGroupBox*>();
    QVBoxLayout *pointsLayout = qobject_cast<QVBoxLayout*>(pointsGroup->layout());
    QGridLayout *gridLayout = qobject_cast<QGridLayout*>(pointsLayout->itemAt(1)->layout());

    for (int i = 0; i < monitoringPoints.size(); ++i) {
        int row = i + 1;

        gridLayout->addWidget(new QLabel(monitoringPoints[i].name), row, 0);
        gridLayout->addWidget(new QLabel(QString::number(monitoringPoints[i].address)), row, 1);

        QLabel *valueLabel = new QLabel("N/A");
        gridLayout->addWidget(valueLabel, row, 2);
        monitoringPoints[i].valueLabel = valueLabel;

        QLabel *statusLabel = new QLabel("Неизвестно");
        statusLabel->setAlignment(Qt::AlignCenter);
        statusLabel->setStyleSheet("background-color: gray; color: white; padding: 5px;");
        gridLayout->addWidget(statusLabel, row, 3);
        monitoringPoints[i].statusLabel = statusLabel;
    }
}

void PipelineMonitor::connectToModbus()
{
    if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState) {
        return;
    }

    if (!tcpSocket) {
        tcpSocket = new QTcpSocket(this);
        connect(tcpSocket, &QTcpSocket::connected, this, &PipelineMonitor::onConnected);
        connect(tcpSocket, &QTcpSocket::disconnected, this, &PipelineMonitor::onDisconnected);
        connect(tcpSocket, &QTcpSocket::errorOccurred, this, &PipelineMonitor::onErrorOccurred);
        connect(tcpSocket, &QTcpSocket::readyRead, this, &PipelineMonitor::onReadyRead);
    }

    statusBar()->showMessage("Подключение...");
    tcpSocket->connectToHost("127.0.0.1", 502);
}

void PipelineMonitor::disconnectFromModbus()
{
    if (tcpSocket) {
        tcpSocket->disconnectFromHost();
    }
    pollTimer->stop();
    statusBar()->showMessage("Отключено");
}

void PipelineMonitor::onConnected()
{
    statusBar()->showMessage("Подключено к Modbus серверу");
    pollTimer->start();
}

void PipelineMonitor::onDisconnected()
{
    statusBar()->showMessage("Отключено");
    pollTimer->stop();
}

void PipelineMonitor::onErrorOccurred(QAbstractSocket::SocketError error)
{
    statusBar()->showMessage(QString("Ошибка: %1").arg(tcpSocket->errorString()));
}

void PipelineMonitor::readModbusData()
{
    if (!tcpSocket || tcpSocket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    // Читаем все точки мониторинга
    for (const auto &point : monitoringPoints) {
        QByteArray request = createModbusRequest(point.address);
        tcpSocket->write(request);
    }
}

QByteArray PipelineMonitor::createModbusRequest(int address)
{
    QByteArray request;
    QDataStream stream(&request, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    // Modbus TCP заголовок
    stream << quint16(0x0001); // Transaction ID
    stream << quint16(0x0000); // Protocol ID
    stream << quint16(0x0006); // Length
    stream << quint8(0x01);    // Unit ID

    // Modbus PDU - читаем 6 регистров начиная с адреса 0
    stream << quint8(0x03);    // Function code (Read Holding Registers)
    stream << quint16(0);      // Starting address (всегда 0)
    stream << quint16(6);      // Quantity of registers (все 6 точек)

    return request;
}

void PipelineMonitor::onReadyRead()
{
    QByteArray data = tcpSocket->readAll();
    parseModbusResponse(data);
}

void PipelineMonitor::parseModbusResponse(const QByteArray &data)
{
    if (data.size() < 11) return; // Минимальный размер для 6 регистров

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);

    quint16 transactionId, protocolId, length;
    quint8 unitId;

    stream >> transactionId >> protocolId >> length >> unitId;

    quint8 functionCode;
    stream >> functionCode;

    if (functionCode == 0x03) { // Read Holding Registers response
        quint8 byteCount;
        stream >> byteCount;

        if (byteCount >= 12) { // 6 регистров * 2 байта
            // Читаем все 6 значений
            for (int i = 0; i < 6; i++) {
                quint16 value;
                stream >> value;
                updatePointDisplay(i, value);
            }
        }
    }
}

void PipelineMonitor::updatePointDisplay(int index, quint16 value)
{
    if (index < 0 || index >= monitoringPoints.size()) return;

    MonitoringPoint &point = monitoringPoints[index];
    point.value = value;

    point.valueLabel->setText(QString::number(value));

    QString status = getStatusFromValue(value);
    point.statusLabel->setText(status);

    QColor color = getColorFromStatus(status);
    point.statusLabel->setStyleSheet(
        QString("background-color: %1; color: white; padding: 5px;")
            .arg(color.name())
        );

    // Обновляем визуализацию трубопровода
    if (pipelineWidget) {
        pipelineWidget->updateSegment(index, value, status, color);
    }
}

QString PipelineMonitor::getStatusFromValue(quint16 value)
{
    if (value < 20) return "Низкий";
    if (value < 50) return "Норма";
    if (value < 80) return "Высокий";
    return "Авария";
}

QColor PipelineMonitor::getColorFromStatus(const QString &status)
{
    if (status == "Низкий") return Qt::yellow;
    if (status == "Норма") return Qt::green;
    if (status == "Высокий") return QColor(255, 165, 0);
    if (status == "Авария") return Qt::red;
    return Qt::gray;
}
