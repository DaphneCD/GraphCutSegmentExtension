#ifndef APPDATA_H
#define APPDATA_H

#include "Data3D.h"
#include "Image3D.h"
#include "common.h"
#include "MyBasic.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include <vector>

using namespace std;

class AppData
{
public:
    AppData();
    void loadImage(vtkImageData* image);
	vtkSmartPointer<vtkImageData> getLabelMap();

    Image3D<short> image;
    Data3D<LABEL> label;
    Data3D<LABEL> seeds;
    //mask with user seeds
    Data3D<LABEL> mask;
    MyBasic::Range3D tightBox;
    MyBasic::Index3D star_first;
    MyBasic::Index3D star_last;
    Config cfg;


};

#endif // APPDATA_H
