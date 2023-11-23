#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot

    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the safe_move service and pass the requested joint angles
    if (!client.call(srv))
        ROS_ERROR("Failed to call service DriveToTarget");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    int white_pixel = 255;
    bool found_white_pixel = false;
    int white_pixel_x_pos = 0;

    // Loop through each pixel in the image and check if it's white
    for (int i = 0; i < img.height * img.step; i += 3) {
        int is_white = img.data[i] == white_pixel && img.data[i + 1] == white_pixel && img.data[i + 2] == white_pixel;
        if (is_white) {
            found_white_pixel = true;
            white_pixel_x_pos = (i % img.step) / 3; // Get the X coordinate of the white pixel
            break;
        }
    }

    if (found_white_pixel) {
        // Determine the section of the image where the white pixel is found
        int section = img.width / 3;
        int position = white_pixel_x_pos / section;

        if (position == 0) {
            drive_robot(0.5, 0.5); // White pixel is on the left side
        } else if (position == 1) {
            drive_robot(0.5, 0.0); // White pixel is in the middle
        } else if (position == 2) {
            drive_robot(0.5, -0.5); // White pixel is on the right side
        }
    } else {
        drive_robot(0.0, 0.0); // No white pixel found
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
