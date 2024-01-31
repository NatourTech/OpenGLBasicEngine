#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <string>
#include <fstream>



#define color_size_bytes 4
#define setBlackPixels(ARR, X, Y) ARR[X][(Y)] = 0; ARR[X][(Y)+1] = 0; ARR[X][(Y)+2] = 0; ARR[X][(Y)+3] = 255;
#define setWhitePixels(ARR, X, Y) ARR[X][(Y)] = 255; ARR[X][(Y)+1] = 255; ARR[X][(Y)+2] = 255; ARR[X][(Y)+3] = 255;
#define set_val(ARR, X, Y, val) ARR[X][(Y)] = val; ARR[X][(Y)+1] = val; ARR[X][(Y)+2] = val; ARR[X][(Y)+3] = 255;
#define set_val(ARR, INDEX, val) ARR[INDEX] = val; ARR[INDEX+1] = val; ARR[INDEX+2] = val; ARR[INDEX+3] = 255;
#define to_index(i, j) i * width * color_size_bytes + j * color_size_bytes
#define to_index_normal(i, j) (i) * width + (j)
#define calculatePixelAverage(ARR, X, Y) ((ARR[X][(Y)] + ARR[X][(Y)+1] + ARR[X][(Y)+2])/3)
#define PI 3.141592654


static void printMat(const glm::mat4 mat)
{
	std::cout<<" matrix:"<<std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout<< mat[j][i]<<" ";
		std::cout<<std::endl;
	}
}

Game::Game() : Scene()
{
}

Game::Game(float angle ,float relationWH, float near1, float far1) : Scene(angle,relationWH,near1,far1)
{ 	
}

void Game::Init()
{		

	AddShader("../res/shaders/pickingShader");	
	AddShader("../res/shaders/basicShader");
	

	// Add texture tyo lena image
	AddTexture("../res/textures/lena256.jpg", false);
	AddEdgesText();
	HalftoneAlg();
	FloydSteinbergAlg();
	


	//to delet when implement the other algorithms
	//AddTexture("../res/textures/lena256.jpg", false);
	//AddTexture("../res/textures/lena256.jpg", false);

	//Adding Shapes 
	AddShape(Plane, -1, TRIANGLES); 
	AddShape(Plane, -1, TRIANGLES); 
	AddShape(Plane, -1, TRIANGLES);
	AddShape(Plane, -1, TRIANGLES);


	for (int i = 0; i < 4; i++) {
		pickedShape = i;
		SetShapeTex(i, i);
	}
	MoveCamera(0,zTranslate,10);
	pickedShape = -1;
	
	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Game::Update(const glm::mat4 &MVP,const glm::mat4 &Model,const int  shaderIndx)
{
	Shader *s = shaders[shaderIndx];
	int r = ((pickedShape+1) & 0x000000FF) >>  0;
	int g = ((pickedShape+1) & 0x0000FF00) >>  8;
	int b = ((pickedShape+1) & 0x00FF0000) >> 16;
	s->Bind();
	s->SetUniformMat4f("MVP", MVP);
	s->SetUniformMat4f("Normal",Model);
	s->SetUniform4f("lightDirection", 0.0f , 0.0f, -1.0f, 0.0f);
	if(shaderIndx == 0)
		s->SetUniform4f("lightColor",r/255.0f, g/255.0f, b/255.0f,1.0f);
	else 
		s->SetUniform4f("lightColor",0.7f,0.8f,0.1f,1.0f);
	s->Unbind();
}

void Game::WhenRotate()
{
}

void Game::WhenTranslate()
{
}

void Game::Motion()
{
	if(isActive)
	{
	}
}

Game::~Game(void)
{
}




//canny  Alg Impolementation to detect edges
unsigned char* applyConv(const unsigned char* data, int width, int height, const glm::detail::tmat3x3<float, glm::highp>& filter) {
	auto* output = (unsigned char*)malloc(height * width * (color_size_bytes * sizeof(unsigned char)));
	memcpy(output, data, height * width * color_size_bytes * sizeof(unsigned char));
	for (int i = 1; i < height - 1; i++)
		for (int j = 1; j < width - 1; j++) {
			set_val(output, to_index(i, j), 0);
			for (int h = i; h < i + 3; h++)
				for (int w = j; w < j + 3; w++) {
					int newval = output[to_index(i, j)] + filter[h - i][w - j] * data[to_index(h, w)];
					newval = std::max(0, newval);
					newval = std::min(255, newval);
					set_val(output, to_index(i, j), newval);
				}
		}
	return output;
}

unsigned char* applyGaus(unsigned char* data, int width, int height) {
	//  3x3 Gaussian kernel
	auto filter = glm::detail::tmat3x3<float, glm::highp>(
		1, 2, 1,
		2, 4, 2,
		1, 2, 1);

	float sum = 16.0;
	filter = filter / sum;
	//Applying Convolution On The data matrix to gain blur image ( guasiian filter )
	unsigned char* output = applyConv(data, width, height, filter);
	return output;


	
}

// apply grad
int* applyGrad(const unsigned char* data, int width, int height, const glm::detail::tmat3x3<float, glm::highp>& filter) {
	int* output = (int*)malloc(width * height * sizeof(int));
	for (int i = 1; i < height - 1; i++)
		for (int j = 1; j < width - 1; j++) {
			output[to_index_normal(i, j)] = 0;
			for (int h = i; h < i + 3; h++)
				for (int w = j; w < j + 3; w++) {
					int newval = output[to_index_normal(i, j)] + filter[h - i][w - j] * data[to_index(h, w)];
					output[to_index_normal(i, j)] = newval;
				}
		}
	return output;
}

//grade_x
int* applyGradx(unsigned char* data, int width, int height) {
	auto filter = glm::detail::tmat3x3<float, glm::highp>(
		-1, 0, 1,
		-2, 0, 2,
		-1, 0, 1);
	int* output = applyGrad(data, width, height, filter);
	return output;
}
//grade_y
int* applyGrady(unsigned char* data, int width, int height) {
	auto filter = glm::detail::tmat3x3<float, glm::highp>(
		-1, -2, -1,
		0, 0, 0,
		1, 2, 1);
	int* output = applyGrad(data, width, height, filter);
	return output;
}

int*
getGrad(int* gradX, int* gradY, int height, int width) {
	auto* output = (int*)malloc(height * width * sizeof(int));
	//    int count = 0;
	memset(output, 0, height * width * sizeof(int));
	for (int i = 1; i < height - 1; i++)
		for (int j = 1; j < width - 1; j++) {
			int index = to_index_normal(i, j);
			int new_val = gradY[index] * gradY[index] + gradX[index] * gradX[index];
			new_val = std::sqrt(new_val);
			output[to_index_normal(i, j)] = new_val;
			//            count++;
		}
	//    std::cout << count << std::endl;
	return output;
}

//gradiant 

static double convert_to_degree(int rad) {
	return rad * (180 / PI) + 180;
}
double* GradientOrientation(int* grad_x, int* grad_y, int height, int width) {
	auto* gradient_orientation = (double*)malloc(height * width * sizeof(double));
	for (int y = 1; y < height - 1; y++) {
		for (int x = 1; x < width - 1; x++) {
			int index = to_index_normal(y, x);
			double theta = convert_to_degree(atan2(grad_y[index], grad_x[index]));
			gradient_orientation[index] = theta;
		}
	}
	return gradient_orientation;
}


int*
NonMaximumSuppression(int* gradient_magnitude, double* gradient_direction, int height, int width) {
	auto* output = (int*)calloc(height * width * sizeof(int), sizeof(int));
	int pi = 180;
	for (int y = 1; y < height - 1; y++) {
		for (int x = 1; x < width - 1; x++) {
			int index = to_index_normal(y, x);
			int before_pixel;
			int after_pixel;
			double direction = gradient_direction[index];
			//            std::cout << direction << std::endl;
			if ((0 <= direction < pi / 8) || (15 * pi / 8 <= direction <= 2 * pi)) {
				before_pixel = gradient_magnitude[to_index_normal(y, x - 1)];
				after_pixel = gradient_magnitude[to_index_normal(y, x + 1)];
			}
			else if ((pi / 8 <= direction * 3 * pi / 8) || (9 * pi / 8 <= direction < 11 * pi / 8)) {
				before_pixel = gradient_magnitude[to_index_normal(y + 1, x - 1)];
				after_pixel = gradient_magnitude[to_index_normal(y - 1, x + 1)];
			}
			else if ((3 * pi / 8 <= direction < 5 * pi / 8) || (11 * pi / 8 <= direction < 13 * pi / 8)) {
				before_pixel = gradient_magnitude[to_index_normal(y - 1, x)];
				after_pixel = gradient_magnitude[to_index_normal(y + 1, x)];
			}
			else {
				before_pixel = gradient_magnitude[to_index_normal(y - 1, x - 1)];
				after_pixel = gradient_magnitude[to_index_normal(y + 1, x + 1)];
			}
			if (gradient_magnitude[to_index_normal(y, x)] >= before_pixel &&
				gradient_magnitude[to_index_normal(y, x)] >= after_pixel) {
				output[to_index_normal(y, x)] = gradient_magnitude[to_index_normal(y, x)];
			}
		}
	}
	return output;
}




unsigned char*
Thresholding(int* grad, int highthresh, int lowthresh, int height, int width) {
	auto* output = (unsigned char*)malloc(height * width * color_size_bytes * sizeof(unsigned char));
	memset(output, 0, height * width * color_size_bytes * sizeof(unsigned char));
	std::ofstream txt_file;
	txt_file.open("img4.txt", std::ofstream::out);
	if (txt_file.is_open()) {
		for (int i = 1; i < height - 1; i++) {
			for (int j = 1; j < width - 1; j++) {
				int index = to_index(i, j);
				int val = grad[to_index_normal(i, j)];
				if (val < lowthresh) {
					set_val(output, index, 0);
					txt_file << 0 << ",";
				}
				else if (val < highthresh) {
					set_val(output, index, 0);
					txt_file << 0 << ",";
				}
				else {
					set_val(output, index, 255);
					txt_file << 1 << ",";
				}
			}
			txt_file << '\n';
		}
	}
	else {
		std::cout << "error in opening file" << std::endl;
	}
	txt_file.close();
	return output;
}


void Game::AddEdgesText() {
	int max_grad = std::sqrt(255 * 255 * 2);
	int width, height, numComponents;
	unsigned char* data = stbi_load("../res/textures/lena256.jpg",&width, &height, &numComponents, 4);
	data = applyGaus(data, width, height);
	int* gradX = applyGradx(data, width, height);
	int* gradY = applyGrady(data, width, height);
	int* grad = getGrad(gradX, gradY, height, width);
	double* theta = GradientOrientation(gradX, gradY, height, width);
	int* suppressed = NonMaximumSuppression(grad, theta, height, width);
	data = Thresholding(suppressed, max_grad * 0.5, max_grad * 0.3, height, width);
	delete[]  gradX;
	delete[] gradY;
	delete[] grad;
	
	AddTexture(width, height, data);
}


static int round(int val, int val_amount) {
	return (val / val_amount) * val_amount;
}

static void WriteToTxt6(unsigned char* output, int width, int height, std::string name) {
	std::ofstream txt_file;
	txt_file.open(name, std::ofstream::out);
	if (txt_file.is_open()) {
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				txt_file << (int)output[to_index(i, j)] / 16 << ",";
			}
			txt_file << '\n';
		}
	}
	else {
		std::cout << "error in opening file" << std::endl;
	}
	txt_file.close();
}

void Game::FloydSteinbergAlg() {
	int width, height, numComponents;
	unsigned char* data = stbi_load("../res/textures/lena256.jpg", &width, &height, &numComponents, 4);

	//for high percesion we used double 
	double alpha = 7.0 / 16;
	double beta = 3.0 / 16;
	double gamma = 5.0 / 16;
	double delta = 1.0 / 16;

	unsigned char* output = (unsigned char*)malloc(height * width * color_size_bytes * sizeof(unsigned char));

	for (int y = 0; y < height - 1; y++) {
		for (int x = 1; x < width - 1; x++) {
			int oldPixel = data[to_index(x, y)];

			//Adjust threshold for 16 levels
			int newPixel = round(oldPixel, 16);
			int error = oldPixel - newPixel;

			// Update current pixel
			set_val(output, to_index(x, y), newPixel);



			// Distribute error to neighboring pixels
			set_val(data, to_index(x + 1, y), data[to_index(x + 1, y)] + (error * alpha));
			set_val(data, to_index(x - 1, y + 1), data[to_index(x - 1, y + 1)] + (error * beta));
			set_val(data, to_index(x, y + 1), data[to_index(x, y + 1)] + (error * gamma));
			set_val(data, to_index(x + 1, y + 1), data[to_index(x + 1, y + 1)] + (error * delta));
		}
	}

	WriteToTxt6(output, width, height, "img6.txt");

	AddTexture(width, height, output);
}



//helper function: 1D array To 2D Array
static unsigned char** convertTo2DArray(unsigned char* data, int width, int height) {
	unsigned char** result = (unsigned char**)malloc(height * sizeof(unsigned char*));
	for (int i = 0; i < height; i++)
		result[i] = (unsigned char*)malloc(width * 4);
	int dataSize = color_size_bytes * width * height;
	for (int i = 0; i < dataSize; i++) {
		result[i / (width * 4)][i % (4 * width)] = data[i];
	}
	return result;
}

static unsigned char* convertTo1DArray(unsigned char** arr, int width, int height) {
	unsigned char* output = (unsigned char*)malloc(width * height * sizeof(uint64_t));
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width * 4; j++)
			output[i * width * 4 + j] = arr[i][j];
	return output;
}

static void WriteToTxt(unsigned char* data, int width, int height, std::string name) {
	std::ofstream txt_file;
	txt_file.open(name, std::ofstream::out);
	if (txt_file.is_open()) {
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				txt_file << (int)data[to_index(i, j)] / 255 << ",";
			}
			txt_file << '\n';
		}
	}
	else {
		std::cout << "error in opening file" << std::endl;
	}
	txt_file.close();
}

void Game::HalftoneAlg() {
	int width, height, numComponents;
	unsigned char* imageData = stbi_load("../res/textures/lena256.jpg", &width, &height, &numComponents, 4);
	unsigned char** image2DArray = convertTo2DArray(imageData, width, height);
	unsigned char** output2DArray = (unsigned char**)malloc(2 * height * sizeof(unsigned char*));
	for (int i = 0; i < 2 * height; i++)
		output2DArray[i] = (unsigned char*)malloc(2 * width * color_size_bytes * sizeof(unsigned char));
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width * color_size_bytes; j = j + color_size_bytes) {
			int average = calculatePixelAverage(image2DArray, i, j);
			if (average < 0.20 * 255) {
				setBlackPixels(output2DArray, 2 * i, 2 * j);
				setBlackPixels(output2DArray, 2 * i, 2 * j + color_size_bytes);
				setBlackPixels(output2DArray, 2 * i + 1, 2 * j);
				setBlackPixels(output2DArray, 2 * i + 1, 2 * j + color_size_bytes);
			}
			else if (average < 0.4 * 255) {
				setWhitePixels(output2DArray, 2 * i, 2 * j);
				setBlackPixels(output2DArray, 2 * i, 2 * j + color_size_bytes);
				setBlackPixels(output2DArray, 2 * i + 1, 2 * j);
				setBlackPixels(output2DArray, 2 * i + 1, 2 * j + color_size_bytes);
			}
			else if (average < 0.6 * 255) {
				setWhitePixels(output2DArray, 2 * i, 2 * j);
				setBlackPixels(output2DArray, 2 * i, 2 * j + color_size_bytes);
				setBlackPixels(output2DArray, 2 * i + 1, 2 * j);
				setWhitePixels(output2DArray, 2 * i + 1, 2 * j + color_size_bytes);
			}
			else if (average < 0.8 * 255) {
				setWhitePixels(output2DArray, 2 * i, 2 * j);
				setWhitePixels(output2DArray, 2 * i, 2 * j + color_size_bytes);
				setBlackPixels(output2DArray, 2 * i + 1, 2 * j);
				setWhitePixels(output2DArray, 2 * i + 1, 2 * j + color_size_bytes);
			}
			else {
				setWhitePixels(output2DArray, 2 * i, 2 * j);
				setWhitePixels(output2DArray, 2 * i, 2 * j + color_size_bytes);
				setWhitePixels(output2DArray, 2 * i + 1, 2 * j);
				setWhitePixels(output2DArray, 2 * i + 1, 2 * j + color_size_bytes);
			}


		}
	// Text writing:
	imageData = convertTo1DArray(output2DArray, 2 * width, 2 * height);
	WriteToTxt(imageData, 2 * width, 2 * height, "img5.txt");
	AddTexture(2 * width, 2 * height, imageData);

}