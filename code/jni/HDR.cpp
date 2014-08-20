#include <string>
#include <opencv2/opencv.hpp>
#include "NativeLogging.h"

using namespace std;
using namespace cv;

const int numChannels = 3;
static const char* TAG = "CreateHDR";

static void show_image(Mat &image) {
	if (!image.data) {
		cout << "Image is missing\n";
		exit (-1);
	}
	cout << "Image size: " << image.rows << " x " << image.cols << "\n";
	namedWindow( "Display window", WINDOW_AUTOSIZE );
	imshow("Display window", image);
	waitKey();

}

void read_input_images(vector<string>& inputImagePaths, vector<Mat>& images)
{
    /* Read the images at the locations in inputImagePaths
     * and store them in the images vector.
     * Helpful OpenCV functions: imread.
     */
	for (auto path : inputImagePaths) {
		images.push_back(imread(path));
	}
}

int showORBFeatures(Mat &image) {
	Mat descriptor;
	vector<KeyPoint> keypoints;

	ORB detector (20000);
	detector(image, Mat(), keypoints,descriptor);
	// draw these points in the image
	for(auto keypoint : keypoints) {
		circle(image,
				keypoint.pt,
				3,
				Scalar(0, 255, 0));
	}

	return keypoints.size();
}


void align_images(vector<Mat>& images)
{
    /* Update the images vector with aligned images.
     * Helpful OpenCV classes and functions:
     * ORB, BFMatcher, findHomography, warpPerspective
     * The above classes and functions are just recommendations.
     * You are free to experiment with other methods as well.
     */
    vector<Mat> descriptors(images.size());
    vector<vector<KeyPoint>> keypoints(images.size());

	ORB detector (2000);
    
    BFMatcher matcher (NORM_HAMMING, true);
    int refImageIdx = 0;

    for(int i = 0; i < images.size(); i++) {
		detector(images[i], Mat(), keypoints[i],descriptors[i]);
        vector<vector<DMatch> > matches;
        if(i == refImageIdx)
            continue;

        matcher.knnMatch(descriptors[i], descriptors[refImageIdx], matches, 1);

        // find the homography
		vector<Point2f> trainPts;
        vector<Point2f> queryPts;
		for (int j = 0; j < matches.size(); j++) {
			if (matches[j].size() > 0) {
				trainPts.push_back(keypoints[refImageIdx][matches[j][0].trainIdx].pt);
				queryPts.push_back(keypoints[i][matches[j][0].queryIdx].pt);
			}
		}

        Mat H12 = findHomography(queryPts, trainPts, CV_RANSAC, 10);

        Mat transformedImage;
        warpPerspective(images[i], transformedImage, H12, images[i].size());
        images[i] = transformedImage;
    }
}

static void compute_weights(vector<Mat>& images, vector<Mat>& weights)
{
    /*
     *  Compute the weights for each image and place them in the given weights vector.
     *  Helpful OpenCV functions: Laplacian, split, merge.
     */
	for (auto imgIn : images) {
		//Get contrast
		Mat imgContrast(imgIn.rows,imgIn.cols, CV_32F);
		Laplacian(imgIn, imgContrast, CV_32F);

		//Get saturation
		vector<Mat> imgOutChannels;
		Mat imgStdDiv(imgIn.rows,imgIn.cols, CV_32F);
		split (imgIn, imgOutChannels);
		for (int i = 0; i < imgIn.rows; i++) {
			for (int j = 0; j < imgIn.cols; j++) {
				float avg = (float)((int)imgOutChannels[0].at<float>(i,j)
								   +(int)imgOutChannels[1].at<float>(i,j)
								   +(int)imgOutChannels[2].at<float>(i,j))/3.0;
				float sqr_avg = pow(avg,2.0);
				float avg_sqr = (pow(imgOutChannels[0].at<float>(i,j),2.0)
								+pow(imgOutChannels[1].at<float>(i,j),2.0)
								+pow(imgOutChannels[2].at<float>(i,j),2.0))/3.0;
				float std = sqrt(avg_sqr - sqr_avg);
				imgStdDiv.at<float>(i,j) = std;
			}
		}

		//Get exposure
		Mat imgExpo(imgIn.rows,imgIn.cols, CV_32F);
		for (int i = 0; i < imgIn.rows; i++) {
			for (int j = 0; j < imgIn.cols; j++) {
				float exponent[3];
				for (int k = 0; k < 3; k++) {
					float exponent_ = pow((imgOutChannels[k].at<float>(i,j)-0.5),2)
									  /(2*0.2*0.2)*(-1);
					exponent[k] = exp(exponent_);
				}
				imgExpo.at<float>(i,j) = exponent[0] * exponent[1] * exponent[2];
			}
		}

		//Build weights
		Mat W(imgIn.rows,imgIn.cols, CV_32F);
		for (int i = 0; i < W.rows; i++) {
			for (int j = 0; j < W.cols; j++) {
				W.at<float>(i,j) = //pow(imgContrast.at<float>(i,j),1.0)
								 pow(imgExpo.at<float>(i,j), 1.0);
								 //* pow(imgStdDiv.at<float>(i,j),1.0);
			}
		}
		weights.push_back(W);
	}
	//Normalization of weights
	for (int i = 0; i < images[0].rows; i++) {
		for (int j = 0; j < images[0].cols; j++) {
			float denom = 0.0;
			for (auto &weight : weights) {
				denom += weight.at<float>(i,j);
			}
			for (auto &weight : weights) {
				if (denom) {
					weight.at<float>(i,j) = weight.at<float>(i,j)/denom;
				}
				if (isnan(weight.at<float>(i,j))) {
					weight.at<float>(i,j) = 1.0/(images.size());
				}
			}
		}
	}
}

void get_gaussian_pyr(int numLevels, Mat &inMat, vector<Mat> &gaussPyr) {
	buildPyramid(inMat,gaussPyr,numLevels);
}

void get_laplacian_pyr(int numLevels, Mat &imgIn, vector<Mat> &laplacePyr) {
	cout << "laplacian in \n";
	get_gaussian_pyr(numLevels-1, imgIn, laplacePyr);
	for (int i = 1; i < numLevels; i++) {
		Mat pyrUpImg;
		pyrUp(laplacePyr[i], pyrUpImg);
		resize(pyrUpImg, pyrUpImg, laplacePyr[i-1].size());
		subtract(laplacePyr[i-1], pyrUpImg, laplacePyr[i-1]);
	}
	cout << "laplacian out \n";
}

void collapse_laplacian_pyr(int numLevels, vector<Mat> &laplacePyr, Mat &imgOut) {
	imgOut = laplacePyr[laplacePyr.size()-1];
	for (int i = numLevels - 2; i >= 0; i--) {
		Mat pyrUpImg;
		pyrUp(imgOut, pyrUpImg);
		resize(pyrUpImg, pyrUpImg, laplacePyr[i].size());
		add(laplacePyr[i], pyrUpImg, imgOut);
	}
}

string getImgType(int imgTypeInt)
{
	int numImgTypes = 35; // 7 base types, with five channel options each (none or C1, ..., C4)

	int enum_ints[] =       {CV_8U,  CV_8UC1,  CV_8UC2,  CV_8UC3,  CV_8UC4,
		CV_8S,  CV_8SC1,  CV_8SC2,  CV_8SC3,  CV_8SC4,
		CV_16U, CV_16UC1, CV_16UC2, CV_16UC3, CV_16UC4,
		CV_16S, CV_16SC1, CV_16SC2, CV_16SC3, CV_16SC4,
		CV_32S, CV_32SC1, CV_32SC2, CV_32SC3, CV_32SC4,
		CV_32F, CV_32FC1, CV_32FC2, CV_32FC3, CV_32FC4,
		CV_64F, CV_64FC1, CV_64FC2, CV_64FC3, CV_64FC4};

	string enum_strings[] = {"CV_8U",  "CV_8UC1",  "CV_8UC2",  "CV_8UC3",  "CV_8UC4",
		"CV_8S",  "CV_8SC1",  "CV_8SC2",  "CV_8SC3",  "CV_8SC4",
		"CV_16U", "CV_16UC1", "CV_16UC2", "CV_16UC3", "CV_16UC4",
		"CV_16S", "CV_16SC1", "CV_16SC2", "CV_16SC3", "CV_16SC4",
		"CV_32S", "CV_32SC1", "CV_32SC2", "CV_32SC3", "CV_32SC4",
		"CV_32F" , "CV_32FC1", "CV_32FC2", "CV_32FC3", "CV_32FC4",
		"CV_64F", "CV_64FC1", "CV_64FC2", "CV_64FC3", "CV_64FC4"};

	for(int i=0; i<numImgTypes; i++)
	{
		if(imgTypeInt == enum_ints[i]) return enum_strings[i];
	}
	return "unknown image type";
}

void blend_pyramids(int numLevels, vector<Mat>& images, vector<Mat>& weights, Mat& output)
{
    /*
     *  Blend the images using the calculated weights.
     *  Store the result of exposure fusion in the given output image.
     *  Helpful OpenCV functions: buildPyramid, pyrUp.
     */
	//Get gaussian pyr for weights

	vector<Mat> outputPyr(numLevels);
	vector<Mat> junksPyr;
	get_gaussian_pyr(numLevels, weights[0], junksPyr);
	for (int i = 0; i < numLevels; i++) {
		outputPyr[i] = Mat::zeros(junksPyr[i].rows, junksPyr[i].cols, CV_32FC3);
	}

	for(int i = 0; i < images.size(); i++) {
		vector<Mat> weightPyr;
		get_gaussian_pyr(numLevels-1, weights[i], weightPyr);
	
		Mat imgIn = images[i];
		
		//Get laplacian pyr for images
		vector<Mat> imgPyr(numLevels);
		get_laplacian_pyr(numLevels, imgIn, imgPyr);
		for (int k = 0; k < numLevels; k++) {
			vector<Mat> imgChannels;
			split(imgPyr[k], imgChannels);
			for (auto &imgCh : imgChannels) {
				multiply(imgCh, weightPyr[k], imgCh);
			}
			merge(imgChannels, imgPyr[k]);
			add(imgPyr[k], outputPyr[k], outputPyr[k]);
		}
	}
	collapse_laplacian_pyr(numLevels, outputPyr, output);
}

bool CreateHDR(vector<string> inputImagePaths, string outputImagePath)
{
    //Read in the given images.
    vector<Mat> images;
    read_input_images(inputImagePaths, images);

    //Verify that the images were correctly read.
    if(images.empty() || images.size()!=inputImagePaths.size())
    {
        LOG_ERROR(TAG, "Images were not read in correctly!");
        return false;
    }

    //Verify that the images are RGB images of the same size.
    Size imgSize = images[0].size();
    for(const Mat& img: images)
    {
        if(img.channels()!=numChannels)
        {
            LOG_ERROR(TAG, "CreateHDR expects 3 channel RGB images!");
            return false;
        }
        if(img.size()!=imgSize)
        {
            LOG_ERROR(TAG, "HDR images must be of equal sizes!");
            return false;
        }
    }
    LOG_INFO(TAG, "%lu images successfully read.", images.size());

    //Now we'll make sure that the images line up correctly.
//	for (auto &img : images)
//		show_image(img);
    align_images(images);
    LOG_INFO(TAG, "Image alignment complete.");

	//Convert image pixels from unsigned char to float
	for (auto &img : images) {
		Mat img_new(img.rows,img.cols,CV_32FC3);
		for (int r = 0; r < img.rows; r++) {
			for (int c = 0; c < img.cols; c++) {
				for (int k = 0; k < 3; k++) {
					img_new.at<Vec3f>(r,c)[k] = img.at<Vec3b>(r,c)[k]/255.0;
				}
			}
		}
		img = img_new;
	}
    //Compute the weights for each image.
    vector<Mat> weights;
    compute_weights(images, weights);
    if(weights.size()!=images.size())
    {
        LOG_ERROR(TAG, "Image weights were not generated!");
        return false;
    }
    LOG_INFO(TAG, "Weight computation complete.");

    //Fusion using pyramid blending.
    int numLevels = 9;
    Mat outputImage;
    blend_pyramids(numLevels, images, weights, outputImage);
    if(outputImage.empty())
    {
        LOG_ERROR(TAG, "Blending did not produce an output!");
        return false;
    }
    LOG_INFO(TAG, "Blending complete!");

	//Convert image pixels from float to unsigned char
	Mat img_new(outputImage.rows,outputImage.cols,CV_8UC3);
	for (int r = 0; r < outputImage.rows; r++) {
		for (int c = 0; c < outputImage.cols; c++) {
			for (int k = 0; k < 3; k++) {
				img_new.at<Vec3b>(r,c)[k] = saturate_cast<uchar>(outputImage.at<Vec3f>(r,c)[k]*255.0);
			}
		}
	}
	outputImage = img_new;
    //Save output.
    bool result = imwrite(outputImagePath, outputImage);
	//show_image(outputImage);
    if(!result)
    {
        LOG_ERROR(TAG, "Failed to save output image to file!");
    }
	vector<Mat> imgPyr(numLevels);
	get_laplacian_pyr(numLevels, outputImage, imgPyr);
	int counter = 0;
	for (auto &img : imgPyr) {
		stringstream ss;
		ss << counter;
		imwrite(ss.str()+".jpg", img);
		counter++;
	}
    return result;
}
