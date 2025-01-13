#include "heart_rate_widget.hpp"

HeartRateWidget::HeartRateWidget(QWidget *parent) : QWidget(parent) {
  heartRateLabel = new QLabel("Heart Rate: 0", this);
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(heartRateLabel);
  setLayout(layout);
}

void HeartRateWidget::updateHeartRate(int heartRate) {
  heartRateLabel->setText("Heart Rate: " + QString::number(heartRate));
}