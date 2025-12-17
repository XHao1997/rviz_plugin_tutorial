/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2024, Metro Robots
 *  All rights reserved.
 *********************************************************************/

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <rviz_common/display_context.hpp>
#include <rviz_panel_tutorial/demo_panel.hpp>

namespace rviz_panel_tutorial
{

DemoPanel::DemoPanel(QWidget * parent)
: Panel(parent)
{
  auto layout = new QVBoxLayout(this);

  // 主状态 label
  label_ = new QLabel("● Waiting for robot feedback...");
  label_->setAlignment(Qt::AlignCenter);
  label_->setStyleSheet(
      "QLabel {"
      "  color: #999999;"
      "  font-weight: bold;"
      "}"
  );

  // 土壤传感器 label（注意：这里用成员 probe_num_，不是局部变量）
  probe_num_ = new QLabel("Soil Probe #: (no data)");
  probe_num_->setAlignment(Qt::AlignCenter);
  probe_num_->setStyleSheet(
      "QLabel {"
      "  color: #333333;"
      "  font-weight: bold;"
      "}"
  );

  QPushButton * button_home         = new QPushButton("To Home");
  QPushButton * button_plan         = new QPushButton("Plan");
  QPushButton * button_execute      = new QPushButton("Execute");
  QPushButton * button_next_capture = new QPushButton("Next Capture");
  QPushButton * button_soil_plan    = new QPushButton("Soil Plan");
  QPushButton * button_soil_probe   = new QPushButton("Soil Probe");
  QPushButton * button_move_back    = new QPushButton("Move Back");

  auto row1 = new QHBoxLayout();
  row1->addWidget(button_home);
  row1->addWidget(button_plan);
  row1->addWidget(button_execute);

  auto row2 = new QHBoxLayout();
  row2->addWidget(button_next_capture);
  row2->addWidget(button_soil_plan);
  row2->addWidget(button_soil_probe);
  row2->addWidget(button_move_back);

  layout->addWidget(label_);
  layout->addWidget(probe_num_);   // 显示在状态下面
  layout->addLayout(row1);
  layout->addLayout(row2);

  QObject::connect(button_plan, &QPushButton::released,
                   this, &DemoPanel::buttonPlanActivated);
  QObject::connect(button_execute, &QPushButton::released,
                   this, &DemoPanel::buttonExecuteActivated);
  QObject::connect(button_home, &QPushButton::released,
                   this, &DemoPanel::buttonHomeActivated);
  QObject::connect(button_next_capture, &QPushButton::released,
                   this, &DemoPanel::buttonNextCaptureActivated);
  QObject::connect(button_soil_plan, &QPushButton::released,
                   this, &DemoPanel::buttonSoilPlanActivated);
  QObject::connect(button_soil_probe, &QPushButton::released,
                   this, &DemoPanel::buttonSoilProbeActivated);
  QObject::connect(button_move_back, &QPushButton::released,
                   this, &DemoPanel::buttonMoveBackActivated);
}

DemoPanel::~DemoPanel() = default;

void DemoPanel::onInitialize()
{
  node_ptr_ = getDisplayContext()->getRosNodeAbstraction().lock();
  rclcpp::Node::SharedPtr node = node_ptr_->get_raw_node();

  // 发布按钮命令
  publisher_ = node->create_publisher<std_msgs::msg::Int8>("/soil_task_command", 10);

  // 订阅机器人状态反馈
  subscription_ = node->create_subscription<std_msgs::msg::Int8>(
      "/soil_task_feedback", 10,
      std::bind(&DemoPanel::topicCallback, this, std::placeholders::_1));

  // 订阅土壤传感器数据
  soil_sub_ = node->create_subscription<soil_sensor_interface::msg::SoilPobeData>(
      "/soil_sensor_data", 10,
      [this](const soil_sensor_interface::msg::SoilPobeData::SharedPtr msg)
      {
        // 假设消息里有这两个字段名，根据你真实 msg 修改
        const QString text = QString("Soil H: %1 %%   T: %2 °C")
                               .arg(msg->soil_humidity, 0, 'f', 1)
                               .arg(msg->soil_temperature, 0, 'f', 1);
        probe_num_->setText(text);
      });
}

void DemoPanel::topicCallback(const std_msgs::msg::Int8 & msg)
{
  switch (msg.data) {
    case 0:
      label_->setText("● Robot action success.");
      label_->setStyleSheet(
        "QLabel {"
        "  color: rgba(80, 221, 19, 1);"
        "  font-weight: bold;"
        "}"
      );
      break;
    case -1:
      label_->setText("● Robot planned failed.");
      label_->setStyleSheet(
        "QLabel {"
        "  color: rgb(255,0,0);"
        "  font-weight: bold;"
        "}"
      );
      break;
    case 1:
      label_->setText("● Robot is planning.");
      label_->setStyleSheet(
        "QLabel {"
        "  color: rgba(115, 210, 227, 1);"
        "  font-weight: bold;"
        "}"
      );
      break;
    case 3:
      label_->setText("● Robot is at home position.");
      label_->setStyleSheet(
        "QLabel {"
        "  color: rgb(84,248,8);"
        "  font-weight: bold;"
        "}"
      );
      break;
    case 4:
      label_->setText("● Robot executed successfully.");
      label_->setStyleSheet(
        "QLabel {"
        "  color: rgb(71,209,7);"
        "  font-weight: bold;"
        "}"
      );
      break;
    case 5:
      label_->setText("● Robot execution failed.");
      label_->setStyleSheet(
        "QLabel {"
        "  color: rgba(255,0,0,1);"
        "  font-weight: bold;"
        "}"
      );
      break;
    default:
      label_->setText(QString("● Code: %1").arg(msg.data));
      label_->setStyleSheet(
        "QLabel {"
        "  color: rgb(32,32,32);"
        "  font-weight: bold;"
        "}"
      );
      break;
  }
}

void DemoPanel::buttonPlanActivated()
{
  std_msgs::msg::Int8 message;
  message.data = 0;
  publisher_->publish(message);
}

void DemoPanel::buttonExecuteActivated()
{
  std_msgs::msg::Int8 message;
  message.data = 1;
  publisher_->publish(message);
}

void DemoPanel::buttonHomeActivated()
{
  std_msgs::msg::Int8 message;
  message.data = 2;
  publisher_->publish(message);
}

void DemoPanel::buttonNextCaptureActivated()
{
  std_msgs::msg::Int8 message;
  message.data = 3;  // next_capture
  publisher_->publish(message);
}

void DemoPanel::buttonSoilPlanActivated()
{
  std_msgs::msg::Int8 message;
  message.data = 6;  // soil_plan
  publisher_->publish(message);
}

void DemoPanel::buttonSoilProbeActivated()
{
  std_msgs::msg::Int8 message;
  message.data = 5;  // soil_probe / FIND_PROBE_POSITION
  publisher_->publish(message);
}

void DemoPanel::buttonMoveBackActivated()
{
  std_msgs::msg::Int8 message;
  message.data = 4;  // move_back
  publisher_->publish(message);
}

}  // namespace rviz_panel_tutorial

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(rviz_panel_tutorial::DemoPanel, rviz_common::Panel)
