#include "scanfromabove.h"

#include <iostream>
#include <string>

using namespace std;

int Exec::ScanFromAbove::expand (int free_id) {

  std::string ns = ros::names::clean (ros::this_node::getNamespace());

  ROS_INFO("expand: %s", ns.c_str());

  // Place code doing the work here

  return free_id;
}


bool Exec::ScanFromAbove::prepare () {
  bool res = true;
  ROS_INFO ("Exec::ScanFromAbove::prepare");

  if (res) {
    set_active_flag (node_ns, node_id, true);
  }

  return res;
}

void Exec::ScanFromAbove::start () {
  ROS_INFO ("Exec::ScanFromAbove::start: %s - %d", node_ns.c_str(), node_id);

  ros::NodeHandle n;

  if (!do_before_work()) {
    return;
  }

  //
  // If the executor expands the node then the code below should be similar to the  
  // code for the sequence node.
  //
  // Otherwise the code below is the code doing the actual scan/mapping
  //


  //
  // Replace the sleep with useful work.
  //

  sleep(20);

  //
  // When we reach this point the node execution whould be finished.
  //
  
  ROS_INFO ("Exec::ScanFromAbove: FINISHED");

  wait_for_postwork_conditions ();
}

bool Exec::ScanFromAbove::abort () {
  bool res = false;
  ROS_INFO("Exec::ScanFromAbove::abort");

  return res;
}

bool Exec::ScanFromAbove::get_constraints (std::vector<std::string> & cons) {
  cons.clear ();
  bool res = false;

  return res;
}

