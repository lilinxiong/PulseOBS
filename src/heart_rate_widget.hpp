#ifndef HEART_RATE_WIDGET_H
#define HEART_RATE_WIDGET_H

#include <QWidget>
#include <QLabel>

class HeartRateWidget : public QWidget {
    Q_OBJECT

  public:
    HeartRateWidget(QWidget *parent = nullptr);
    ~HeartRateWidget();

    void updateHeartRate(int heartRate);

  private:
    QLabel *heartRateLabel;
};

#endif