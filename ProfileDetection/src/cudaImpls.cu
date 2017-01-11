#include <algorithm>
#include "cudaImpls.h"

#if __USE_GPGPU__

#if _DEBUG
#   pragma comment(lib, "opencv_core320d.lib")
#   pragma comment(lib, "opencv_cudafilters320d.lib")
#   pragma comment(lib, "opencv_cudaimgproc320d.lib")
#   pragma comment(lib, "opencv_highgui320d.lib")
#   pragma comment(lib, "opencv_imgcodecs320d.lib")
#   pragma comment(lib, "opencv_imgproc320d.lib")
#elif NDEBUG
#   pragma comment(lib, "opencv_core320.lib")
#   pragma comment(lib, "opencv_cudafilters320.lib")
#   pragma comment(lib, "opencv_cudaimgproc320.lib")
#   pragma comment(lib, "opencv_highgui320.lib")
#   pragma comment(lib, "opencv_imgcodecs320.lib")
#   pragma comment(lib, "opencv_imgproc320.lib")
#endif

#ifdef __CUDACC__
#define KERNEL_ARGS2(grid, block) <<< grid, block >>>
#define KERNEL_ARGS3(grid, block, sh_mem) <<< grid, block, sh_mem >>>
#define KERNEL_ARGS4(grid, block, sh_mem, stream) <<< grid, block, sh_mem, stream >>>
#else
#define KERNEL_ARGS2(grid, block)
#define KERNEL_ARGS3(grid, block, sh_mem)
#define KERNEL_ARGS4(grid, block, sh_mem, stream)
#endif

#pragma comment(lib, "cuda.lib")
#pragma comment(lib, "cudart.lib")

using namespace std;

__global__ void inRange_kernel(const cv::cuda::PtrStepSz<uchar3> src, cv::cuda::PtrStepSzb dst,
    int lbc0, int ubc0, int lbc1, int ubc1, int lbc2, int ubc2)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= src.cols || y >= src.rows) return;

    uchar3 v = src(y, x);

    if (v.x >= lbc0 && v.x <= ubc0 && v.y >= lbc1 && v.y <= ubc1 && v.z >= lbc2 && v.z <= ubc2)
        dst(y, x) = 255;
    else
        dst(y, x) = 0;
}

void inRange_gpu(cv::cuda::GpuMat &src, cv::Scalar &lowerb, cv::Scalar &upperb, cv::cuda::GpuMat &dst)
{
    const int m = 32;
    int numRows = src.rows, numCols = src.cols;
    if (numRows == 0 || numCols == 0) return;

    const dim3 gridSize(ceil((float)numCols / m), ceil((float)numRows / m), 1);
    const dim3 blockSize(m, m, 1);

    inRange_kernel KERNEL_ARGS2(gridSize, blockSize) (src, dst, lowerb[0], upperb[0], lowerb[1], upperb[1], lowerb[2], upperb[2]);
}
#endif