
#ifndef COMPLIANT_COMMAND_NODE_H
#define COMPLIANT_COMMAND_NODE_H

#include <compliance/compliant_control.h>
#include <ros/ros.h>
#include <std_srvs/SetBool.h>

namespace compliant_command_node
{

struct DefaultComplianceParameters
{
  std::vector<double> stiffness{4000, 4000, 4000, 4000, 4000, 4000};
  // Related to the cutoff frequency of the low-pass filter.
  double filter_param = 10.;
  // Deadband for force/torque measurements
  std::vector<double> deadband{10, 10, 10, 10, 10, 10};
  // Stop when force exceeds X N or torque exceeds X Nm in any dimension.
  // The robot controller's built-in safety limits are ~90 N, ? Nm
  std::vector<double> end_condition_wrench{80, 80, 80, 60, 60, 60};
  // Highest allowable force/torque across all dimensions.
  double highest_allowable_force = 88, highest_allowable_torque = 50;
  // Current force/torque data
  geometry_msgs::WrenchStamped force_torque_data;
  // Outgoing velocity msg
  std::vector<double> velocity_out{0, 0, 0, 0, 0, 0};
  // Assume a bias wrench of all zeros
  geometry_msgs::WrenchStamped bias;
};

class PublishComplianceJointVelocities
{
public:
  PublishComplianceJointVelocities() :
    compliant_control_instance_(
      compliance_params_.stiffness,
      compliance_params_.deadband,
      compliance_params_.end_condition_wrench,
      compliance_params_.filter_param,
      compliance_params_.bias,
      compliance_params_.highest_allowable_force,
      compliance_params_.highest_allowable_torque
    )
  {
  	enable_compliance_service_ = n_.advertiseService(
  	  n_.getNamespace() + "toggle_compliance_publication", &PublishComplianceJointVelocities::toggleCompliance, this);
  }

  // Spin and publish compliance velocities, unless disabled by a service call
  void spin()
  {
	  while (ros::ok())
	  {
	    ros::spinOnce();
	    ros::Duration(0.01).sleep();

	    if (compliance_enabled_)
	      ;// update the compliance calculations, and publish them
	  }
  }


private:
  // A service callback. Toggles compliance publication on/off
  bool toggleCompliance(std_srvs::SetBool::Request &req,
    std_srvs::SetBool::Response &res)
  {
    compliance_enabled_ = req.data;
    res.success = true;
    return true;
  }

  ros::NodeHandle n_;

  // An object to do compliance calculations.
  // Key equation: compliance_velocity[i] = wrench[i]/stiffness[i]
  compliant_control::CompliantControl compliant_control_instance_;

  bool compliance_enabled_ = true;

  static DefaultComplianceParameters compliance_params_;

  // Publish compliance commands unless interrupted by a service call
  ros::ServiceServer enable_compliance_service_;
};

} // namespace compliant_command_node

#endif