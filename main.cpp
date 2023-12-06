#include<iostream>
#include <iomanip> 
#include<fstream>
#include<vector>
#include <SFML/Graphics.hpp>

#define _CRT_SECURE_NO_WARNINGS

typedef std::vector <float>  vector_t; // Vector
typedef std::vector <vector_t> vector2_t;// Two-dimensional vector (matrix)


const int N = 11; // The number of correct images


class Reading {
	vector_t X;// The vector of pixels fed to the input

public:
	Reading() { }
	
	Reading(const char* filename, int* width, int* height) {
		
		vector_t bmp;
		FILE* f;
		fopen_s(&f,filename, "rb");
		
		if (f == NULL) {
			std::cerr << "Файл не найден!" << std::endl;
			width = NULL;
			height = NULL;
		}

		unsigned char info[54];
		fread(info, sizeof(unsigned char), 54, f); // Read the 54-byte header

		// Extract image height and width from header
		*width = *(int*)&info[18];
		*height = *(int*)&info[22];
		int data_offset = *(int*)(&info[0x0A]);
		fseek(f, (long int)(data_offset - 54), SEEK_CUR);

		int row_padded = (*width * 3 + 3) & (~3);
		unsigned char* data = new unsigned char[row_padded];
		unsigned char tmp;

		for (int i = 0; i < *height; i++)
		{
			fread(data, sizeof(unsigned char), row_padded, f);
			for (int j = 0; j < *width * 3; j += 3)
			{
				tmp = data[j];
				data[j] = data[j + 2];
				data[j + 2] = tmp;


				data[j] / 255 > 0? bmp.push_back(-1) : bmp.push_back(1);
				
			}
		}
		vector_t x;
		int h = 0;
		for (int i = 0; i < (*width) * (*height); i++) {
			++h;
			x.push_back(bmp[(*width) * (*height) - i-1]);
			if (h % 10 == 0) {
				for (int j = 0; j < (*width); j++) {
					X.push_back(x[(*width) - j - 1]);
				}
				x.clear();
			}
		}
		
	}
	

	/// <summary>
	/// Defining the dimension of a vector
	/// </summary>
	/// <returns>Output of the value in integer form</returns>
	int XSize() {
		return X.size();
	}
	/// <summary>
	/// Output of the vector whose values were found from the photo
	/// </summary>
	/// <returns>Vector output</returns>
	vector_t XOutput() {
		return X;
	}

	void PrintX() {
		std::cout << "\n\n";
		for (int i = 0; i < X.size(); i++) {
			std::cout  << std::setw(2) << X[i] << ',';
		}
		std::cout << '\n';
	}
};

class HopfieldNetwork {
public:
	HopfieldNetwork() { size = 0;}
	HopfieldNetwork(int size) : size(size), weights(size, std::vector<double>(size, 0)) {}


	vector_t& operator [] (int index) {
		return V[index];
	}

	void train(const vector_t  patterns) {
		for (int i = 0; i < size; ++i) {
			for (int j = 0; j < size; ++j) {
				if (i == j) {
					weights[i][j] = 0;
				}
				else {
					
						weights[i][j] += patterns[i] * patterns[j];
					
					weights[i][j] /= size;
				}
			}
		}
	}

	friend vector_t recall(const vector_t& input, Reading** X, HopfieldNetwork* vectorWithVector, int maxIterations);

private:
	vector_t* V;
	int size;
	std::vector<std::vector<double>> weights;
};

vector_t recall(const vector_t& input,Reading **X, HopfieldNetwork *vectorWithVector, int maxIterations ) {
		vector_t output = input;

		int prozent[N]; // An array of percentages for comparison
		int max_index = 0; // The index of the maximum percentage
		int max = 0; // The maximum number
		
		for (int i = 0; i < N; i++) {
			prozent[i] = 0;
			// Comparing a vector entered by the user with vectors with reference images
			// To understand which image we are restoring the image to
			for (int j = 0; j < input.size(); j++) {
				if (input[j] == X[i]->XOutput()[j]) {
					prozent[i] = prozent[i] + 1;
				}
			}
			if (prozent[i] > max) {
				max_index = i;
				max = prozent[i];
			}
		}
		// A cycle for restoring a damaged image
		for (int k = 0; k < maxIterations; ++k) {
			for (int i = 0; i < input.size(); ++i) {
				double net = 0;
				for (int j = 0; j < input.size(); ++j) {
					net += vectorWithVector[max_index].weights[i][j] * output[j];
				}
				output[i] = net >= 0 ? 1 : -1;
			}
		}
		return output;
}



int main()
{
	setlocale(LC_ALL, "rus");
	// The width of the image
	int width = 10;
	// The length of the image
	int height = 10;
	
	Reading **X = new Reading*[N];
	
	// An array of their class instances. Our correct pictures	
	X[0] = new Reading("picture\\0.bmp",&width,&height);
	X[1] = new Reading("picture\\1.bmp", &width, &height);
	X[2] = new Reading("picture\\2.bmp", &width, &height);
	X[3] = new Reading("picture\\3.bmp", &width, &height);
	X[4] = new Reading("picture\\4.bmp", &width, &height);
	X[5] = new Reading("picture\\4_1.bmp", &width, &height);
	X[6] = new Reading("picture\\5.bmp", &width, &height);
	X[7] = new Reading("picture\\6.bmp", &width, &height);
	X[8] = new Reading("picture\\7.bmp", &width, &height);
	X[9] = new Reading("picture\\8.bmp", &width, &height);
	X[10] = new Reading("picture\\9.bmp", &width, &height);
	
	HopfieldNetwork network[N];
	

	for (int i = 0; i < N; i++) {
		network[i] =  HopfieldNetwork(100);
		network[i].train(X[i]->XOutput());
		X[i]->PrintX();
				
	}
	
	
	sf::RenderWindow window(sf::VideoMode(800, 550), L"Нейронная сеть определяет цифры по пикселям"); //новое окно

	window.setVerticalSyncEnabled(true);


	// An array of our squares
	sf::RectangleShape* prectangle = new sf::RectangleShape[100];
	int x =10, y = 10, h=0;
	for (int i = 0; i < 100; i++) {
		++h;
		prectangle[i].setPosition(sf::Vector2f(x, y)); // The position of our square
		prectangle[i].setSize(sf::Vector2f(50, 50)); // The size of the square
		prectangle[i].setFillColor(sf::Color::White);// Image texture
		prectangle[i].setOutlineThickness(2); // The width of the outline of the square
		prectangle[i].setOutlineColor(sf::Color::Black);// The white color of the outline of the square
		x += 50; // Going through the lines
		if (h %10==0) { y += 50; x = 10; }
	}

	// Rectangle for the image recovery button
	sf::RectangleShape button1; 
	button1.setPosition(600, 100); 
	button1.setSize(sf::Vector2f(120, 70)); 
	button1.setFillColor(sf::Color(250, 150, 100));
	button1.setOutlineThickness(2); 
	button1.setOutlineColor(sf::Color::Black);

	// Rectangle for the button to erase all squares
	sf::RectangleShape button2; 
	button2.setPosition(600, 250); 
	button2.setSize(sf::Vector2f(120, 70)); 
	button2.setFillColor(sf::Color::Cyan);
	button2.setOutlineThickness(2);
	button2.setOutlineColor(sf::Color::Black);



	sf::Font font;
	if (!font.loadFromFile("russhirift.ttf"))
	{
		std::cout << "Ошибка при открытии файла со шрифтом" << '\n';
	}

	// The text on the button 1
	sf::Text TextOnButton1; 
	TextOnButton1.setPosition(610, 120);
	TextOnButton1.setFont(font);
	TextOnButton1.setString(L"Нажми и узнай\nкакая цифра");
	TextOnButton1.setCharacterSize(15);
	TextOnButton1.setFillColor(sf::Color::Black); 

	// The text on the button 2
	sf::Text TextOnButton2; 
	TextOnButton2.setPosition(605, 270);
	TextOnButton2.setFont(font);
	TextOnButton2.setString(L"Нажми и сотри\nквадраты"); 
	TextOnButton2.setCharacterSize(15);
	TextOnButton2.setFillColor(sf::Color::Black);
	

	

	while (window.isOpen())
	{
		sf::Event event;
		sf::Vector2i pos = sf::Mouse::getPosition(window);// Reserved the mouse in the vector by getting its coordinates

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			int x = 10, y = 10, h = 0;
			for (int i = 0; i < 100; i++) {
				++h;
				if (event.type == sf::Event::MouseButtonPressed)// Checking that the mouse is pressed
				{
					if (event.key.code == sf::Mouse::Left)// And specifically the left one
					{
						if (prectangle[i].getGlobalBounds().contains(pos.x, pos.y))// If the mouse falls into the sprite

						{
							// We change the color to the opposite
							if (prectangle[i].getFillColor() == sf::Color::Black) {
								prectangle[i].setFillColor(sf::Color::White);
							}
							else {
								prectangle[i].setFillColor(sf::Color::Black);
							}
						}
					}
				}
				x += 50; 
				if (h % 10 == 0) { y += 50; x = 10; }
			}

			if (event.type == sf::Event::MouseButtonPressed)// Checking that the mouse is pressed
			{
				if (event.key.code == sf::Mouse::Left)// And specifically the left one
				{
					if (button2.getGlobalBounds().contains(pos.x, pos.y))// If the mouse hits the erase all squares button
					{
						// Make all squares white
						for (int i = 0; i < 100; i++) {
							prectangle[i].setFillColor(sf::Color::White);
						}
					}
				}
			}

			if (event.type == sf::Event::MouseButtonPressed)// Checking that the mouse is pressed
			{
				if (event.key.code == sf::Mouse::Left)// And specifically the left one
				{
					if (button1.getGlobalBounds().contains(pos.x, pos.y))// If the mouse clicks on the number recognition button
					{
						vector_t y;
						for (int i = 0; i < 100; i++) {
							if (prectangle[i].getFillColor() == sf::Color::Black) {
								y.push_back(1);
							}
							else y.push_back(-1);
						}
						 // Restoring the image
						vector_t output = recall(y, X, network,100);
						
						// The restored image is depicted on sprites
						int h = 0;
						for (int i = 0; i < output.size(); i++) {
							if (output[i] == -1) {
								prectangle[i].setFillColor(sf::Color::White);
							}
							else {
								prectangle[i].setFillColor(sf::Color::Black);
							}
							
						}
						
					}
				}
			}


		}

		window.clear(sf::Color::White);
		for (int i = 0; i < 100; i++) {
			window.draw(prectangle[i]);
		}

		window.draw(button1);
		window.draw(TextOnButton1);
		window.draw(button2);
		window.draw(TextOnButton2);
		window.display();
	}

	return 0;
}