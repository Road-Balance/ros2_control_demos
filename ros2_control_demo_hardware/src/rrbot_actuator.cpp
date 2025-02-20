// Copyright (c) 2021, Stogl Robotics Consulting UG (haftungsbeschränkt)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//
// Authors: Subhas Das, Denis Stogl
//

#include "ros2_control_demo_hardware/rrbot_actuator.hpp"

#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

#include "hardware_interface/actuator_interface.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/rclcpp.hpp"

namespace ros2_control_demo_hardware
{
CallbackReturn RRBotModularJoint::on_init(const hardware_interface::HardwareInfo & info)
{
  if (hardware_interface::ActuatorInterface::on_init(info) != CallbackReturn::SUCCESS)
  {
    return CallbackReturn::ERROR;
  }

  hw_start_sec_ = stod(info_.hardware_parameters["example_param_hw_start_duration_sec"]);
  hw_stop_sec_ = stod(info_.hardware_parameters["example_param_hw_stop_duration_sec"]);
  hw_slowdown_ = stod(info_.hardware_parameters["example_param_hw_slowdown"]);

  hw_joint_state_ = std::numeric_limits<double>::quiet_NaN();
  hw_joint_command_ = std::numeric_limits<double>::quiet_NaN();

  const hardware_interface::ComponentInfo & joint = info_.joints[0];
  // RRBotModularJoint has exactly one state and command interface on each joint
  if (joint.command_interfaces.size() != 1)
  {
    RCLCPP_FATAL(
      rclcpp::get_logger("RRBotModularJoint"),
      "Joint '%s' has %zu command interfaces found. 1 expected.", joint.name.c_str(),
      joint.command_interfaces.size());
    return CallbackReturn::ERROR;
  }

  if (joint.command_interfaces[0].name != hardware_interface::HW_IF_POSITION)
  {
    RCLCPP_FATAL(
      rclcpp::get_logger("RRBotModularJoint"),
      "Joint '%s' have %s command interfaces found. '%s' expected.", joint.name.c_str(),
      joint.command_interfaces[0].name.c_str(), hardware_interface::HW_IF_POSITION);
    return CallbackReturn::ERROR;
  }

  if (joint.state_interfaces.size() != 1)
  {
    RCLCPP_FATAL(
      rclcpp::get_logger("RRBotModularJoint"), "Joint '%s' has %zu state interface. 1 expected.",
      joint.name.c_str(), joint.state_interfaces.size());
    return CallbackReturn::ERROR;
  }

  if (joint.state_interfaces[0].name != hardware_interface::HW_IF_POSITION)
  {
    RCLCPP_FATAL(
      rclcpp::get_logger("RRBotModularJoint"), "Joint '%s' have %s state interface. '%s' expected.",
      joint.name.c_str(), joint.state_interfaces[0].name.c_str(),
      hardware_interface::HW_IF_POSITION);
    return CallbackReturn::ERROR;
  }

  return CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface> RRBotModularJoint::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;

  state_interfaces.emplace_back(hardware_interface::StateInterface(
    info_.joints[0].name, hardware_interface::HW_IF_POSITION, &hw_joint_state_));

  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> RRBotModularJoint::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;

  command_interfaces.emplace_back(hardware_interface::CommandInterface(
    info_.joints[0].name, hardware_interface::HW_IF_POSITION, &hw_joint_command_));

  return command_interfaces;
}

CallbackReturn RRBotModularJoint::on_activate(const rclcpp_lifecycle::State & /*previous_state*/)
{
  RCLCPP_INFO(rclcpp::get_logger("RRBotModularJoint"), "Starting ...please wait...");

  for (int i = 0; i < hw_start_sec_; i++)
  {
    rclcpp::sleep_for(std::chrono::seconds(1));
    RCLCPP_INFO(rclcpp::get_logger("RRBotModularJoint"), "%.1f seconds left...", hw_start_sec_ - i);
  }

  // set some default values for joints
  if (std::isnan(hw_joint_state_))
  {
    hw_joint_state_ = 0;
    hw_joint_command_ = 0;
  }

  RCLCPP_INFO(rclcpp::get_logger("RRBotModularJoint"), "System Successfully started!");

  return CallbackReturn::SUCCESS;
}

CallbackReturn RRBotModularJoint::on_deactivate(const rclcpp_lifecycle::State & /*previous_state*/)
{
  RCLCPP_INFO(rclcpp::get_logger("RRBotModularJoint"), "Stopping ...please wait...");

  for (int i = 0; i < hw_stop_sec_; i++)
  {
    rclcpp::sleep_for(std::chrono::seconds(1));
    RCLCPP_INFO(rclcpp::get_logger("RRBotModularJoint"), "%.1f seconds left...", hw_stop_sec_ - i);
  }

  RCLCPP_INFO(rclcpp::get_logger("RRBotModularJoint"), "System successfully stopped!");

  return CallbackReturn::SUCCESS;
}

hardware_interface::return_type RRBotModularJoint::read()
{
  RCLCPP_INFO(rclcpp::get_logger("RRBotModularJoint"), "Reading...");

  // Simulate RRBot's movement
  hw_joint_state_ = hw_joint_state_ + (hw_joint_command_ - hw_joint_state_) / hw_slowdown_;
  RCLCPP_INFO(
    rclcpp::get_logger("RRBotModularJoint"), "Got state %.5f for joint '%s'!", hw_joint_state_,
    info_.joints[0].name.c_str());

  RCLCPP_INFO(rclcpp::get_logger("RRBotModularJoint"), "Joints successfully read!");

  return hardware_interface::return_type::OK;
}

hardware_interface::return_type ros2_control_demo_hardware::RRBotModularJoint::write()
{
  RCLCPP_INFO(rclcpp::get_logger("RRBotModularJoint"), "Writing...please wait...");

  // Simulate sending commands to the hardware
  RCLCPP_INFO(
    rclcpp::get_logger("RRBotModularJoint"), "Got command %.5f for joint '%s'!", hw_joint_command_,
    info_.joints[0].name.c_str());

  RCLCPP_INFO(rclcpp::get_logger("RRBotModularJoint"), "Joints successfully written!");

  return hardware_interface::return_type::OK;
}

}  // namespace ros2_control_demo_hardware

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(
  ros2_control_demo_hardware::RRBotModularJoint, hardware_interface::ActuatorInterface)
