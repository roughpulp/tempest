#include <iostream>
#include <string>

#include "tempest.h"

using namespace std;
using namespace rpulp::tempest;

int main()
{
	cout << "tempest-codec" << endl;
	
    EncodeVideoParams encode_params("TRUC", Size(352, 288), 4);

    
	vector<ImageClip> images;
	images.push_back(ImageClip("path1", 1));
	images.push_back(ImageClip("path2", 1));
	images.push_back(ImageClip("path3", 1));
	
	images_to_clip(images, encode_params, "clip.h264");
}
