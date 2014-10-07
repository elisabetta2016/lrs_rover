#include "movearm.h"

#include <iostream>

#include "lrs_msgs_tst/ConfirmReq.h"

extern ros::NodeHandle * global_nh;
extern ros::Publisher * global_confirm_pub;

using namespace std;


bool Exec::MoveArm::prepare () {
  bool res = true;
  ROS_INFO ("Exec::MoveArm::prepare");

  if (res) {
    set_active_flag (node_ns, node_id, true);
  }

  return res;
}


void Exec::MoveArm::start () {
  ROS_INFO ("Exec::MoveArm::start: %s - %d", node_ns.c_str(), node_id);

  if (!do_before_work ()) {
    return;
  }

  geometry_msgs::PoseStamped p;
  if (pose_params["p"].have_value) {
    p = pose_params["p"].value;
  } else {
    ROS_ERROR ("move-arm: parameter p is missing");
    set_succeeded_flag (node_ns, node_id, false);
    set_finished_flag (node_ns, node_id, true);
    return;
  }

  ROS_INFO ("Exec::MoveArm: Execution unit: %s", tni.execution_ns.c_str());

  ROS_INFO ("Exec::MoveArm: %f %f %f %s", 
	    p.pose.position.x, p.pose.position.y, p.pose.position.z, 
	    p.header.frame_id.c_str());

  sleep (5);

  ROS_INFO ("Exec::MoveArm: FINISHED");

  wait_for_postwork_conditions ();
}

bool Exec::MoveArm::abort () {
  bool res = false;
  ROS_INFO("Exec::MoveArm::abort");
  return res;
}

bool Exec::MoveArm::get_constraints (std::vector<std::string> & cons) {
  cons.clear ();
  bool res = false;
  res = true;
  return res;
}