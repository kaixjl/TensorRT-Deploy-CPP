
#include "preprocess_ops.h"
#include "preprocess_util.hpp"
#include "preprocessor.h"

namespace gf
{

void NormalizeImage::Run(cv::cuda::GpuMat *data, const int &num)
{
	NormalizeImageOnGpu(data, *m_stream, num,
						PreprocessorFactory::MUL_MATRIX, PreprocessorFactory::SUBTRACT_MATRIX);
}

void Permute::Run(cv::cuda::GpuMat *data, const int &num)
{
	///@note this method essentially extract each channel of a image and put it to an array.
	///@note such that: array <=> [C,H,W]
	///@note this ops has been done in CvtFromGpuMat, thus this will do nothing for gpu version.
}

void Resize::Run(cv::cuda::GpuMat *data, const int &num)
{
	auto scale = GenerateScale(*data);
	ResizeOnGpu(data, data, *m_stream, num,
				scale.second, scale.first, (int)Config::INTERP);

}

std::pair<float, float> Resize::GenerateScale(const cv::cuda::GpuMat &im)
{
	std::pair<float, float> resize_scale;
	int                     origin_w = im.cols;
	int                     origin_h = im.rows;

	if (Config::KEEP_RATIO) {
		int   im_size_max     = std::max(origin_w, origin_h);
		int   im_size_min     = std::min(origin_w, origin_h);
		int   target_size_max =
				  *std::max_element(Config::TARGET_SIZE.begin(), Config::TARGET_SIZE.end());
		int   target_size_min =
				  *std::min_element(Config::TARGET_SIZE.begin(), Config::TARGET_SIZE.end());
		float scale_min       =
				  static_cast<float>(target_size_min) / static_cast<float>(im_size_min);
		float scale_max       =
				  static_cast<float>(target_size_max) / static_cast<float>(im_size_max);
		float scale_ratio     = std::min(scale_min, scale_max);
		resize_scale = {scale_ratio, scale_ratio};
	}
	else {
		//always [H,W] order.
		resize_scale.first =
			static_cast<float>(Config::TARGET_SIZE[0]) / static_cast<float>(origin_h);

		resize_scale.second =
			static_cast<float>(Config::TARGET_SIZE[1]) / static_cast<float>(origin_w);

	}

	return resize_scale;
}

void LetterBoxResize::Run(cv::cuda::GpuMat *data, const int &num)
{
	float resize_scale = GenerateScale(*data);
	auto  new_shape_w  = (int)std::round((float)data->cols * resize_scale);
	auto  new_shape_h  = (int)std::round((float)data->rows * resize_scale);

	auto pad_w = (float)(Config::TARGET_SIZE[1] - new_shape_w) / 2.0f;
	auto pad_h = (float)(Config::TARGET_SIZE[0] - new_shape_h) / 2.0f;

	int top    = (int)std::round(pad_h - 0.1);
	int bottom = (int)std::round(pad_h + 0.1);
	int left   = (int)std::round(pad_w - 0.1);
	int right  = (int)std::round(pad_w + 0.1);

	ResizeOnGpu(data, data, *m_stream, num,
				(float)new_shape_h / (float)data->rows, (float)new_shape_w / (float)data->cols,
				cv::INTER_AREA);

	PadOnGpu(data, data, *m_stream,
			 top, bottom, left, right,
			 cv::BORDER_CONSTANT, cv::Scalar(127.5), num);
}

float LetterBoxResize::GenerateScale(const cv::cuda::GpuMat &im)
{
	int origin_w = im.cols;
	int origin_h = im.rows;

	int target_h = Config::TARGET_SIZE[0];
	int target_w = Config::TARGET_SIZE[1];

	float ratio_h      = static_cast<float>(target_h) / static_cast<float>(origin_h);
	float ratio_w      = static_cast<float>(target_w) / static_cast<float>(origin_w);
	float resize_scale = std::min(ratio_h, ratio_w);
	return resize_scale;
}

void PadStride::Run(cv::cuda::GpuMat *data, const int &num)
{
	const int s = (int)Config::STRIDE;
	if (s <= 0)return;

	int rh = data->rows;
	int rw = data->cols;
	int nh = (rh / s) * s + (rh % s != 0) * s;
	int nw = (rw / s) * s + (rw % s != 0) * s;
	PadOnGpu(data, data, *m_stream, 0, nh - rh, 0, nw - rw,
			 cv::BORDER_CONSTANT, cv::Scalar(0), num);
}

void TopDownEvalAffine::Run(cv::cuda::GpuMat *data, const int &num)
{
	ResizeOnGpu(data, data, *m_stream, num, (float)Config::TRAIN_SIZE[0] / (float)data->rows,
				(float)Config::TRAIN_SIZE[1] / (float)data->cols, (int)Config::INTERP);
}
}