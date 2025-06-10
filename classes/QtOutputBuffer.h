
#pragma once
#include <QtWidgets/QTextEdit>
#include <streambuf>
#include <string>

class QtOutputBuffer : public std::streambuf {
private:
    QTextEdit* textEdit;
    std::string buffer;

public:
    explicit QtOutputBuffer(QTextEdit* edit);
protected:
    virtual int_type overflow(int_type c) override;
};