#include <opencv2/opencv.hpp>
//#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;


struct DownSampled_img {
	unsigned char* img_buff;
	unsigned char* img_luma;
	int width;
	int height;
};


class DownSampler {

private:
	DownSampled_img downsampled_img;
	Mat downsampledMat_buff;
	Mat downsampledMat_buffLuma;

public:
	int getDownX();
	int getDownY();
	unsigned char* downSample(unsigned char* t_src, int t_height, int t_width, bool luma, int target_res_x, int target_res_y, int pixels);

};