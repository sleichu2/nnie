#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <sys/ioctl.h>
#include <dirent.h>
#include "ins_nnie_interface.h"
#include "Tensor.h"
#include "util.h"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"
#include "opencv2/dnn.hpp"

/******************************************************************************
* function : show usage
******************************************************************************/
void Usage(char *pchPrgName)
{
    printf("Usage : %s <index> \n", pchPrgName);
    printf("index:\n");
    printf("\t 1) Yolov3(Read File).\n");
    printf("\t 2) SaliencyDetect(Read File).\n");
}

void yolov3DetectDemo(const char *model_path, const char *image_path)
{
    insta360::NNIE yolov3_mnas;

    yolov3_mnas.init(model_path);

    printf("yolov3 start\n");

    struct timeval tv1;
    struct timeval tv2;
    long t1, t2, time;

    gettimeofday(&tv1, NULL);
    yolov3_mnas.run(image_path);

    gettimeofday(&tv2, NULL);
    t1 = tv2.tv_sec - tv1.tv_sec;
    t2 = tv2.tv_usec - tv1.tv_usec;
    time = (long)(t1 * 1000 + t2 / 1000);
    printf("yolov3 NNIE inference time : %dms\n", time);

    gettimeofday(&tv1, NULL);

    Tensor output0 = yolov3_mnas.getOutputTensor(0);
    Tensor output1 = yolov3_mnas.getOutputTensor(1);
    Tensor output2 = yolov3_mnas.getOutputTensor(2);

    /*yolov3的参数*/
    int img_width = 416;
    int img_height = 416;
    int num_classes = 3;
    int kBoxPerCell = 3;

    int feature_index0 = 0;
    int feature_index1 = 1;
    int feature_index2 = 2;

    float conf_threshold = 0.05;
    float nms_threshold = 0.5;
    int is_nms = 1;

    std::vector<int> ids;
    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;

    const std::vector<std::vector<cv::Size2f>> anchors = {
        {{58, 188}, {119, 254}, {238, 326}},
        {{19, 56}, {36, 98}, {87, 96}},
        {{6, 12}, {13, 24}, {39, 37}}};

    parseYolov3Feature(img_width,
                       img_height,
                       num_classes,
                       kBoxPerCell,
                       feature_index0,
                       conf_threshold,
                       anchors[0],
                       output0,
                       ids,
                       boxes,
                       confidences);

    parseYolov3Feature(img_width,
                       img_height,
                       num_classes,
                       kBoxPerCell,
                       feature_index1,
                       conf_threshold,
                       anchors[1],
                       output1,
                       ids,
                       boxes,
                       confidences);

    parseYolov3Feature(img_width,
                       img_height,
                       num_classes,
                       kBoxPerCell,
                       feature_index2,
                       conf_threshold,
                       anchors[2],
                       output2,
                       ids,
                       boxes,
                       confidences);

    std::vector<int> indices;
    char *cls_names[] = {"person", "car", "dog"};
    std::vector<ObjectDetection> detection_results;

    if (is_nms)
    {
        cv::dnn::NMSBoxes(boxes, confidences, conf_threshold, nms_threshold, indices);
    }
    else
    {
        for (int i = 0; i < boxes.size(); ++i)
        {
            indices.push_back(i);
        }
    }

    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        cv::Rect box = boxes[idx];

        // remap box in src input size.
        auto remap_box = RemapBoxOnSrc(cv::Rect2d(box), img_width, img_height);
        ObjectDetection object_detection;
        object_detection.box = remap_box;
        object_detection.cls_id = ids[idx];
        object_detection.confidence = confidences[idx];
        detection_results.push_back(std::move(object_detection));

        float xmin = object_detection.box.x;
        float ymin = object_detection.box.y;
        float xmax = object_detection.box.x + object_detection.box.width;
        float ymax = object_detection.box.y + object_detection.box.height;
        float confidence = object_detection.confidence;
        int cls_id = object_detection.cls_id;
        char *cls_name = cls_names[cls_id];
        printf("%d %s %.3f %.3f %.3f %.3f %.3f\n", cls_id, cls_name, confidence, xmin, ymin, xmax, ymax);
    }

    gettimeofday(&tv2, NULL);
    t1 = tv2.tv_sec - tv1.tv_sec;
    t2 = tv2.tv_usec - tv1.tv_usec;
    time = (long)(t1 * 1000 + t2 / 1000);
    printf("yolov3 postProcess : %dms\n", time);

    printf("yolov3 finish\n");
}

/******************************************************************************
* function 
******************************************************************************/
int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        Usage(argv[0]);
        return -1;
    }
    switch (*argv[1])
    {
    case '1':
    {
        const char *image_path = "./data/nnie_image/test/dog_bike_car.bgr";
        const char *model_path = "./data/nnie_model/detection/inst_yolo3.wk";
        yolov3DetectDemo(model_path, image_path);
    }
    break;


    default:
    {
        Usage(argv[0]);
    }
    break;
    }

    return 1;
}
