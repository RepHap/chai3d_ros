//
// Created by stefan spiss on 06.07.17.
//

#include <ros/ros.h>
#include <boost/thread/thread.hpp>
#include <memory>

#include <falcon_ros/FalconRosDriver.h>


int main(int argc, char** argv) {
    ros::init(argc, argv, "falcon_ros_node");
    ros::NodeHandle node;

    FalconRosDriver driver(node, 1000.0, "/falcon/position", "/falcon/velocity", "/falcon/buttons", "/falcon/force", true);

    driver.startFalconRosNode();

    ros::shutdown();
}
