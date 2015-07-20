#include "AppData.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include "smart_assert.h"
#include "ImageProcessing.h"

AppData::AppData()
{
  //  loadConfig("default_cfg.txt");
}

void AppData::loadImage(vtkImageData* vimage)
{
    int* dims=vimage->GetDimensions();
    double* sps=vimage->GetSpacing();
//    short* vpixels=static_cast<short*>(vimage->GetScalarPointer());

    image.resize(dims[2],dims[1],dims[0]);
    image.distS=sps[2];
    image.distR=sps[1];
    image.distC=sps[0];

//	short* pixels=new short[dims[2]*dims[1]*dims[0]];
	for(int k=0;k<dims[2];k++)
	{
		short* pixels=new short[dims[1]*dims[0]];
		for(int j=0;j<dims[1];j++)
		{
			for(int i=0;i<dims[0];i++)
			{
				short* vpixels=static_cast<short*>(vimage->GetScalarPointer(i,j,k));
//				int curpixel=k*(dims[1]*dims[0])+j*dims[0]+i;
				int curpixel=j*dims[0]+i;
				pixels[curpixel]=vpixels[0];
			}
		}
		Data2D<short> slice = Data2D<short>(dims[1],dims[0],pixels);
		image.setSlice(k,slice);
	}
    mask.resize(dims[2],dims[1],dims[0]);
    seeds.resize(dims[2],dims[1],dims[0]);
    seeds.setAll(UNKNOWN);

	cout<<"dims:"<<dims[2]<<";"<<dims[1]<<";"<<dims[0]<<endl;
//    smart_assert(!vimage,"ImageS:"+dims[2]+"ImageR:"+dims[1]+"ImageC:"+dims[0]);
}

vtkSmartPointer<vtkImageData> AppData::getLabelMap()
{
	cout<<"AppData::getLabelMap()"<<endl;
	vtkSmartPointer<vtkImageData> labelMap = vtkSmartPointer<vtkImageData>::New();
	int dims[3]={label.getNumCol(),label.getNumRow(),label.getNumSli()};
	cout<<"dims:"<<dims[0]<<";"<<dims[1]<<";"<<dims[2]<<endl;
	double spacing[3]={image.distC,image.distR,image.distS};
	cout<<"spacing:"<<spacing[0]<<";"<<spacing[1]<<";"<<spacing[2]<<endl;
	
	labelMap->SetDimensions(dims[0],dims[1],dims[2]);
	labelMap->AllocateScalars(VTK_INT,1);
//	labelMap->SetSpacing(spacing);

	int* getdim=labelMap->GetDimensions();
	cout<<"getdim;"<<getdim[0]<<";"<<getdim[1]<<";"<<getdim[2]<<endl;
	cout<<labelMap->GetNumberOfPoints()<<endl;

/*	for(int s=0;s<dims[2];s++)
	{
		// First set all label to 0
		for(int j=0;j<dims[1];j++)
		{
			for(int i=0;i<dims[0];i++)
			{
//				cout<<"labelMap->GetScalarPointer:"<<i<<";"<<j<<";"<<s<<endl;
				int* vpixels=static_cast<int*>(labelMap->GetScalarPointer(i,j,s));
				vpixels[0]=0;
			}
		}
//		cout<<"Slice "<<s<<" :already set label 0."<<endl;
		// Second set boundary point label to 1
		vector<MyBasic::Index2D> bds = IP::boundaryPoints(label.getSlice(s),4);
//		cout<<"Set label 1:"<<bds.size()<<endl;
		for(int i=0;i<bds.size();i++)
		{
			int* vpixels=static_cast<int*>(labelMap->GetScalarPointer(bds[i].col,bds[i].row,s));
			vpixels[0]=1;
//			cout<<bds[i].col<<";"<<bds[i].row<<";"<<s<<";"<<endl;
		}
//		cout<<"Slice "<<s<<" Finish set label 1!"<<endl;
	}*/

	for(int s=0;s<dims[2];s++)
	{
		// First set all label to 0
		for(int j=0;j<dims[1];j++)
		{
			for(int i=0;i<dims[0];i++)
			{
//				cout<<"labelMap->GetScalarPointer:"<<i<<";"<<j<<";"<<s<<endl;
				int* vpixels=static_cast<int*>(labelMap->GetScalarPointer(i,j,s));
				vpixels[0]=label.at(s,j,i);
			}
		}
	}

	return labelMap;
}


