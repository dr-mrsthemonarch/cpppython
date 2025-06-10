#include "../classes/QtOutputBuffer.h"

QtOutputBuffer::QtOutputBuffer(QTextEdit* edit) : textEdit(edit) {}

QtOutputBuffer::int_type QtOutputBuffer::overflow(int_type c) {
    if (c != EOF) {
        buffer += static_cast<char>(c);
        if (c == '\n') {
            // Emit signal to update GUI in main thread
            QMetaObject::invokeMethod(textEdit, [this]() {
                textEdit->append(QString::fromStdString(buffer.substr(0, buffer.length()-1)));
                textEdit->ensureCursorVisible();
            }, Qt::QueuedConnection);
            buffer.clear();
        }
    }
    return c;
}