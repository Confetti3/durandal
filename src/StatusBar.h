#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QStatusBar>
#include <QLabel>

class StatusBar : public QStatusBar
{
    Q_OBJECT

public:
    explicit StatusBar(QWidget* parent = nullptr);

public slots:
    void setCursorPosition(int line, int column);
    void setWordCount(int count);
    void setFileName(const QString& name);
    void setFileType(const QString& type);
    void setModified(bool modified);
    void clear();

private:
    QLabel* m_lineColLabel;
    QLabel* m_wordCountLabel;
    QLabel* m_fileTypeLabel;
    QLabel* m_modifiedLabel;
    QLabel* m_fileNameLabel;

    void setupUI();
};

#endif // STATUSBAR_H
