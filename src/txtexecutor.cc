#include "ros/ros.h"

#include <boost/thread.hpp>

#include "std_msgs/String.h"

#include <iostream>
#include <sstream>
#include <queue>

#include "lrs_msgs_tst/ConfirmReq.h"

#include "lrs_srvs_tst/TSTCreateExecutor.h"
#include "lrs_srvs_tst/TSTStartExecutor.h"
#include "lrs_srvs_tst/TSTExecutorExpand.h"
#include "lrs_srvs_tst/TSTExecutorGetConstraints.h"
#include "lrs_srvs_tst/TSTGetNode.h"

#include "concurrent.h"
#include "sequence.h"
#include "testif.h"
#include "confirm.h"
#include "wait.h"
#include "executors/flyto.h"
#include "executors/flywaypoints.h"

//#include "lrs_msgs/ConfirmReq.h"

#include "executil.h"

std::map<std::string, Executor *> execmap;
std::map<std::string, boost::thread *> threadmap;
ros::NodeHandle * global_nh;
ros::Publisher * global_confirm_pub;

boost::mutex mutex;

using namespace std;

void spin_thread () {
  ros::spin();
}

bool create_executor (lrs_srvs_tst::TSTCreateExecutor::Request  &req,
		      lrs_srvs_tst::TSTCreateExecutor::Response &res ) {
  boost::mutex::scoped_lock mutex;
  ROS_INFO("quadexecutor: create_executor: %s %d - %d", req.ns.c_str(), req.id, req.run_prepare);

  ostringstream os;
  os << req.ns << "-" << req.id;
  if (execmap.find (os.str()) != execmap.end()) {
    ROS_INFO ("Executor already exists, overwrites it: %s %d", req.ns.c_str(), req.id);
  }

  string type = get_node_type (req.ns, req.id);
  
  bool found = false;
  if (type == "conc") {
    execmap[os.str()] = new Exec::Concurrent (req.ns, req.id);
    found = true;
  }

  if (type == "seq") {
    execmap[os.str()] = new Exec::Sequence (req.ns, req.id);
    found = true;
  }

  if (type == "test-if") {
    execmap[os.str()] = new Exec::TestIf (req.ns, req.id);
    found = true;
  }

  if (type == "confirm") {
    execmap[os.str()] = new Exec::Confirm (req.ns, req.id);
    found = true;
  }

  if (type == "wait") {
    execmap[os.str()] = new Exec::Wait (req.ns, req.id);
    found = true;
  }

  if (type == "fly-to") {
    execmap[os.str()] = new Exec::FlyTo (req.ns, req.id);
    found = true;
  }

  if (type == "fly-waypoints") {
    execmap[os.str()] = new Exec::FlyWaypoints (req.ns, req.id);
    found = true;
  }

  if (found) {
    res.success = true;
    res.error = 0;
    if (req.run_prepare) {
      bool prep = execmap[os.str()]->prepare ();
      if (!prep) {
	res.success = false;
	res.error = 2;
      }
    }
  } else {
    ROS_ERROR ("Could not create executor of type: %s", type.c_str());
    res.success = false;
    res.error = 1;
  }

  return true;
}


bool start_executor (lrs_srvs_tst::TSTStartExecutor::Request  &req,
		     lrs_srvs_tst::TSTStartExecutor::Response &res ) {
  boost::mutex::scoped_lock mutex;
  ROS_INFO("start_executor: %s %d", req.ns.c_str(), req.id);

  ostringstream os;
  os << req.ns << "-" << req.id;


  if (execmap.find (os.str()) != execmap.end()) {
    string type = get_node_type (req.ns, req.id);
    boost::thread * thread;
    //      thread = new boost::thread(&Exec::Concurrent::start, execmap[os.str()]);
    thread = new boost::thread(&Executor::start, execmap[os.str()]);
    // thread->interrupt () to abort...
    threadmap[os.str()] = thread;
    res.success = true;
    res.error = 0;
  } else {
    ROS_ERROR ("Executor does not exist: %s %d", req.ns.c_str(), req.id);
    res.success = false;
    res.error = 1;
  }

  return true;
}

bool executor_expand (lrs_srvs_tst::TSTExecutorExpand::Request  &req,
		      lrs_srvs_tst::TSTExecutorExpand::Response &res ) {
  boost::mutex::scoped_lock mutex;
  ROS_INFO("executor_expand: %s %d", req.ns.c_str(), req.id);

  ostringstream os;
  os << req.ns << "-" << req.id;

  if (execmap.find (os.str()) != execmap.end()) {
    if (execmap[os.str()]->expand ()) {
      res.error = 0;
      res.success = true;
    } else {
      ROS_ERROR ("executor_expand: Expand failed");
      res.success = false;
      res.error = 2;
    }
  } else {
    ROS_ERROR ("Executor does not exist: %s %d", req.ns.c_str(), req.id);
    res.success = false;
    res.error = 1;
  }

  return true;
}

bool executor_get_constraints (lrs_srvs_tst::TSTExecutorGetConstraints::Request  &req,
			       lrs_srvs_tst::TSTExecutorGetConstraints::Response &res ) {
  boost::mutex::scoped_lock mutex;
  ROS_INFO("executor_get_constraints: %s %d", req.ns.c_str(), req.id);

  ostringstream os;
  os << req.ns << "-" << req.id;

  if (execmap.find (os.str()) != execmap.end()) {
    std::vector<std::string> cons;
    if (execmap[os.str()]->get_constraints (cons)) {
      res.constraints = cons;
      res.error = 0;
      res.success = true;
    } else {
      ROS_ERROR ("executor_get_constraints: Failed to get constraints: %s", os.str().c_str());
      res.success = false;
      res.error = 2;
    }
  } else {
    ROS_ERROR ("Executor does not exist: %s %d", req.ns.c_str(), req.id);
    res.success = false;
    res.error = 1;
  }

  return true;
}

int main(int argc, char **argv) {

  //  ros::init(argc, argv, "quadexecutor", ros::init_options::AnonymousName);
  ros::init(argc, argv, "quadexecutor");

  ros::NodeHandle n;
  global_nh = &n;

  ros::Publisher confirm_pub = n.advertise<lrs_msgs_tst::ConfirmReq>("confirm_request", 1, true); // queue size 1 and latched
  global_confirm_pub = &confirm_pub;

  std::vector<ros::ServiceServer> services;

  std::string prefix = "tst_executor/";

  services.push_back (n.advertiseService(prefix + "create_executor", create_executor));
  services.push_back (n.advertiseService(prefix + "executor_expand", executor_expand));
  services.push_back (n.advertiseService(prefix + "executor_get_constraints", executor_get_constraints));
  services.push_back (n.advertiseService(prefix + "start_executor", start_executor));

  ROS_INFO("Ready to be an executor factory");

  ros::Rate loop_rate(500);
  ///  int count = 0;

  //  boost::thread sthread (spin_thread);

  ros::spin();
  
  return 0;
}
