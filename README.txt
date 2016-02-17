Thomas Rapp

Affine Warp



Description: 

	This program is written in c++ and requires

	all compilation and packages associated with

	the language.

	

	This program utilizes OpenGL to display and 

	manipulate an image. The program also uses 

	OpenImageIO's read and write image functionalities 

	to read and write images from and to a specified file.

	Libraries are included in the program, however, one 

	must have the libraries downloaded for OpenGL, GLUT,

	and OpenImageIO.



	This program utilizes OpenGL to display 

	and manipulate an image using affine

	warping procedures.



Usage:

	A Makefile is provided. In the directory with the 

	source file, type the terminal command 'make' to 

	compile program and create executable called 'warper.exe'. 



	To execute:

	" ./warper 'input_image_file' 'optional_output_image_file' "



	The program will then ask for user input.

	The first letter in input will specify 

	the type of warp/transformation desired.

	The value(s) after the first letter of the

	arguement specify a floating point

	arguement that will determine the degree or

	distance to which the warp/transformation

	affects the original image.

	

	The following inputs are acceptable 

	(descriptions follow in parentheses):



	r theta (rotation by 'theta' degrees)

	s sx sy (scale by 'sx' units in x-coor and 'sy' units in y-coor)

	t dx dy (translate by 'dx' units in x-coor and 'dy' units in y-coor)

	h hx hy (shear by 'hx' units in x-coor and 'hy' units in y-coor)

	d done  (initiates combined computation of warp(s) and display resulting image)



	The program can handle combinations of warps.
