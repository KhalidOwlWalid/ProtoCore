/*
This example is not only for use of setting the interval, but also for sending MAV_CMD to the autopilot.

For each different commands, there will be parameters that you will need to pass it via the msg_long message struct.

Refer here for more information: https://mavlink.io/en/messages/common.html#mav_commands

To run this script, run the following command:
./mavlink_set_interval

In the mavproxy terminal, you should see the following:
Got COMMAND_ACK: SET_MESSAGE_INTERVAL: ACCEPTED

This indicates that the autopilot has acknowledged the request and have changes the frequency.

Please note that if you have a GCS Application running at the same time (e.g. Mission Planner/QGroundControl), it might actively override
your set message interval request. However, once the script is ran, you should see a short burst of frequency updates on the mavlink inspector 
for a short while.
*/

#include "mavlink_common.hpp"

int socket_fd;
struct sockaddr_in autopilot_address = {};
socklen_t autopilot_address_len = sizeof(autopilot_address);
auto last_time = get_current_time_ms();

// This port value is known from receiving heartbeat from the autopilot
// This example only implements someting simple for use of reference
// However, in the case of a more advanced example, you would first need to figure out
// what port the autopilot is on, and then you will need to send the commands via that port
#define AUTOPILOT_MAVLINK_PORT 58768

int main() {

    spdlog::info("Mavlink Parameter Request example");

    setup_sock_fd(socket_fd, AF_INET, SOCK_DGRAM);
    setup_address(autopilot_address, AF_INET, ARDUPILOT_SITL_ADDRESS, AUTOPILOT_MAVLINK_PORT);

    const uint8_t system_id = 255;
    mavlink_message_t mav_msg;
    mavlink_command_long_t msg_long;
    msg_long.command = MAV_CMD_SET_MESSAGE_INTERVAL;
    msg_long.target_system = 1;
    msg_long.target_component = 1;
    msg_long.confirmation = 0;
    msg_long.param1 = MAVLINK_MSG_ID_BATTERY_STATUS;
    msg_long.param2 = 10000;
    msg_long.param3 = 0;
    msg_long.param4 = 0;
    msg_long.param5 = 0;
    msg_long.param6 = 0;
    msg_long.param7 = 0;
    mavlink_msg_command_long_encode(system_id, 190, &mav_msg, &msg_long);

    int success = send_mavlink_packet(socket_fd, mav_msg, (const struct sockaddr *)&autopilot_address);

    if (success) {
        spdlog::info("Message interval request sent");
    }

    last_time = get_current_time_ms();

    close(socket_fd);
    return 0;
}
