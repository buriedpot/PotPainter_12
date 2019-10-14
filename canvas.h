#ifndef CANVAS_H
#define CANVAS_H

#include <QObject>
#include <QWidget>

class Canvas : public QWidget
{
    Q_OBJECT
public:
    explicit Canvas(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // CANVAS_H
