/*
	Thomas Rapp  tsrapp@g.clemson.edu
	Affine Warp
	
	Description: This program utilizes OpenGL to display and manipulate an image using affine
	warping procedures.
*/

#include "Matrix.h"
#include "Vector.h"
#include <cstdlib>
#include <iostream>
#include <cstring>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <OpenImageIO/imageio.h>
OIIO_NAMESPACE_USING

using namespace std;

struct pixel {
	float r, g, b, a;
};

int W, H, W2, H2; 		 	//width and height resolutions of image
int channels; 				 	//amount of color channels in image(3 or 4)
float* rawpix;   			 	//raw pixel data from read_image, 1d array
char* writeFile;			 	//filename to write to
bool write = false;
pixel **mypix;			    	//2d array for image manipulation
pixel **output;			 	//output image data
pixel **save;

int readImage(char* file) {

	ImageInput *in = ImageInput::open(file);
	if (!in) {
		std::cerr << "Could not open file: " << geterror() << std::endl;
		exit(-1);
	}

	const ImageSpec spec = in->spec();
	W = spec.width;
	H = spec.height;
	channels = spec.nchannels;
	
	//allocate memory for data structures
 	rawpix = new float[W*H*channels];
   
   mypix = new pixel*[H];
   mypix[0] = new pixel[W*H];
   for (int i = 1; i < H; i++)
   	mypix[i] = mypix[i-1] + W;
	
	//read image data
	if (!in->read_image(TypeDesc::TypeFloat, rawpix)) {
      std::cerr << "Could not read pixels from " << file;
      std::cerr << ", error = " << in->geterror() << std::endl;
      delete in;
      exit(-1); 
   }
   
   //store image data in 2d array
	for (int row = 0; row < H; row++) {
		for (int col = 0; col < W; col++) {
			mypix[row][col].r = rawpix[channels*((H-row)*W + col)];
			mypix[row][col].g = rawpix[channels*((H-row)*W + col)+1];
			mypix[row][col].b = rawpix[channels*((H-row)*W + col)+2];
			mypix[row][col].a = 1.0; 
		}
	}
   
   if (!in->close()) {
      std::cerr << "Error closing " << file;
      std::cerr << ", error = " << in->geterror() << std::endl;
      delete in;
      exit(-1);
   }
   delete in;

   return 0;
}

//read pixel data from display and write to specified file
//input: file to write to
int saveImage(char* filename) {
	ImageOutput *out = ImageOutput::create(filename);
	if (!out) {
		std::cerr << "Could not create: " << geterror();
		exit(-1);
	}

	//read pixel data from display, store in 2d array
	glReadPixels(0,0,W2,H2,GL_RGBA,GL_FLOAT,output[0]);
	
	save = new pixel*[H2];
   save[0] = new pixel[W2*H2];
   for (int i = 1; i < H2; i++)
   	save[i] = save[i-1] + W2;
	
	//'flip' image data 
	for (int row = 0; row < H2; row++) {
   	for (int col = 0; col < W2; col++) {
   		save[row][col].r = output[H2-row-1][col].r;
			save[row][col].g = output[H2-row-1][col].g;
			save[row][col].b = output[H2-row-1][col].b;
			save[row][col].a = output[H2-row-1][col].a;
   	}
   }
   
	//open file according to image specs, write image data to file
	ImageSpec newspec (W2,H2,4,TypeDesc::TypeFloat);
	out->open(filename,newspec);
	out->write_image(TypeDesc::TypeFloat,save[0]);

	if (!out->close()) {
		std::cerr << "Error closing " << filename;
		std::cerr << ", error = " << out->geterror() << std::endl;
		delete out;
		exit(-1);
	}
	delete out;
	
	return 0;
}

/*
   Convert the string s to lower case
*/
void lowercase(char *s){
   int i;

   if(s != NULL) {
      for(i = 0; s[i] != '\0'; i++) {
         if(s[i] >= 'A' && s[i] <= 'Z')
            s[i] += ('a' - 'A');
      }
   }
}

/* 
   Multiply M by a rotation matrix of angle theta
*/
void Rotate(Matrix3x3 &M, float theta){
   int row, col;
   Matrix3x3 R(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
   double rad, c, s;

   rad = PI * theta / 180.0;
   c = cos(rad);
   s = sin(rad);

   R[0][0] = c;
   R[0][1] = -s;
   R[1][0] = s;
   R[1][1] = c;

   Matrix3x3 Prod = R * M;

   for(row = 0; row < 3; row++) {
      for(col = 0; col < 3; col++) {
         M[row][col] = Prod[row][col];
      }
   }
}

void Scale(Matrix3x3 &M, float x, float y) {
	int row, col;
	Matrix3x3 S(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	
	S[0][0] *= x;
	S[1][1] *= y;
	
	Matrix3x3 Prod = S * M;
	
	for(row = 0; row < 3; row++) {
      for(col = 0; col < 3; col++) {
         M[row][col] = Prod[row][col];
      }
   }
}

void Translation(Matrix3x3 &M, float x, float y) {
	int row, col;
	Matrix3x3 T(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	
	T[0][2] = x;
	T[1][2] = y;
	
	Matrix3x3 Prod = T * M;
	
	for(row = 0; row < 3; row++) {
      for(col = 0; col < 3; col++) {
         M[row][col] = Prod[row][col];
      }
   }
}

void Shear(Matrix3x3 &M, float x, float y) {
	int row, col;
	Matrix3x3 H(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	
	H[0][1] += x;
	H[1][0] += y;
	
	Matrix3x3 Prod = H * M;
	
	for(row = 0; row < 3; row++) {
      for(col = 0; col < 3; col++) {
         M[row][col] = Prod[row][col];
      }
   }
}

void transform(Matrix3x3 &M) {
	float minX, maxX, minY, maxY;

	//compute new corners of output image
	Vector3d topLeft(0, H, 1);
	Vector3d topRight(W, H, 1);
   Vector3d botLeft(0, 0, 1);
   Vector3d botRight(W, 0, 1);
   topLeft = M * topLeft;
   topRight = M * topRight;
   botLeft = M * botLeft;
   botRight = M * botRight;
   
   //normalize corners
   topLeft = topLeft/topLeft.z;
   topRight = topRight/topRight.z;
   botLeft = botLeft/botLeft.z;
   botRight = botRight/botRight.z;
   
   //find max and min values for x and y of corners
   minX = std::min(std::min(topLeft.x, topRight.x), std::min(botLeft.x, botRight.x));
   maxX = std::max(std::max(topLeft.x, topRight.x), std::max(botLeft.x, botRight.x));
   minY = std::min(std::min(topLeft.y, topRight.y), std::min(botLeft.y, botRight.y));
   maxY = std::max(std::max(topLeft.y, topRight.y), std::max(botLeft.y, botRight.y));
  
   //compute W2, H2
   W2 = fabs(maxX - minX);
   H2 = fabs(maxY - minY);
   
   //allocate space for transformed image
   output = new pixel*[H2];
   output[0] = new pixel[W2*H2];
   for (int i = 1; i < H2; i++)
   	output[i] = output[i-1] + W2;
   	
   //invert final transform matrix
   Matrix3x3 inverseM = M.inv();
   
   //determine if origin needs to be changed
   Vector3d origin(0,0,0);
   if (M[0][2] == 0) {
   	origin.y = minY;
   } 
   if (M[1][2] == 0) {
   	origin.x = minX;
   }
   
   //compute inverse mapping
   for(int row = 0; row < H2; row++) {
   	for(int col = 0; col < W2; col++) {
   		Vector3d outpix(col, row, 1);
   		outpix = outpix + origin;
   		Vector3d inpix = inverseM * outpix;
   		
   		float u = inpix.x/inpix.z;
   		float v = inpix.y/inpix.z;
   		
   		if (int(round(v)) < H && int(round(u)) < W && int(round(v)) >= 0 && int(round(u)) >= 0)
   			output[row][col] = mypix[static_cast<int>(round(v))][static_cast<int>(round(u))];
   	}
   }
}

/*
   Routine to build a projective transform from input text, display, or
   write transformed image to a file
*/
void process_input(Matrix3x3 &M){
   char command[1024];
   bool done;
   float theta;
   float Tx, Ty;
   float Sx, Sy;
   float Hx, Hy;


   /* build identity matrix */
   M.identity();

   for(done = false; !done;) {

      /* prompt and accept input, converting text to lower case */
      printf("> ");
      scanf("%s", command);
      lowercase(command);

      /* parse the input command, and read parameters as needed */
      if(strcmp(command, "d") == 0) {
         done = true;
      } else if(strlen(command) != 1) {
         printf("invalid command, enter r, s, t, h, d\n");
      } else {
         switch(command[0]) {

            case 'r':		/* Rotation, accept angle in degrees */
               if(scanf("%f", &theta) == 1)
                  Rotate(M, theta);
               else
                  fprintf(stderr, "invalid rotation angle\n");
               break;
            case 's':		/* Scale, accept scale factors */
            	if(scanf("%f %f", &Sx, &Sy) == 2)
            		Scale(M, Sx, Sy);
            	else
            		fprintf(stderr, "invalid scale values\n");
               break;
            case 't':		/* Translation, accept translations */
            	if(scanf("%f %f", &Tx, &Ty) == 2)
            		Translation(M, Tx, Ty);
            	else
            		fprintf(stderr, "invalid translation values\n");
               break;
            case 'h':		/* Shear, accept shear factors */
            	if(scanf("%f %f", &Hx, &Hy) == 2)
            		Shear(M, Hx, Hy);
            	else
            		fprintf(stderr, "invalid shear values\n");
               break;
            case 'd':		/* Done, that's all for now */
               done = true;
               break;
            default:
               printf("invalid command, enter r, s, t, h, d\n");
         }
      }
   }
}

void handleKey(unsigned char key, int x, int y) {
	switch(key) {					
		case 'q':
		case 'Q':
		case 27:
			exit(0);
			
		default:
			return;
	}
}

//displays image to screen
void display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawPixels(W2,H2,GL_RGBA,GL_FLOAT,output[0]);
	glFlush();
}

//initializes GL window and calls display and key event functions
//input: command line arguement values
int main(int argc, char** argv) {
	Matrix3x3 M(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	
	//checking for second file arguement in order to allow image write functionality
	if (argc == 2 || argc == 3) {
		if (argc == 3) {
			writeFile = argv[2];
			write = true;
		}
	} else {
		std::cerr << "Usage: ./warper.cpp input_image_file optional_output_file" << std::endl;
		exit(-1);
	}
	
	//readImage data with first file arguement and initialize GL display window
	readImage(argv[1]);
	
	//build transformation matrix
	process_input(M);
	
	cout << "Accumulated Matrix: " << endl;
	cout << M << endl;
	
	transform(M);
	
	if (write) {
		saveImage(writeFile);
	}
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(W2,H2);
	glutCreateWindow("Assignment 6");

	//facilitate functionality: display, keyboard presses
	glutDisplayFunc(display);
	glutKeyboardFunc(handleKey);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, W2, 0, H2);
	glClearColor(1,1,1,1);
	
	glutMainLoop();
	return 0;	
}
