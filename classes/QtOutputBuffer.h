#pragma once

#include <QObject>
#include <QTextEdit>
#include <QTextCursor>
#include <QApplication>
#include <streambuf>
#include <string>

class QtOutputBuffer : public QObject, public std::streambuf {
    Q_OBJECT

public:
    using int_type = std::streambuf::int_type;

    explicit QtOutputBuffer(QTextEdit* edit);
    ~QtOutputBuffer();

protected:
    int_type overflow(int_type c) override;
    int sync() override;

private:
    void flushBuffer();

    QTextEdit* textEdit;
    std::string buffer;

    Q_SIGNALS:
        void textReady(const QString& text);

private Q_SLOTS:
    void handleText(const QString& text);
};