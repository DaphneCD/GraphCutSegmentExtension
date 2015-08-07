/*==============================================================================

  Program: 3D Slicer


  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/
#include <time.h>
// InteractiveSeg Logic includes
#include "vtkSlicerInteractiveSegLogic.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerAnnotationModuleLogic.h"


// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLLabelMapVolumeNode.h"
#include "vtkMRMLScriptedModuleNode.h"

// MRMLLogic includes
#include "vtkSlicerApplicationLogic.h"


// Markups includes
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkSlicerMarkupsLogic.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVolumeNode.h"

//Annotation includes
#include "vtkMRMLAnnotationLinesNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"

//CropVolume includes
#include "vtkMRMLCropVolumeParametersNode.h"
#include "vtkSlicerCropVolumeLogic.h"


// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkObjectFactory.h>
#include <vtkSurfaceReconstructionFilter.h>
#include <vtkContourFilter.h>
#include <vtkReverseSense.h>
#include <vtkMatrix4x4.h>
#include <vtkDelaunay3D.h>
#include <vtkCleanPolyData.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkImplicitModeller.h>
#include <vtkAlgorithmOutput.h>
#include <vtkPolygon.h>
#include <vtkRibbonFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkCallbackCommand.h>


// STD includes
#include <cassert>
#include <vector>
#include <iostream>

#include "TumorSegm/AppData.h"
#include "TumorSegm/MyBasic.h"
#include "TumorSegm/ImageProcessing.h"
#include "TumorSegm/AdaptiveSegment3D.h"



//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerInteractiveSegLogic);

void recordTime(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);
time_t start;
bool isPainter;

//----------------------------------------------------------------------------
vtkSlicerInteractiveSegLogic::vtkSlicerInteractiveSegLogic()
{
	this->gData = AppData::AppData();
	this->seg = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerInteractiveSegLogic::~vtkSlicerInteractiveSegLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerInteractiveSegLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//CHEN
int min(int x,int y)
{
    if(x<y)
        return x;
    else return y;
}

int max(int x,int y)
{
    if(x>y)
        return x;
    else return y;
}

void vtkSlicerInteractiveSegLogic::SetVolumesLogic(vtkSlicerVolumesLogic* volumes)
{
	this->volumesLogic=volumes;
}

vtkSlicerVolumesLogic* vtkSlicerInteractiveSegLogic::GetVolumesLogic()
{
	return this->volumesLogic;
}

void vtkSlicerInteractiveSegLogic::SetCropVolumeLogic(vtkSlicerCropVolumeLogic* cropVolume)
{
	this->cropVolumeLogic=cropVolume;
}

vtkSlicerCropVolumeLogic* vtkSlicerInteractiveSegLogic::GetCropVolumeLogic()
{
	return this->cropVolumeLogic;
}

void vtkSlicerInteractiveSegLogic::SetROINode(vtkMRMLAnnotationROINode* ROINode)
{
	this->ROI=ROINode;
}

vtkMRMLAnnotationROINode* vtkSlicerInteractiveSegLogic::GetROINode()
{
	return this->ROI;
}

void vtkSlicerInteractiveSegLogic::SetCropROINode(vtkMRMLAnnotationROINode* cropROINode)
{
	this->cropROI=cropROINode;
}

vtkMRMLAnnotationROINode* vtkSlicerInteractiveSegLogic::GetCropROINode()
{
	return this->cropROI;
}

int vtkSlicerInteractiveSegLogic::checkMarkups(vtkMRMLScalarVolumeNode* input,vtkMRMLMarkupsFiducialNode* markups)
{
	cout<<"Checking markups...."<<endl;
	vtkSmartPointer<vtkMatrix4x4> RASToIJKMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
	input->GetRASToIJKMatrix(RASToIJKMatrix);

    int markupNum=markups->GetNumberOfFiducials();
    vector<float> position;  //temp variance to store points' IJK 3 coordinates
	vector<float> worldposition;  //temp variance to store points' World 3 coordinates
    vector<vector<float>> points;  //vector to store sorted markup points in IJK coordinate
	vector<vector<float>> worldpoints;   //vector to store sorted markup points in world coordinate
    if(markupNum!=4)
        return 0;
    else
    {
		// Sort the markup according to their Z coordinates
        for (int i=0;i<markupNum;i++)
        {
			double pos[4];
			markups->GetNthFiducialWorldCoordinates(i,pos);
//			cout<<"WorldPOS:"<<pos[0]<<";"<<pos[1]<<";"<<pos[2]<<";"<<pos[3]<<endl;
			float temp[4];
			std::copy(pos, pos + 4, temp);
			float* ijkpos = RASToIJKMatrix->MultiplyPoint(temp);
//			cout<<"IJKPOS:"<<ijkpos[0]<<";"<<ijkpos[1]<<";"<<ijkpos[2]<<";"<<ijkpos[3]<<endl;
			
			for(int j=0;j<3;j++)
			{
				position.push_back(ijkpos[j]);
				worldposition.push_back(temp[j]);
			}
			if(i==0)
			{
				points.push_back(position);
				worldpoints.push_back(worldposition);
			}
			else
			{
				int j;
				for(j=0;j<points.size();j++)
				{
					if(points.at(j).at(2)>position.at(2))
						break;
				}
				if(j==points.size())
				{
					points.push_back(position);
					worldpoints.push_back(worldposition);
				}
				else
				{
					points.insert(points.begin()+j,position);
					worldpoints.insert(worldpoints.begin()+j,worldposition);
				}
			}
			position.clear();
			worldposition.clear();
        }

		vector<float> star_first=points.front();
		vector<float> star_last=points.back();
		vector<float> box1=points.at(1);
		vector<float> box2=points.at(2);

/*		cout<<"POINTS[0]:"<<star_first[0]<<";"<<star_first[1]<<";"<<star_first[2]<<endl;
		cout<<"POINTS[1]:"<<box1[0]<<";"<<box1[1]<<";"<<box1[2]<<endl;
		cout<<"POINTS[2]:"<<box2[0]<<";"<<box2[1]<<";"<<box2[2]<<endl;
		cout<<"POINTS[3]:"<<star_last[0]<<";"<<star_last[1]<<";"<<star_last[2]<<endl;*/
		
		if(worldpoints.at(1).at(2)!=worldpoints.at(2).at(2))  //the two points of bounding box are not sure to be on the same slice,
			return -2;                                        //due to the coorinate transformation (so it is checked before transformation)
		else 
                {
                    //Set tumor first and last center and tight box
 //                   gData.star_first=MyBasic::Index3D(int(star_first[2]),int(star_first[1]),int(star_first[0]));
 //                   gData.star_last=MyBasic::Index3D(int(star_last[2]),int(star_last[1]),int(star_last[0]));
					gData.star_first=MyBasic::Index3D(int(worldpoints.front().at(2)),int(worldpoints.front().at(1)),int(worldpoints.front().at(0)));
                    gData.star_last=MyBasic::Index3D(int(worldpoints.back().at(2)),int(worldpoints.back().at(1)),int(worldpoints.back().at(0)));
                    gData.tightBox=MyBasic::Range3D(int(star_first[2]),int(star_last[2]),
                                                   min(int(box1[1]),int(box2[1])),max(int(box1[1]),int(box2[1])),
                                                   min(int(box1[0]),int(box2[0])),max(int(box1[0]),int(box2[0])));

					vtkSmartPointer<vtkMRMLAnnotationROINode> ROI1 = vtkSmartPointer<vtkMRMLAnnotationROINode>::New();
	                this->SetROINode(vtkMRMLAnnotationROINode::SafeDownCast(ROI1));
					this->ROI->SetXYZ(double((worldpoints.at(1).at(0)+worldpoints.at(2).at(0))/2),
						              double((worldpoints.at(1).at(1)+worldpoints.at(2).at(1))/2),
									  double((worldpoints.at(0).at(2)+worldpoints.at(3).at(2))/2));
					this->ROI->SetRadiusXYZ(double(abs(worldpoints.at(1).at(0)-worldpoints.at(2).at(0))/2),
						                    double(abs(worldpoints.at(1).at(1)-worldpoints.at(2).at(1))/2),
						                    double(abs(worldpoints.at(0).at(2)-worldpoints.at(3).at(2))/2));
	                this->ROI->Initialize(this->GetMRMLScene());

                    return 1;
                }		
    }
}

int vtkSlicerInteractiveSegLogic::crop(vtkMRMLScalarVolumeNode* input,vtkMRMLCropVolumeParametersNode* parametersNode)
{
    this->cropROI = vtkSmartPointer<vtkMRMLAnnotationROINode>::New();
	double center[3];
	this->ROI->GetXYZ(center);
	cout<<"ROI center :"<<center[0]<<";"<<center[1]<<";"<<center[2]<<";"<<endl;

	vtkSmartPointer<vtkMRMLAnnotationROINode> cropROI1 = vtkSmartPointer<vtkMRMLAnnotationROINode>::New();
	this->SetCropROINode(vtkMRMLAnnotationROINode::SafeDownCast(cropROI1));
	this->cropROI->SetXYZ(center);
	double radius[3];
	this->ROI->GetRadiusXYZ(radius);
	cout<<"radius*1.2 :"<<radius[0]*1.2<<";"<<radius[1]*1.2<<";"<<radius[2]*1.2<<";"<<endl;

	double bounds[6];
	input->GetRASBounds(bounds);
	cout<<"RAS Bounds :"<<bounds[0]<<";"<<bounds[1]<<";"<<bounds[2]<<";"<<endl
		                <<bounds[3]<<";"<<bounds[4]<<";"<<bounds[5]<<";"<<endl;

	double radius1[3];
	radius1[0]=min(radius[0]*1.2,min(abs(center[0]-bounds[0]),abs(bounds[1]-center[0])));
	radius1[1]=min(radius[1]*1.2,min(abs(center[1]-bounds[2]),abs(bounds[3]-center[1])));
	radius1[2]=min(radius[2],min(abs(center[2]-bounds[4]),abs(bounds[5]-center[2])));
	cout<<"radius1 :"<<radius1[0]<<";"<<radius1[1]<<";"<<radius1[2]<<";"<<endl;

	this->cropROI->SetRadiusXYZ(radius1[0],radius1[1],radius1[2]);
	this->cropROI->Initialize(this->GetMRMLScene());

    parametersNode->SetInputVolumeNodeID(input->GetID());
    parametersNode->SetROINodeID(cropROI->GetID());
	parametersNode->SetVoxelBased(false);
	parametersNode->SetInterpolationMode(2);
	parametersNode->SetSpacingScalingConst(1.0);

//	this->GetCropVolumeLogic()->SetMRMLScene(this->GetMRMLScene());

	cout<<input->GetID()<<" ; "<<cropROI->GetID()<<" ;"<<endl;
	return this->GetCropVolumeLogic()->Apply(parametersNode);
//	return 1;
	
}

char* vtkSlicerInteractiveSegLogic::apply(vtkMRMLScalarVolumeNode* input,bool flag3D,bool flag2D,
//										vtkMRMLScalarVolumeNode* output,
										vtkMRMLScene* scene)
{
    //Set background box
    //Set pixels outside a box which is larger than the tightbox to be background
    MyBasic::Range3D bkgBox;
	MyBasic::Range3D imgBox;
	vtkImageData* orgimage=input->GetImageData();
	int* dims=orgimage->GetDimensions();
	gData.wholeRange.col=dims[0];
	gData.wholeRange.row=dims[1];
	gData.wholeRange.sli=dims[2];
	cout<<"dims :"<<dims[0]<<";"<<dims[1]<<";"<<dims[2]<<endl;

	imgBox.start.col=0;
	imgBox.start.row=0;
	imgBox.start.sli=0;

	imgBox.end.col=dims[0]-1;
	imgBox.end.row=dims[1]-1;
	imgBox.end.sli=dims[2]-1;

	//calculate shifted tight in cropped volume
	double bounds[6];
	this->ROI->GetRASBounds(bounds);
	cout<<"bounds :"<<bounds[0]<<";"<<bounds[1]<<";"<<bounds[2]<<endl
		            <<bounds[3]<<";"<<bounds[4]<<";"<<bounds[5]<<endl;
	float RASstart[4];
	float RASend[4];
	if(abs(bounds[0])<abs(bounds[1]))
	{
		RASstart[0]=bounds[0];
		RASend[0]=bounds[1];
	}
	else
	{
		RASstart[0]=bounds[1];
		RASend[0]=bounds[0];
	}

	if(abs(bounds[2])<abs(bounds[3]))
	{
		RASstart[1]=bounds[2];
		RASend[1]=bounds[3];
	}
	else
	{
		RASstart[1]=bounds[3];
		RASend[1]=bounds[2];
	}

	if(abs(bounds[4])<abs(bounds[5]))
	{
		RASstart[2]=bounds[4];
		RASend[2]=bounds[5];
	}
	else
	{
		RASstart[2]=bounds[5];
		RASend[2]=bounds[4];
	}
	RASstart[3]=1.0;
	RASend[3]=1.0;

	vtkSmartPointer<vtkMatrix4x4> RASToIJKMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
	input->GetRASToIJKMatrix(RASToIJKMatrix);
	float IJKstart[3];
	float IJKend[3];
	float* IJK=RASToIJKMatrix->MultiplyPoint(RASstart);
	IJKstart[0]=IJK[0];
	IJKstart[1]=IJK[1];
	IJKstart[2]=IJK[2];

	cout<<"RASstart :"<<RASstart[0]<<";"<<RASstart[1]<<";"<<RASstart[2]<<endl;
	cout<<"RASend :"<<RASend[0]<<";"<<RASend[1]<<";"<<RASend[2]<<endl;
	cout<<"IJKstart :"<<IJKstart[0]<<";"<<IJKstart[1]<<";"<<IJKstart[2]<<endl;
	IJK=RASToIJKMatrix->MultiplyPoint(RASend);
	IJKend[0]=IJK[0];
	IJKend[1]=IJK[1];
	IJKend[2]=IJK[2];

	//set gData's shiftedtightBox
	gData.shifttightBox.start.col=min((int)IJKstart[0],(int)IJKend[0]);
	gData.shifttightBox.start.row=min((int)IJKstart[1],(int)IJKend[1]);
	gData.shifttightBox.start.sli=min((int)IJKstart[2],(int)IJKend[2]);
	gData.shifttightBox.end.col=max((int)IJKstart[0],(int)IJKend[0]);
	gData.shifttightBox.end.row=max((int)IJKstart[1],(int)IJKend[1]);
	gData.shifttightBox.end.sli=max((int)IJKstart[2],(int)IJKend[2]);
	cout<<"IJKend :"<<IJKend[0]<<";"<<IJKend[1]<<";"<<IJKend[2]<<endl;
	cout<<"gData.shifttightBox :"<<gData.shifttightBox.start.col<<";"<<gData.shifttightBox.start.row<<";"<<gData.shifttightBox.start.sli<<endl;
	cout<<"gData.shifttightBox :"<<gData.shifttightBox.end.col<<";"<<gData.shifttightBox.end.row<<";"<<gData.shifttightBox.end.sli<<endl;

	//set gData's shifted first slice tumor center and last slice tumor center
	float starfirst[4]={gData.star_first.col,gData.star_first.row,gData.star_first.sli,1.0};
	cout<<"gData.star_first :"<<gData.star_first.col<<";"<<gData.star_first.row<<";"<<gData.star_first.sli<<endl;
	IJK=RASToIJKMatrix->MultiplyPoint(starfirst);
	gData.shiftstar_first.col=IJK[0];
	gData.shiftstar_first.row=IJK[1];
	gData.shiftstar_first.sli=IJK[2];
	cout<<"gData.shiftstar_first :"<<gData.shiftstar_first.col<<";"<<gData.shiftstar_first.row<<";"<<gData.shiftstar_first.sli<<endl;

	float starlast[4]={gData.star_last.col,gData.star_last.row,gData.star_last.sli,1.0};
	cout<<"gData.star_last :"<<gData.star_last.col<<";"<<gData.star_last.row<<";"<<gData.star_last.sli<<endl;
	IJK=RASToIJKMatrix->MultiplyPoint(starlast);
	gData.shiftstar_last.col=IJK[0];
	gData.shiftstar_last.row=IJK[1];
	gData.shiftstar_last.sli=IJK[2];
	cout<<"gData.shiftstar_last :"<<gData.shiftstar_last.col<<";"<<gData.shiftstar_last.row<<";"<<gData.shiftstar_last.sli<<endl;

	// set bkgBox
	bkgBox.start = (gData.shifttightBox.start+imgBox.start)/2;
    bkgBox.end = (gData.shifttightBox.end+imgBox.end)/2;
    bkgBox.start.sli=gData.shifttightBox.start.sli; 
	bkgBox.end.sli=gData.shifttightBox.end.sli;

	cout<<"imgBox"<<endl;
	cout<<imgBox.start.sli<<";"<<imgBox.start.row<<";"<<imgBox.start.col<<endl<<imgBox.end.sli<<";"<<imgBox.end.row<<";"<<imgBox.end.col<<endl;
	cout<<"bkgBox"<<endl;
	cout<<bkgBox.start.sli<<";"<<bkgBox.start.row<<";"<<bkgBox.start.col<<endl<<bkgBox.end.sli<<";"<<bkgBox.end.row<<";"<<bkgBox.end.col<<endl;

	 //loadImage into gData
    gData.loadImage(orgimage,imgBox);

    Data3D<bool> mat_mask(gData.image.getSize(),true);
	cout<<"aaaaaaaaaaaaaa"<<mat_mask.getNumCol()<<";"<<mat_mask.getNumRow()<<";"<<mat_mask.getNumSli()<<endl;
    mat_mask.set(bkgBox,false);
    cout<<"aaaaaaaaaaaaaa"<<endl;
    gData.seeds.set(mat_mask,BACKGROUND);
	cout<<"bbbbbbbbbbbbbbb"<<endl;

    //set pixels outside tightBox on that slice to be background
	
//    gData.seeds.set(gData.tightBox,UNKNOWN);
	gData.seeds.set(gData.shifttightBox,UNKNOWN);

	cout<<"this->seg:"<<this->seg<<endl;
    if(this->seg==NULL)
        initSeg(flag3D,flag2D);
    else reseg(flag3D,flag2D);

 /*   if(1 == showResult(input,model,scene))
	{
		return 1;
		cout<<"Finish Segmentation"<<endl;
	}
	else
		return 0;*/

	vtkSmartPointer<vtkMRMLLabelMapVolumeNode> labelVolume = vtkSmartPointer<vtkMRMLLabelMapVolumeNode>::New();
	vtkSmartPointer<vtkMRMLLabelMapVolumeNode> outputLabelVolume = vtkSmartPointer<vtkMRMLLabelMapVolumeNode>::New();
	cout<<"Created outputLabelVolume"<<endl;
	vtkSmartPointer<vtkImageData> labelMap = vtkSmartPointer<vtkImageData>::New();
	labelMap=gData.getLabelMap();
	cout<<"labelMap:"<<labelMap<<endl;

	std::string newLabel = std::string(input->GetName())+"-label";
	labelVolume=this->GetVolumesLogic()->CreateAndAddLabelVolume(input,newLabel.c_str());
	labelVolume->SetAndObserveImageData(labelMap);	
	labelVolume->UpdateScene(this->GetMRMLScene());
	
	return labelVolume->GetID();;

}

char*  vtkSlicerInteractiveSegLogic::reapply(vtkMRMLLabelMapVolumeNode* newlabels,bool flag3D,bool flag2D)
{
	calcTime();
	vtkImageData* newlabelmap=newlabels->GetImageData();
	vtkSmartPointer<vtkImageData> oldlabelmap=gData.getLabelMap();

	// Find changed labels and add them as seeds
	int dims[3];
	dims[2]=gData.image.getNumSli();
	dims[1]=gData.image.getNumRow();
	dims[0]=gData.image.getNumCol();
	for(int s=0;s<dims[2];s++)
	{
		for(int j=0;j<dims[1];j++)
		{
			for(int i=0;i<dims[0];i++)
			{
				char* oldlabel=static_cast<char*>(oldlabelmap->GetScalarPointer(i+gData.imgStart.col,j+gData.imgStart.row,s+gData.imgStart.sli));
				char* newlabel=static_cast<char*>(newlabelmap->GetScalarPointer(i+gData.imgStart.col,j+gData.imgStart.row,s+gData.imgStart.sli));
				if(oldlabel[0]!=newlabel[0])
				{
					if(newlabel[0]==1)
					    gData.seeds.at(s,j,i)=FOREGROUND;
					if(newlabel[0]==0)
					    gData.seeds.at(s,j,i)=BACKGROUND;
				}
			}
		}
	}

	reseg(flag3D,flag2D);

	// Display segment result on the 2D views
//	vtkSmartPointer<vtkImageData> labelMap = vtkSmartPointer<vtkImageData>::New();
//	labelMap=gData.getLabelMap();

	newlabels->SetAndObserveImageData(oldlabelmap);	
	newlabels->UpdateScene(this->GetMRMLScene());

	vtkSmartPointer<vtkMRMLLabelMapVolumeNode> editVolume = vtkSmartPointer<vtkMRMLLabelMapVolumeNode>::New();
	editVolume=this->GetVolumesLogic()->CreateAndAddLabelVolume(newlabels,newlabels->GetName());
	editVolume->SetAndObserveImageData(newlabelmap);	
	editVolume->UpdateScene(this->GetMRMLScene());

	vtkSmartPointer<vtkMRMLLabelMapVolumeNode> labelVolume = vtkSmartPointer<vtkMRMLLabelMapVolumeNode>::New();
	labelVolume=this->GetVolumesLogic()->CreateAndAddLabelVolume(newlabels,newlabels->GetName());
	labelVolume->SetAndObserveImageData(gData.getLabelMap());	
	labelVolume->UpdateScene(this->GetMRMLScene());

	return labelVolume->GetID();
}

vector<MyBasic::Index3D> setSideStars(MyBasic::Index3D _s1,MyBasic::Index3D star_middle, MyBasic::Index3D _s2)
{
   //initialize seeds
   vector<MyBasic::Index3D> stars(_s2.sli-_s1.sli+1);

   stars[star_middle.sli] = star_middle;

   MyBasic::Index3D star_first = _s1;
   MyBasic::Index3D star_last = _s2;

   stars.front() = star_first;
   stars.back() = star_last;

   int dr = (star_middle.row - star_first.row)/(star_middle.sli-star_first.sli);
   int dc = (star_middle.col - star_first.col)/(star_middle.sli-star_first.sli);
   for(int i=1;i<star_middle.sli;i++)
   {
       stars[i] = star_first + Index3D(i,dr*i,dc*i);
   }

   dr = (star_middle.row - star_last.row)/(star_middle.sli-star_first.sli);
   dc = (star_middle.col - star_last.col)/(star_middle.sli-star_first.sli);
   for(int i=star_last.sli-1;i>star_middle.sli;i--)
   {
       stars[i] = star_last + Index3D(i-star_last.sli,dr*(i-star_last.sli),dc*(i-star_last.sli));
   }

   cout<<"Stars:"<<endl;
   for(int i=0;i<stars.size();i++)
   {
	   cout<<"star"<<i<<":"<<endl<<stars[i].sli<<";"<<stars[i].row<<";"<<stars[i].col<<endl;
	   stars[i].print();
   }
   return stars;
}


void vtkSlicerInteractiveSegLogic::initSeg(bool flag3D,bool flag2D)
{
	cout<<"Begin inti segment"<<endl;
    Data3D<LABEL>& mask = gData.mask;
	cout<<"mask:"<<endl;
	cout<<mask.getNumSli()<<";"<<mask.getNumRow()<<";"<<mask.getNumCol()<<endl;

//	cout<<"image:"<<endl;
//	cout<<gData.image.getNumSli()<<";"<<gData.image.getNumRow()<<";"<<gData.image.getNumCol()<<endl;
    
//    MyBasic::Range3D& tightBox = gData.tightBox;
	cout<<"tightBox"<<endl;
	cout<<gData.shifttightBox.start.sli<<";"<<gData.shifttightBox.start.row<<";"<<gData.shifttightBox.start.col<<endl
		<<gData.shifttightBox.end.sli<<";"<<gData.shifttightBox.end.row<<";"<<gData.shifttightBox.end.col<<endl;

    gData.mask.setAll(BACKGROUND);
	gData.mask.set(gData.shifttightBox,FOREGROUND);
	cout<<"gData.mask.set"<<endl;

   // MyBasic::Range3D seg_region(0,16,10,46,9,43);
//	this->seg = new AdaptiveSegment3D(gData.image,gData.mask,int((gData.tightBox.start.sli+gData.tightBox.end.sli)/2));//,seg_region);
	this->seg = new AdaptiveSegment3D(gData.image,gData.mask,gData.shifttightBox.start.sli);//,seg_region);
    MyBasic::Index3D star_middle, star_first, star_last;
    star_middle = (gData.shifttightBox.start + gData.shifttightBox.end)/2;
	star_first= gData.shiftstar_first;
	star_last = gData.shiftstar_last;

	cout<<"setSideStars"<<endl;
	cout<<star_middle.sli<<";"<<star_middle.row<<";"<<star_middle.col<<endl;
	cout<<star_first.sli<<";"<<star_first.row<<";"<<star_first.col<<endl;
	cout<<star_last.sli<<";"<<star_last.row<<";"<<star_last.col<<endl;
    vector<MyBasic::Index3D> stars = setSideStars(star_first,star_middle,star_last);

    for(int i=0;i<stars.size();i++)
    {
        this->seg->addTumorCenter(stars[i]);
        this->seg->addStar2D(stars[i]);
    }

    this->seg->addStar3D(star_middle);

    //add user seeds
    Data3D<LABEL>& seeds = gData.seeds;

    this->seg->setSeeds(seeds);
    gData.seeds.save("label");

    Config cfg;
    cfg.starshape2 = flag2D;
    cfg.starshape3 = flag3D;  //2D segmentation, no 3D starshape
//	cfg.starshape2 = true;
//    cfg.starshape3 = true;
    cfg.smoothterm = true;
    cfg.dataterm = true;
    cfg.hardConstraint =true;
    cfg.white_to_dark = true;
    cfg.bin_num = 190;
	cfg.max_area = IP::bwarea<LABEL>(mask.getSlice(gData.shifttightBox.start.sli));
    cfg.min_area = cfg.max_area/3;
    cfg.seed_radius = 2;

	cout<<"Set Configure"<<endl;
    this->seg->configure(cfg);

    double decrease_area_min[7]={0.5117,0.6762,0.8207,0.8698,0.8453,0.7268,0.5069};
    vector<double> min_ratio(decrease_area_min,decrease_area_min+7);
    vector<double> max_ratio(7,1.3);
    this->seg->setAreaRatio(min_ratio,max_ratio);
	cout<<"Set Area Ratio"<<endl;

    this->seg->segment();
    gData.label = this->seg->getLabeling();
	cout<<"inti seg label"<<endl;
	cout<<"star2:"<<cfg.starshape2<<endl;
	cout<<"star3:"<<cfg.starshape3<<endl;

//	gData.label.print();
}

void vtkSlicerInteractiveSegLogic::reseg(bool flag3D,bool flag2D)
{
	this->seg->setConfig(flag2D,flag3D);

    this->seg->resegment(&gData.seeds);
    gData.label = this->seg->getLabeling();
	cout<<"reseg label"<<endl;
//	gData.label.print();
}

//int vtkSlicerInteractiveSegLogic::showResult(vtkMRMLScalarVolumeNode* input,vtkMRMLModelNode* model,vtkMRMLScene* scene)
//{
//	cout<<"Showing result..."<<endl;
//    Data3D<LABEL>& label = gData.label;
//    Data3D<short>& lableimg = gData.image;
//    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
//	vtkSmartPointer<vtkCellArray> cellArray = vtkSmartPointer<vtkCellArray>::New();
//
//	vtkSmartPointer<vtkMatrix4x4> IJKToRASMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
//	input->GetIJKToRASMatrix(IJKToRASMatrix);
//	
//	// Create polydata point set from markup points
//	int numberOfPoints=0;
//	for(int s=0;s<label.getNumSli();s++)
//    {
//        vector<MyBasic::Index2D> bds = IP::boundaryPoints(label.getSlice(s),4);	numberOfPoints+=bds.size();
//	}
//	numberOfPoints+=(label.getNumSli());
//
////	cellArray->InsertNextCell(numberOfPoints);
//	for(int s=0;s<label.getNumSli();s++)
//    {	
//        vector<MyBasic::Index2D> bds = IP::boundaryPoints(label.getSlice(s),4);	
//		vector<MyBasic::Index2D> pts = IP::boundaryPoints(label.getSlice(s),4);	
//		vector<MyBasic::Index2D> sortedpts;
//		cout << s << "  slice's  boundary:" << bds.size() << endl;
//		cellArray->InsertNextCell(bds.size());
//        // Sort boundary points clockwise
//		int next=0;//the next element in pts that wiil be appended to sortedpts
//		for(int i=0;i<bds.size();i++)
//		{				
//			int tempnext=0;
//			sortedpts.push_back(pts[next]);
//			pts.erase(pts.begin()+next);
//			double dis;
//			for(int p=0;p<pts.size();p++)
//			{		
//				// initialize dis
//				if(p==0)
//				{
//					dis=(pts[p].row-sortedpts[sortedpts.size()-1].row)*(pts[p].row-sortedpts[sortedpts.size()-1].row)
//					   +(pts[p].col-sortedpts[sortedpts.size()-1].col)*(pts[p].col-sortedpts[sortedpts.size()-1].col);
//				}
//				else
//				{
//					double newdis=(pts[p].row-sortedpts[sortedpts.size()-1].row)*(pts[p].row-sortedpts[sortedpts.size()-1].row)
//						         +(pts[p].col-sortedpts[sortedpts.size()-1].col)*(pts[p].col-sortedpts[sortedpts.size()-1].col);
//					if(newdis<dis)
//					{
//						tempnext=p;
//						dis=newdis;
//					}
//				}					
//			}		
////			cout<<"pts.size():"<<pts.size()<<";next:"<<next<<"tempnext:"<<tempnext<<endl;
//			next=tempnext;
//		}
//
//		float first[3];
//		for(int i=0;i<sortedpts.size();i++)
//		{
//			float temp[4] = {sortedpts[i].col,sortedpts[i].row,s,1};
//			float* ras = IJKToRASMatrix->MultiplyPoint(temp);
//			int id=points->InsertNextPoint(ras[0],ras[1],ras[2]);
//			cellArray->InsertCellPoint(id);	
//			if(i==0)
//			{						
//				first[0]=ras[0];
//				first[1]=ras[1];
//				first[2]=ras[2];				
//			}		
////			cout<<"id:"<<id<<endl;
//		}
//		int id=points->InsertNextPoint(first[0],first[1],first[2]);
//		cellArray->InsertCellPoint(id);	
//		
//	
//    }	
//
//	vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
//	polyData->SetPoints(points);
//	polyData->SetLines(cellArray);
//
//
///*	vtkSmartPointer<vtkDelaunay3D> delaunay = vtkSmartPointer<vtkDelaunay3D>::New();
//	delaunay->SetInputData(polyData);
//	delaunay->Update();
//
//	vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
//	surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
//	surfaceFilter->Update();*/
//
//	vtkSmartPointer<vtkRibbonFilter> ribbonFilter = vtkSmartPointer<vtkRibbonFilter>::New();
//	ribbonFilter->SetInputData(polyData);
//	ribbonFilter->SetWidth(0.1);
//	ribbonFilter->SetAngle(3);
//
//	//Draw ROI
//	float box1[3],box4[3];
//	box1[0]=gData.tightBox.start.col;
//	box1[1]=gData.tightBox.start.row;
//	box1[2]=gData.tightBox.start.sli;
//	
//	box4[0]=gData.tightBox.end.col;
//	box4[1]=gData.tightBox.end.row;
//	box4[2]=gData.tightBox.end.sli;
//	
//	float center[4]={(box1[0]+box4[0])/2,(box1[1]+box4[1])/2,(box1[2]+box4[2])/2,1};
//	float* centerRAS = IJKToRASMatrix->MultiplyPoint(center);
//	cout<<"center:"<<center[0]<<";"<<center[1]<<";"<<center[2]<<";"<<endl;
//	cout<<"centerRAS:"<<centerRAS[0]<<";"<<centerRAS[1]<<";"<<centerRAS[2]<<";"<<endl;
//
//	vtkSmartPointer<vtkMRMLAnnotationROINode> ROI = vtkSmartPointer<vtkMRMLAnnotationROINode>::New();
//	ROI->SetXYZ(double(centerRAS[0]),double(centerRAS[1]),double(centerRAS[2]));
//	cout<<"centerRAS:"<<double(centerRAS[0])<<";"<<double(centerRAS[1])<<";"<<double(centerRAS[2])<<";"<<endl;
//
//	float radius[4]={(box4[0]-box1[0])/2,(box4[1]-box1[1])/2,(box4[2]-box1[2])/2,1};
//	float* radiusRAS = IJKToRASMatrix->MultiplyPoint(radius);
//	cout<<"radius:"<<radius[0]<<";"<<radius[1]<<";"<<radius[2]<<";"<<endl;
//	cout<<"radiusRAS:"<<radiusRAS[0]<<";"<<radiusRAS[1]<<";"<<radiusRAS[2]<<";"<<endl;
//	ROI->SetRadiusXYZ(double(radiusRAS[0]),double(radiusRAS[1]),double(radiusRAS[2]));
//	ROI->Initialize(this->GetMRMLScene());
//
//	
//	// Create surface from point set
///*	vtkSmartPointer<vtkSurfaceReconstructionFilter> surf = vtkSmartPointer<vtkSurfaceReconstructionFilter>::New();
//	surf->SetInputData(polyData);
//	surf->Update();
//	
//	vtkSmartPointer<vtkContourFilter> cf = vtkSmartPointer<vtkContourFilter>::New();
//    cf->SetInputConnection(surf->GetOutputPort());
////	cf->SetInputData(polyData);
//    cf->SetValue(0, 0);
//	cf->Update();
//		
//	// Sometimes the contouring algorithm can create a volume whose gradient
//    // vector and ordering of polygon (using the right hand rule) are
//    // inconsistent. vtkReverseSense cures this problem.
//    vtkSmartPointer<vtkReverseSense> reverse = vtkSmartPointer<vtkReverseSense>::New();
//    reverse->SetInputConnection(cf->GetOutputPort());
//    reverse->ReverseCellsOff();
//    reverse->ReverseNormalsOff();
//	reverse->Update();
////	vtkSmartPointer<vtkAlgorithmOutput> newSurf = transform_back( polyData, reverse->GetOutput());
//*/	
//
///*	model->SetPolyDataConnection(ribbonFilter->GetOutputPort());	
////	model->SetAndObservePolyData(newSurf);	
//	
//	if(!model->GetDisplayNode())
//	{
//		cout<<"new model->GetDisplayNode"<<endl;
//		vtkSmartPointer<vtkMRMLModelDisplayNode> modelDisplayNode = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
//		modelDisplayNode->SetScene(this->GetMRMLScene());
//		modelDisplayNode->SetVisibility(true);
//		this->GetMRMLScene()->AddNode(modelDisplayNode.GetPointer());	
//		model->SetAndObserveDisplayNodeID(modelDisplayNode->GetID());
////		model->AddAndObserveDisplayNodeID(modelDisplayNode->GetID());
//        modelDisplayNode->SetColor(0,0,1); // Blue
//        modelDisplayNode->BackfaceCullingOff();
//        modelDisplayNode->SetOpacity(0.3); // Between 0-1, 1 being opaque
//		modelDisplayNode->SetPointSize(3);  
//		modelDisplayNode->SetLineWidth(3);
//		modelDisplayNode->SetInputPolyDataConnection(model->GetPolyDataConnection()); 
//	}
//	
//	model->GetDisplayNode()->SliceIntersectionVisibilityOn();	
//	model->GetDisplayNode()->Modified();
//	model->GetDisplayNode()->UpdateScene(this->GetMRMLScene());
//	model->Modified();
//	model->UpdateScene(this->GetMRMLScene());*/
//
//	return 1;
//}


//---------------------------------------------------------------------------
void vtkSlicerInteractiveSegLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndCloseEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
  cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
}

//-----------------------------------------------------------------------------
void vtkSlicerInteractiveSegLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}


//---------------------------------------------------------------------------
void vtkSlicerInteractiveSegLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerInteractiveSegLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
	if (this->GetMRMLScene() &&
      (this->GetMRMLScene()->IsImporting() ||
       this->GetMRMLScene()->IsRestoring()))
    {
    return;
    }
	cout<<"OnMRMLSceneNodeAdded()"<<endl;
	setEditorParamNode();
}

//---------------------------------------------------------------------------
void vtkSlicerInteractiveSegLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

void vtkSlicerInteractiveSegLogic::setEditorParamNode()
{
	cout<<"setEditorParamNode()"<<endl;
	if(this->GetMRMLScene())
	{
	int size = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLScriptedModuleNode");
	cout<<this->GetMRMLScene()<<"::"<<size<<endl;
	if(size>0)
	{
		for(int i=0;i<size;i++)
		{
			vtkMRMLScriptedModuleNode *moduleNode = vtkMRMLScriptedModuleNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLScriptedModuleNode"));
			cout<<moduleNode->GetModuleName()<<endl;
			if(strcmp(moduleNode->GetModuleName(),"Editor")==0)
				//&&this->editorModuleNode==NULL)
			{
				/*		vtkSetAndObserveMRMLNodeMacro(this->editorModuleNode,moduleNode);
				this->editorModuleNode=moduleNode;
				this->OnMRMLNodeModified(this->editorModuleNode);*/
				vtkSmartPointer<vtkCallbackCommand> tmpcallback = vtkSmartPointer<vtkCallbackCommand>::New();
				tmpcallback->SetCallback(recordTime);
				this->callback=tmpcallback;
				this->editorModuleNode=moduleNode;
				this->editorModuleNode->AddObserver(vtkCommand::ModifiedEvent,callback);
				cout<<"Got Editor Module................"<<moduleNode->GetModuleName()<<endl;
			}
		}
	}
	}
	cout<<"setEditorParamNode()END"<<endl;
}

void recordTime(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
{
	cout<<"recordTime!!!!!!!!!"<<endl;
	vtkMRMLScriptedModuleNode *moduleNode = static_cast<vtkMRMLScriptedModuleNode*>(caller);
	cout<<moduleNode->GetParameter("effect")<<endl;
	if(moduleNode->GetParameter("effect").compare("PaintEffect")==0)
	{
		cout<<"Got Paintor................."<<isPainter<<endl;
		time_t currentTime;
	    time(&currentTime);
		if(!isPainter)
		{
			cout<<"Start Time::::::::::"<<currentTime<<endl;
			start=currentTime;
			isPainter = true;
		}
	}
}


void vtkSlicerInteractiveSegLogic::calcTime()
{
	time_t currentTime;
	time(&currentTime);
	cout<<"End Time::::::::::"<<currentTime<<endl;
	isPainter=false;
	this->totalTime+=difftime(currentTime,start);
	cout<<"Totoal Time::::::::"<<this->totalTime<<endl;
	this->editorModuleNode->RemoveObservers(vtkCommand::ModifiedEvent,this->callback);
}


void vtkSlicerInteractiveSegLogic::reset(vtkMRMLMarkupsFiducialNode* markups,vtkMRMLScene* scene,int flag)
{
	if(flag == 1)
	{
		scene->RemoveNode(markups);		
		scene->RemoveNode(this->ROI);
	}
	this->seg=NULL;
}
