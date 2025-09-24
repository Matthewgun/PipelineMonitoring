#include "pipeline_widget.h"
#include <QPainter>
#include <QPainterPath> // Добавляем этот include

PipelineWidget::PipelineWidget(QWidget *parent) : QWidget(parent)
{
    // Инициализация сегментов
    segments = {
        {"20 км", 0, 0, "Неизвестно", Qt::gray, QRectF()},
        {"50 км", 1, 0, "Неизвестно", Qt::gray, QRectF()},
        {"82 км", 2, 0, "Неизвестно", Qt::gray, QRectF()},
        {"110 км", 3, 0, "Неизвестно", Qt::gray, QRectF()},
        {"141 км", 4, 0, "Неизвестно", Qt::gray, QRectF()},
        {"170 км", 5, 0, "Неизвестно", Qt::gray, QRectF()}
    };

    setMinimumSize(400, 400);
}

void PipelineWidget::updateSegment(int index, quint16 value, const QString &status, const QColor &color)
{
    if (index >= 0 && index < segments.size()) {
        segments[index].value = value;
        segments[index].status = status;
        segments[index].color = color;
        update(); // Перерисовываем виджет
    }
}

void PipelineWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Фон
    painter.fillRect(rect(), Qt::white);

    // Рисуем трубопровод
    calculateSegments();

    // Основная труба
    QPen pipePen(Qt::darkGray, 20, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(pipePen);

    // Рисуем линию трубопровода
    painter.drawLine(50, height() / 2, width() - 50, height() / 2);

    // Сегменты с датчиками
    for (int i = 0; i < segments.size(); ++i) {
        const auto &segment = segments[i];

        // Клапан/датчик
        painter.setPen(Qt::black);
        painter.setBrush(segment.color);

        // Рисуем круг для датчика
        QPointF center = segment.rect.center();
        painter.drawEllipse(center, 15, 15);

        // Текст с названием
        painter.setPen(Qt::black);
        painter.drawText(QRectF(center.x() - 40, center.y() - 40, 80, 20),
                         Qt::AlignCenter, segment.name);

        // Текст со значением
        painter.drawText(QRectF(center.x() - 40, center.y() + 20, 80, 20),
                         Qt::AlignCenter, QString::number(segment.value));
    }
}

void PipelineWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    calculateSegments();
}

void PipelineWidget::calculateSegments()
{
    int segmentWidth = (width() - 100) / segments.size();

    for (int i = 0; i < segments.size(); ++i) {
        segments[i].rect = QRectF(50 + i * segmentWidth,
                                  height() / 2 - 30,
                                  segmentWidth,
                                  60);
    }
}
