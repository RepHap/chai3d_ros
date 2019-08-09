# chai3d_ros
Simple ROS Melodic node to retrieve position and velocity from haptics devices supported by Chai3d and to render force to it.

To retrieve the chai3d submodule in the **external** directory, do:
```
git submodule update --init --recursive
```

Additionally, follow the chai3d documentation to find out which packages need to be installed to build the SDK and how to set the user privileges to run chai3d for the haptics devices without root priveleges.

After this steps, the ROS node should compile using catkin\_make.

### Important
Node is not using the haptic thread from chai3d, because it caused segmentation faults. Instead a boost thread is used.
