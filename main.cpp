#include <QApplication>
#include "pipeline_monitor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    PipelineMonitor monitor;
    monitor.show();

    return app.exec();
}
