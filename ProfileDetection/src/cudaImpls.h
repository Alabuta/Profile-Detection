#pragma once

#include "main.h"

#if __USE_GPGPU__
#include <opencv2/core/cuda.hpp>
#include <opencv2/core/cuda_stream_accessor.hpp>
#include <opencv2/core/cuda_types.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudalegacy.hpp>
#include <opencv2/cudaimgproc.hpp>

#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

void inRange_gpu(cv::cuda::GpuMat &src, cv::Scalar &lowerb, cv::Scalar &upperb, cv::cuda::GpuMat &dst);
#endif