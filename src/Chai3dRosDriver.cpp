//
// Created by stefan spiss on 06.07.17.
//

#include <chai3d_ros/Chai3dRosDriver.h>


Chai3dRosDriver::Chai3dRosDriver(ros::NodeHandle node, float loopRate, std::string positionTopic,
                                 std::string velocityTopic, std::string buttonsTopic, std::string forceSubTopic,
                                 bool forceOutput) :
        node(node), loopRate(loopRate), positionTopic(positionTopic), velocityTopic(velocityTopic),
        buttonsTopic(buttonsTopic), forceSubTopic(forceSubTopic), forceOutput(forceOutput),
        position(0.0f, 0.0f, 0.0f), velocity(0.0f, 0.0f, 0.0f), force(0.0f, 0.0f, 0.0f), buttons(4, 0),
        hapticLoop(true), forceConsumed(true) {
    int initState = this->initFalcon();
    if(initState == 0) {
        std::cout << "Error, no device found!" << std::endl;
        std::exit(-1);
    } else if(initState == -1) {
        std::cout << "Error, could not open connection to device 0!" << std::endl;
        std::exit(-2);
    }

    this->position_pub = this->node.advertise<geometry_msgs::Vector3Stamped>(this->positionTopic.c_str(), 1);
    this->velocity_pub = this->node.advertise<geometry_msgs::Vector3Stamped>(this->velocityTopic.c_str(), 1);
    this->buttons_pub = this->node.advertise<std_msgs::Int8MultiArray>(this->buttonsTopic.c_str(), 1);

    this->force_sub = this->node.subscribe<geometry_msgs::Vector3>(this->forceSubTopic, 1,
                                                                   &Chai3dRosDriver::forceCallback, this);
}

Chai3dRosDriver::~Chai3dRosDriver() {
    //cleanUpFalcon();
}

int Chai3dRosDriver::initFalcon() {
    // create device handler
    handler = new chai3d::cHapticDeviceHandler();

    // get a handle to the first haptic device
    if (!handler->getDevice(hapticDevice, 0)) {
        return 0;
    }

    // open a connection to haptic device
    if (!hapticDevice->open()) {
        return -1;
    }

    // calibrate device (if necessary)
    hapticDevice->calibrate();

    // retrieve information about the current haptic device
    chai3d::cHapticDeviceInfo info = hapticDevice->getSpecifications();
    std::cout << "Connected to: " << info.m_manufacturerName << " " << info.m_modelName << std::endl;
    return 1;
}

void Chai3dRosDriver::saturatAndSetForces(double x, double y, double z) {
    if (x < -MAX_FORCE) x = -MAX_FORCE;
    if (y < -MAX_FORCE) y = -MAX_FORCE;
    if (z < -MAX_FORCE) z = -MAX_FORCE;
    if (x > MAX_FORCE) x = MAX_FORCE;
    if (y > MAX_FORCE) y = MAX_FORCE;
    if (z > MAX_FORCE) z = MAX_FORCE;
    this->force.set(z, x, y);
}

void Chai3dRosDriver::forceCallback(const geometry_msgs::Vector3::ConstPtr &data) {
    this->saturatAndSetForces(data->x, data->y, data->z);
    forceConsumed = false;
}

void Chai3dRosDriver::publishFalconData() {

    ros::AsyncSpinner spinner(2);
    spinner.start();

    while (ros::ok()) {
        geometry_msgs::Vector3Stamped pos;
        geometry_msgs::Vector3Stamped velocity;
        std_msgs::Int8MultiArray but;
        geometry_msgs::Vector3Stamped angles;

        pos.header.frame_id = velocity.header.frame_id = angles.header.frame_id = ros::this_node::getName();
        pos.header.stamp = velocity.header.stamp = angles.header.stamp = ros::Time::now();
        pos.vector.x = this->position.y();
        pos.vector.y = this->position.z();
        pos.vector.z = this->position.x();
        velocity.vector.x = this->velocity.y();
        velocity.vector.y = this->velocity.z();
        velocity.vector.z = this->velocity.x();

        but.data.push_back(buttons[0]);
        but.data.push_back(buttons[1]);
        but.data.push_back(buttons[2]);
        but.data.push_back(buttons[3]);

        this->position_pub.publish(pos);
        this->velocity_pub.publish(velocity);
        this->buttons_pub.publish(but);

        if(forceConsumed) {
            force.set(0.0f, 0.0f, 0.0f);
        }

        this->loopRate.sleep();
    }
    spinner.stop();
    hapticLoop = false;
}

void Chai3dRosDriver::startChai3dRosNode() {
    boost::thread hapticsThread(boost::bind(&Chai3dRosDriver::falconCallback, this));

    publishFalconData();

    hapticsThread.join();

    cleanUpFalcon();
}

void Chai3dRosDriver::cleanUpFalcon() {
    hapticDevice->close();
    delete(handler);
}

void Chai3dRosDriver::falconCallback() {

    // main haptic simulation loop
    while (hapticLoop) {

        chai3d::cVector3d currentPos;
        chai3d::cVector3d currentVel;
        
        // read position
        hapticDevice->getPosition(currentPos);

        // read linear velocity
        hapticDevice->getLinearVelocity(currentVel);

        // read user-switch status (button 0)
        bool button0, button1, button2, button3;
        button0 = false;
        button1 = false;
        button2 = false;
        button3 = false;

        hapticDevice->getUserSwitch(0, button0);
        hapticDevice->getUserSwitch(1, button1);
        hapticDevice->getUserSwitch(2, button2);
        hapticDevice->getUserSwitch(3, button3);

        position = currentPos;
        velocity = currentVel;

        this->buttons[0] = button0;
        this->buttons[1] = button1;
        this->buttons[2] = button2;
        this->buttons[3] = button3;

        /**
         * Apply forces
         */
        if (forceOutput) {
            //std::cout << "Force: " << rosDriver->getForce() << std::endl;
            hapticDevice->setForce(force);
            forceConsumed = true;
        } else {
            hapticDevice->setForce(chai3d::cVector3d(0.0f, 0.0f, 0.0f));
        }
    }
}
