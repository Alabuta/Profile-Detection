#include "main.h"

struct Params {
    struct {
        int minH{0}, maxH{255};
        int minS{0}, maxS{255};
        int minV{0}, maxV{255};
    } mainHSVRange;

#if __USE_EXTRA_HSV_RANGE
    struct {
        int minH{0}, maxH{0};
        int minS{0}, maxS{0};
        int minV{0}, maxV{0};
    } extraHSVRange;
#endif

    int open_close_pos{0}, erode_dilate_pos{0};
};

void ReadImage(std::string const &_path, cv::Mat &_image)
{
    if (_image.data != nullptr)
        _image.release();

    _image = cv::imread(_path, cv::IMREAD_COLOR);
}

void MorphologicalOpenCloseCPU(cv::Mat &_image, int open_close_pos)
{
    if (open_close_pos == 0)
        return;

    int an = open_close_pos > 0 ? open_close_pos : -open_close_pos;

    auto element = getStructuringElement(cv::MORPH_RECT, cv::Size(an * 2 + 1, an * 2 + 1));

    if (open_close_pos < 0)
        morphologyEx(_image, _image, cv::MORPH_OPEN, element);

    else
        morphologyEx(_image, _image, cv::MORPH_CLOSE, element);

    element.release();
}

#if __USE_GPGPU__
void MorphologicalOpenCloseGPU(cv::cuda::GpuMat &_image, int open_close_pos)
{
    if (open_close_pos == 0)
        return;

    if (_image.channels() == 3)
        cv::cuda::cvtColor(_image, _image, cv::COLOR_BGR2BGRA);

    int an = open_close_pos > 0 ? open_close_pos : -open_close_pos;

    auto element = getStructuringElement(cv::MORPH_RECT, cv::Size(an * 2 + 1, an * 2 + 1));

    cv::Ptr<cv::cuda::Filter> filter;

    if (open_close_pos < 0)
        filter = cv::cuda::createMorphologyFilter(cv::MORPH_OPEN, _image.type(), element);

    else
        filter = cv::cuda::createMorphologyFilter(cv::MORPH_CLOSE, _image.type(), element);

    filter->apply(_image, _image);
    element.release();
}
#endif

void MorphologicalErodeDilate(cv::Mat &_image, int erode_dilate_pos)
{
    cv::Mat element;

    if (erode_dilate_pos == 0)
        return;

    int an = erode_dilate_pos > 0 ? erode_dilate_pos : -erode_dilate_pos;

    element = getStructuringElement(cv::MORPH_RECT, cv::Size(an * 2 + 1, an * 2 + 1));

    if (erode_dilate_pos < 0)
        cv::erode(_image, _image, element);

    else
        cv::dilate(_image, _image, element);

    element.release();
}

void ProcessImageOnCPU(cv::Mat const &_image, cv::Mat &_result, Params const &_params, std::vector<std::vector<cv::Point>> &_contours, std::vector<cv::Vec4i> &_hierarchy)
{
    if (_image.data == nullptr)
        return;

    cv::Mat imageHSV;
    cvtColor(_image, imageHSV, cv::COLOR_BGR2HSV);

    cv::Mat1b mask;
    inRange(imageHSV,
            cv::Scalar(_params.mainHSVRange.minH, _params.mainHSVRange.minS, _params.mainHSVRange.minV),
            cv::Scalar(_params.mainHSVRange.maxH, _params.mainHSVRange.maxS, _params.mainHSVRange.maxV), mask);

#if __USE_EXTRA_HSV_RANGE
    auto extraHUsed = (_params.extraHSVRange.maxH + _params.extraHSVRange.minH != 0);
    auto extraSUsed = (_params.extraHSVRange.maxS + _params.extraHSVRange.minS != 0);
    auto extraVUsed = (_params.extraHSVRange.maxV + _params.extraHSVRange.minV != 0);

    if (extraHUsed || extraSUsed || extraVUsed) {
        int minH = _params.mainHSVRange.minH, minS = _params.mainHSVRange.minS, minV = _params.mainHSVRange.minV;
        int maxH = _params.mainHSVRange.maxH, maxS = _params.mainHSVRange.maxS, maxV = _params.mainHSVRange.maxV;

        if (extraHUsed) {
            minH = _params.extraHSVRange.minH;
            maxH = _params.extraHSVRange.maxH;
        }

        if (extraSUsed) {
            minS = _params.extraHSVRange.minS;
            maxS = _params.extraHSVRange.maxS;
        }

        if (extraVUsed) {
            minV = _params.extraHSVRange.minV;
            maxV = _params.extraHSVRange.maxV;
        }

        cv::Mat1b extraMaskHSV;
        inRange(imageHSV,
                cv::Scalar(minH, minS, minV),
                cv::Scalar(maxH, maxS, maxV), extraMaskHSV);

        mask |= extraMaskHSV;
    }
#endif

    MorphologicalOpenCloseCPU(mask, _params.open_close_pos);
    MorphologicalErodeDilate(mask, _params.erode_dilate_pos);

    //_result = std::move(mask.clone());

    cv::Mat skel(mask.size(), CV_8UC1, cv::Scalar(0));
    cv::Mat temp, eroded;

    auto element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));

    do {
        cv::erode(mask, eroded, element);
        cv::dilate(eroded, temp, element);

        cv::subtract(mask, temp, temp);

        cv::bitwise_or(skel, temp, skel);

        eroded.copyTo(mask);

    } while (cv::countNonZero(mask) > 0);

    _result = skel;

    element.release();

    findContours(_result, _contours, _hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_TC89_KCOS, cv::Point(0, 0));
}

void DrawContours(cv::Size const &_size, std::vector<std::vector<cv::Point>> &_contours, std::vector<cv::Vec4i> const &_hierarchy)
{
    std::vector<std::vector<cv::Point>> approxContours(_contours.size());

    for (size_t k = 0; k < _contours.size(); k++)
        approxPolyDP(cv::Mat(_contours[k]), approxContours[k], 3, true);

    cv::Mat drawing = cv::Mat::zeros(_size, CV_8UC1);
    for (int i = 0; i < approxContours.size(); i++) {
        drawContours(drawing, approxContours, i, cv::Scalar(255), 2, 8, _hierarchy, 0, cv::Point());
    }

    cv::imshow("Image", drawing);
}

#if __USE_GPGPU__
void ProcessImageOnGPU(cv::cuda::GpuMat const &_image, cv::cuda::GpuMat &_result, Params const &_params)
{
    if (_image.data == nullptr)
        return;

    cv::cuda::cvtColor(_image, _result, cv::COLOR_BGR2HSV);

    cv::cuda::GpuMat imageHSV;
    imageHSV.create(_result.rows, _result.cols, CV_8UC1);
    imageHSV.swap(_result);

    inRange_gpu(imageHSV, cv::Scalar(170, 128, 148), cv::Scalar(178, 255, 255), _result);

    MorphologicalOpenCloseGPU(_result, 16);
}
#endif

void InitHSVMainBounds(Window &_window, Params &_params)
{
    auto gbSettings = _window.AddControl<GroupBox>(std::make_unique<GroupBox>(L"HSV (main)", 16, 88, 128, 108));

    if (!gbSettings.expired()) {
        {
            auto const left = 16, top = 26, width = 16, height = 20;

            gbSettings.lock()->AddControl<Text>(std::make_unique<Text>(L"H", left, (height + 4) * 0 + top, width, height, Text::eALIGN::nLEFT));
            gbSettings.lock()->AddControl<Text>(std::make_unique<Text>(L"S", left, (height + 4) * 1 + top, width, height, Text::eALIGN::nLEFT));
            gbSettings.lock()->AddControl<Text>(std::make_unique<Text>(L"V", left, (height + 4) * 2 + top, width, height, Text::eALIGN::nLEFT));
        }

        auto const left = 32, top = 24, width = 40, height = 20;

        auto editMainMinH = gbSettings.lock()->AddControl<Edit>(std::make_unique<Edit>(L"0", left, (height + 4) * 0 + top, width, height));
        auto editMainMaxH = gbSettings.lock()->AddControl<Edit>(std::make_unique<Edit>(L"255", left + width + 4, (height + 4) * 0 + top, width, height));

        auto editMainMinS = gbSettings.lock()->AddControl<Edit>(std::make_unique<Edit>(L"0", left, (height + 4) * 1 + top, width, height));
        auto editMainMaxS = gbSettings.lock()->AddControl<Edit>(std::make_unique<Edit>(L"255", left + width + 4, (height + 4) * 1 + top, width, height));

        auto editMainMinV = gbSettings.lock()->AddControl<Edit>(std::make_unique<Edit>(L"0", left, (height + 4) * 2 + top, width, height));
        auto editMainMaxV = gbSettings.lock()->AddControl<Edit>(std::make_unique<Edit>(L"255", left + width + 4, (height + 4) * 2 + top, width, height));

        auto SetHSVBound = [] (std::wstring const &line, int &bound)
        {
            try {
                bound = std::max(0, std::min(std::stoi(line), 255));
            }

            catch (...) {
                bound = 0;
            }
        };

        if (!editMainMinH.expired()) {
            editMainMinH.lock()->AddOnChangeListener([&params = _params, SetHSVBound] (std::wstring const &line)
            {
                SetHSVBound(line, params.mainHSVRange.minH);
            });
        }

        if (!editMainMaxH.expired()) {
            editMainMaxH.lock()->AddOnChangeListener([&params = _params, SetHSVBound] (std::wstring const &line)
            {
                SetHSVBound(line, params.mainHSVRange.maxH);
            });
        }

        if (!editMainMinS.expired()) {
            editMainMinS.lock()->AddOnChangeListener([&params = _params, SetHSVBound] (std::wstring const &line)
            {
                SetHSVBound(line, params.mainHSVRange.minS);
            });
        }

        if (!editMainMaxS.expired()) {
            editMainMaxS.lock()->AddOnChangeListener([&params = _params, SetHSVBound] (std::wstring const &line)
            {
                SetHSVBound(line, params.mainHSVRange.maxS);
            });
        }

        if (!editMainMinV.expired()) {
            editMainMinV.lock()->AddOnChangeListener([&params = _params, SetHSVBound] (std::wstring const &line)
            {
                SetHSVBound(line, params.mainHSVRange.minV);
            });
        }

        if (!editMainMaxV.expired()) {
            editMainMaxV.lock()->AddOnChangeListener([&params = _params, SetHSVBound] (std::wstring const &line)
            {
                SetHSVBound(line, params.mainHSVRange.maxV);
            });
        }
    }
}

#if __USE_EXTRA_HSV_RANGE
void InitHSVExtraBounds(Window &_window, Params &_params)
{
    auto gbSettings = _window.AddControl<GroupBox>(std::make_unique<GroupBox>(L"HSV (extra)", 16, 210, 128, 108));

    if (!gbSettings.expired()) {
        {
            auto const left = 16, top = 26, width = 16, height = 20;

            gbSettings.lock()->AddControl<Text>(std::make_unique<Text>(L"H", left, (height + 4) * 0 + top, width, height, Text::eALIGN::nLEFT));
            gbSettings.lock()->AddControl<Text>(std::make_unique<Text>(L"S", left, (height + 4) * 1 + top, width, height, Text::eALIGN::nLEFT));
            gbSettings.lock()->AddControl<Text>(std::make_unique<Text>(L"V", left, (height + 4) * 2 + top, width, height, Text::eALIGN::nLEFT));
        }

        auto const left = 32, top = 24, width = 40, height = 20;

        auto editExtraMinH = gbSettings.lock()->AddControl<Edit>(std::make_unique<Edit>(L"0", left, (height + 4) * 0 + top, width, height));
        auto editExtraMaxH = gbSettings.lock()->AddControl<Edit>(std::make_unique<Edit>(L"0", left + width + 4, (height + 4) * 0 + top, width, height));

        auto editExtraMinS = gbSettings.lock()->AddControl<Edit>(std::make_unique<Edit>(L"0", left, (height + 4) * 1 + top, width, height));
        auto editExtraMaxS = gbSettings.lock()->AddControl<Edit>(std::make_unique<Edit>(L"0", left + width + 4, (height + 4) * 1 + top, width, height));

        auto editExtraMinV = gbSettings.lock()->AddControl<Edit>(std::make_unique<Edit>(L"0", left, (height + 4) * 2 + top, width, height));
        auto editExtraMaxV = gbSettings.lock()->AddControl<Edit>(std::make_unique<Edit>(L"0", left + width + 4, (height + 4) * 2 + top, width, height));

        auto SetHSVBound = [&] (std::wstring const &line, int &bound)
        {
            try {
                bound = std::max(0, std::min(std::stoi(line), 255));
            }

            catch (...) {
                bound = 0;
            }
        };

        if (!editExtraMinH.expired()) {
            editExtraMinH.lock()->AddOnChangeListener([&params = _params, SetHSVBound] (std::wstring const &line)
            {
                SetHSVBound(line, params.extraHSVRange.minH);
            });
        }

        if (!editExtraMaxH.expired()) {
            editExtraMaxH.lock()->AddOnChangeListener([&params = _params, SetHSVBound] (std::wstring const &line)
            {
                SetHSVBound(line, params.extraHSVRange.maxH);
            });
        }

        if (!editExtraMinS.expired()) {
            editExtraMinS.lock()->AddOnChangeListener([&params = _params, SetHSVBound] (std::wstring const &line)
            {
                SetHSVBound(line, params.extraHSVRange.minS);
            });
        }

        if (!editExtraMaxS.expired()) {
            editExtraMaxS.lock()->AddOnChangeListener([&params = _params, SetHSVBound] (std::wstring const &line)
            {
                SetHSVBound(line, params.extraHSVRange.maxS);
            });
        }

        if (!editExtraMinV.expired()) {
            editExtraMinV.lock()->AddOnChangeListener([&params = _params, SetHSVBound] (std::wstring const &line)
            {
                SetHSVBound(line, params.extraHSVRange.minV);
            });
        }

        if (!editExtraMaxV.expired()) {
            editExtraMaxV.lock()->AddOnChangeListener([&params = _params, SetHSVBound] (std::wstring const &line)
            {
                SetHSVBound(line, params.extraHSVRange.maxV);
            });
        }
    }
}
#endif

void InitMorhologicalTransformationsParamsControls(Window &_window, Params &_params)
{
    auto gbSettings = _window.AddControl<GroupBox>(std::make_unique<GroupBox>(L"Morhological Transformations", 152, 88, 292, 108));

    if (!gbSettings.expired()) {
        {
            auto const left = 16, top = 34, width = 40, height = 20;

            gbSettings.lock()->AddControl<Text>(std::make_unique<Text>(L"Open", left, top, width, height, Text::eALIGN::nLEFT));
            gbSettings.lock()->AddControl<Text>(std::make_unique<Text>(L"Close", gbSettings.lock()->width() - width - left, top, width, height, Text::eALIGN::nRIGHT));

            gbSettings.lock()->AddControl<Text>(std::make_unique<Text>(L"Erode", left, top + height + 16, width, height, Text::eALIGN::nLEFT));
            gbSettings.lock()->AddControl<Text>(std::make_unique<Text>(L"Dilate", gbSettings.lock()->width() - width - left, top + height + 16, width, height, Text::eALIGN::nRIGHT));
        }

        auto const left = 64, top = 32, width = 164, height = 32;

        auto tbOpenClose = gbSettings.lock()->AddControl<Trackbar>(std::make_unique<Trackbar>(L"Open-Close", left, top, width, height, -10, 10));
        auto tbErodeDilate = gbSettings.lock()->AddControl<Trackbar>(std::make_unique<Trackbar>(L"Erode-Dilate", left, top + height + 4, width, height, -10, 10));

        if (!tbOpenClose.expired()) {
            tbOpenClose.lock()->AddOnChangeListener([&params = _params] (int value)
            {
                params.open_close_pos = value;
            });
        }

        if (!tbErodeDilate.expired()) {
            tbErodeDilate.lock()->AddOnChangeListener([&params = _params] (int value)
            {
                params.erode_dilate_pos = value;
            });
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    Params params;
    cv::Mat image, result;

    cv::setUseOptimized(true);

#if __USE_GPGPU__
    cv::cuda::GpuMat gpuImage, gpuResult;
    cv::cuda::setDevice(0);
#endif

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    Window window(L"Profile Detection", hInstance, 476, 400);

    auto btnOpen = window.AddControl<Button>(std::make_unique<Button>(L"Open", 16, 16, window.width() - 32, 52));

    if (!btnOpen.expired()) {
        btnOpen.lock()->AddOnClickListener([btnOpen, &image]
        {
            std::string path;

            auto index = GetOpenPath(btnOpen.lock()->handle(), path, {
                Filter{L"Windows bitmaps", L"*.bmp; *.dib"},
                Filter{L"JPEG", L"*.jpeg; *.jpg; *.jpe"},
                Filter{L"JPEG 2000", L"*.jp2"},
                Filter{L"Portable Network Graphics", L"*.png"}
            });

            if (index < 1)
                return;

            ReadImage(path, image);
#if __USE_GPGPU__
            gpuImage.upload(image);
#endif

            auto const width = 720, height = static_cast<int>(720 * image.rows / image.cols);

            cv::namedWindow("Image", cv::WINDOW_KEEPRATIO);
            cv::resizeWindow("Image", width, height);
            cv::imshow("Image", image);
        });
    }

    auto textElapsedTime = window.AddControl<Text>(std::make_unique<Text>(L"...", 0, window.height() - 32, window.width(), 16, Text::eALIGN::nCENTER));

    InitHSVMainBounds(window, params);
#if __USE_EXTRA_HSV_RANGE
    InitHSVExtraBounds(window, params);
#endif

    InitMorhologicalTransformationsParamsControls(window, params);

    auto btnDetect = window.AddControl<Button>(std::make_unique<Button>(L"Detect", 152, 210, 292, 48));

    if (!btnDetect.expired()) {
        btnDetect.lock()->AddOnClickListener([&]
        {

#if __USE_GPGPU__
            if (gpuResult.data != nullptr)
                gpuResult.release();
#endif

            if (result.data != nullptr)
                result.release();

#if __USE_GPGPU__
            auto elapsed = measure<>::execution(ProcessImageOnGPU, gpuImage, gpuResult, params);
#else
            auto elapsed = measure<>::execution(ProcessImageOnCPU, image, result, params, contours, hierarchy);
#endif

#if __USE_GPGPU__
            if (gpuResult.data == nullptr)
                return;

            gpuResult.download(result);
#else
            if (result.data == nullptr)
                return;
#endif

            DrawContours(result.size(), contours, hierarchy);

#if __USE_GPGPU__
            gpuResult.release();
#endif
            result.release();

            if (!textElapsedTime.expired()) {
                textElapsedTime.lock()->SetName(L"Elapsed time (ms): " + std::to_wstring(elapsed));
            }
        });
    }

    auto btnSave = window.AddControl<Button>(std::make_unique<Button>(L"Save", 152, 210 + 58, 292, 48));

    if (!btnSave.expired()) {
        btnSave.lock()->AddOnClickListener([btnSave, &contours]
        {
            std::string path;

            auto index = GetSavePath(btnSave.lock()->handle(), path, {
                Filter{L"Text file", L"*.*"}
            });

            if (index < 1)
                return;

            std::ofstream file(path, std::ios::out);
            std::istringstream stream;

            if (!file.is_open())
                return;
			
            std::cout.setf(std::ios::fixed);

            for (auto &&contour : contours) {
                std::sort(contour.begin(), contour.end(), [] (auto &&a, auto &&b)
                {
                    if (a.x < b.x)
                        return true;

                    if (a.x == b.x && a.y < b.y)
                        return true;

                    return false;
                });

                auto last = std::unique(contour.begin(), contour.end(), [] (auto &&a, auto &&b)
                {
                    return a.x == b.x && a.y == b.y;
                });

                contour.erase(last, contour.end());

                for (auto &&point : contour)
                    file << std::setw(8) << point.x << std::setw(8) << point.y << std::endl;
            }

            std::cout.setf(std::ios::right);
        });
    }

    auto const exitCode = Window::Update();

    if (result.data != nullptr)
        result.release();

    if (image.data != nullptr)
        image.release();

#if __USE_GPGPU__
    cv::cuda::resetDevice();
#endif
    cv::destroyAllWindows();

    return exitCode;
}