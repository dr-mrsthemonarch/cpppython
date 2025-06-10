#include "../classes/QtOutputBuffer.h"

QtOutputBuffer::QtOutputBuffer(QTextEdit* edit) : textEdit(edit) {
    connect(this, &QtOutputBuffer::textReady,
            this, &QtOutputBuffer::handleText,
            Qt::QueuedConnection);
}

QtOutputBuffer::~QtOutputBuffer() {
    // Flush any remaining content when the buffer is destroyed
    sync();
}

QtOutputBuffer::int_type QtOutputBuffer::overflow(int_type c) {
    if (c != EOF) {
        buffer += static_cast<char>(c);

        // Flush on newline, carriage return, or when buffer gets large
        // Also flush on certain formatting characters that often end output blocks
        if (c == '\n' || c == '\r' || buffer.size() >= 256 ||
            (buffer.size() > 50 && (c == '=' || c == '-' || c == ' '))) {
            flushBuffer();
            }
    }
    return c;
}

int QtOutputBuffer::sync() {
    // Always flush remaining content when sync is called
    flushBuffer();
    return 0;
}

void QtOutputBuffer::flushBuffer() {
    if (!buffer.empty()) {
        Q_EMIT textReady(QString::fromStdString(buffer));
        buffer.clear();
    }
}

void QtOutputBuffer::handleText(const QString& text) {
    if (textEdit) {
        // Move to end and insert text
        textEdit->moveCursor(QTextCursor::End);
        textEdit->insertPlainText(text);
        textEdit->moveCursor(QTextCursor::End);

        // Ensure the text is immediately visible
        textEdit->ensureCursorVisible();

        // Process events to update the display immediately
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}