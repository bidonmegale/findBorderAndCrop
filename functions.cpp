#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;


bool ordenaContornosPelaArea(vector<Point> i, vector<Point> j) {

	vector<Point> ponto;
	vector<Point> ponto1;

	double peri = arcLength(i, true);
	approxPolyDP(i, ponto, 0.02 * peri, true);


	double peri1 = arcLength(j, true);
	approxPolyDP(j, ponto1, 0.02 * peri1, true);

	return contourArea(ponto) > contourArea(ponto1);
}


vector<Point> findBorders(Mat imagem, bool printImage) {

	vector<vector<Point>> contours;
	vector<Point> response;

	Mat imgGrayscale;
	Mat imgBlurred;
	Mat imgCanny;

	cvtColor(imagem, imgGrayscale, CV_BGR2GRAY);

	GaussianBlur(imgGrayscale,
		imgBlurred,
		Size(5, 5),
		1.8);

	Canny(imgBlurred,
		imgCanny,
		50,
		100);

	findContours(imgCanny, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	vector<Point> ponto;
	sort(contours.begin(), contours.end(), ordenaContornosPelaArea);


	for (int i = 0; i < contours.size(); i++) {

		double peri = arcLength(contours[i], true);
		approxPolyDP(contours[i], ponto, 0.02 * peri, true);

		vector<vector<Point>> bosta;
		bosta.push_back(ponto);
		
		if(printImage) {
			Scalar color = Scalar(0, 0, 255);
			drawContours(imagem, bosta, 0, color, 3);
		}

		break;

	}
	return ponto;

}

bool ordenaPontosPelaAltura(Point2f i, Point2f j) {
	return i.y > j.y;
}


Mat cropImage(Mat imagemOriginal, vector<Point> border) {

	Point2f tl;
	Point2f tr;
	Point2f br;
	Point2f bl;

	sort(border.begin(), border.end(), ordenaPontosPelaAltura);

	if (border[0].x < border[1].x) {
		tl = border[0];
		tr = border[1];
	} else {
		tl = border[1];
		tr = border[0];
	}


	if (border[2].x < border[3].x) {
		bl = border[2];
		br = border[3];
	} else {
		bl = border[2];
		br = border[3];
	}


	vector<Point2f> pontosImagem;
	pontosImagem.push_back(bl);
	pontosImagem.push_back(br);
	pontosImagem.push_back(tr);
	pontosImagem.push_back(tl);


	float widthA = sqrt(pow((br.x - bl.x), 2) + pow((br.y - bl.y), 2));
	float widthB = sqrt(pow((tr.x - tl.x), 2) + pow((tr.y - tl.y), 2));
	float maxWidth = max(widthA, widthB);

	float heightA = sqrt(pow((tr.x - br.x), 2) + pow((tr.y - br.y), 2));
	float heightB = sqrt(pow((tl.x - bl.x), 2) + pow((tl.y - bl.y), 2));
	float maxHeight = max(heightA, heightB);


	vector<Point2f> novosPontos;
	novosPontos.push_back(Point2f(0.0f, 0.0f));
	novosPontos.push_back(Point2f(maxWidth - 1.0f, 0.0f));
	novosPontos.push_back(Point2f(maxWidth - 1.0f, maxHeight - 1.0f));
	novosPontos.push_back(Point2f(0.0f, maxHeight - 1.0f));


	Mat warpMatrix = getPerspectiveTransform(pontosImagem, novosPontos);
	Mat saida;


	Size size(maxWidth, maxHeight);
	warpPerspective(imagemOriginal, saida, warpMatrix, size, INTER_LINEAR, BORDER_CONSTANT);

	return saida;


}


Mat corrigeIluminacao(Mat imagem) {
	
	Mat lab_image;
	cvtColor(imagem, lab_image, CV_BGR2Lab);

	// Extract the L channel
	vector<Mat> lab_planes(3);
	split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]

									   // apply the CLAHE algorithm to the L channel
	Ptr<CLAHE> clahe = createCLAHE();
	clahe->setClipLimit(4);
	Mat dst;
	clahe->apply(lab_planes[0], dst);

	// Merge the the color planes back into an Lab image
	dst.copyTo(lab_planes[0]);
	merge(lab_planes, lab_image);

	// convert back to RGB
	Mat image_clahe;
	cvtColor(lab_image, image_clahe, CV_Lab2BGR);

	return image_clahe;
}



Mat WBImage(Mat imagem) {

	Mat dst;
	cvtColor(imagem, dst, CV_BGR2GRAY);
	threshold(dst, dst, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	return dst;
}


int main() {

	Mat image = imread("c:\\cropTest\\carta.jpg", 1);
	Mat cropedImage;

	vector<Point> border = findBorders(image, true);

	namedWindow("Image Original", WINDOW_AUTOSIZE);
	imshow("Image Original", image);


	printf("%d", border.size());


	if (border.size() == 4) {
		cropedImage = cropImage(image, border);
	}


	if(border.size() == 4) {
		Mat WB = WBImage(cropedImage);
		namedWindow("Image Cropada", WINDOW_AUTOSIZE);
		imshow("Image Cropada", cropedImage);
	}


	waitKey(0);

}

