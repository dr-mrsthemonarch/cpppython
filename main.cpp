#include "classes/DataAnalysisApp.h"
#include "classes/MainWindow.h"
#include <QtWidgets/QMessageBox>

int main(int argc, char* argv[]) {
    try {
        DataAnalysisApp app(argc, argv);
        MainWindow window;
        window.show();
        return app.exec();
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Initialization Error",
                            QString("Failed to initialize: %1").arg(e.what()));
        return -1;
    }
}