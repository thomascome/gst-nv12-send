#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define WIDTH 640
#define HEIGHT 480
#define FRAMERATE 30

void send_frame_over_udp(const std::vector<guint8> &frame, int sock, const struct sockaddr_in &addr)
{
    size_t max_packet_size = 65507; // Maximum UDP packet size
    size_t frame_size = frame.size();
    size_t offset = 0;

    while (offset < frame_size)
    {
        size_t end = std::min(offset + max_packet_size, frame_size);
        sendto(sock, frame.data() + offset, end - offset, 0, (struct sockaddr *)&addr, sizeof(addr));
        offset = end;
    }
}

GstFlowReturn new_sample(GstAppSink *appsink, gpointer user_data)
{
    GstSample *sample = gst_app_sink_pull_sample(appsink);
    GstBuffer *buffer = gst_sample_get_buffer(sample);

    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    std::vector<guint8> frame(map.data, map.data + map.size);

    auto *udp_info = static_cast<std::pair<int, struct sockaddr_in> *>(user_data);
    send_frame_over_udp(frame, udp_info->first, udp_info->second);

    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);

    return GST_FLOW_OK;
}

int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);

    GstElement *pipeline = gst_parse_launch(
        "videotestsrc ! video/x-raw,format=NV12,width=640,height=480,framerate=30/1 ! appsink name=sink", nullptr);

    GstElement *appsink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
    g_object_set(appsink, "emit-signals", TRUE, "sync", FALSE, nullptr);

    struct sockaddr_in addr
    {
    };
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    std::pair<int, struct sockaddr_in> udp_info = {sock, addr};

    g_signal_connect(appsink, "new-sample", G_CALLBACK(new_sample), &udp_info);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    std::cout << "Streaming NV12 frames over UDP..." << std::endl;
    GMainLoop *main_loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(main_loop);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    close(sock);

    return 0;
}
