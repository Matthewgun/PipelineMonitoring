#ifndef PIPELINE_WIDGET_H
#define PIPELINE_WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QVector>
#include <QPainterPath> // Добавляем этот include
#include <QRectF>
#include <QColor>


struct PipelineSegment {
    QString name;
    int address;
    quint16 value;
    QString status;
    QColor color;
    QRectF rect;
};

class PipelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PipelineWidget(QWidget *parent = nullptr);

    void updateSegment(int index, quint16 value, const QString &status, const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void calculateSegments();
    QVector<PipelineSegment> segments;
};

#endif // PIPELINE_WIDGET_H
