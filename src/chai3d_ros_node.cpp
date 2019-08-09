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

<<<<<<< HEAD
    FalconRosDriver driver(node, 1000.0, "/falcon/position", "/falcon/velocity", "/falcon/buttons", "/falcon/force", true);
=======
    Chai3dRosDriver driver(node, 1000.0, "/chai3d/position", "/chai3d/velocity", "/chai3d/buttons", "/chai3d/force", "/chai3d/rate", true);
>>>>>>> 7db25e7... Publish haptic rate

    driver.startFalconRosNode();

    ros::shutdown();
}
