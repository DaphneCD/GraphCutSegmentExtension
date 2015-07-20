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

void vtkSlicerInteractiveSegLogic::SetROINode(vtkMRMLAnnotationROINode* ROINode)
{
	this->ROI=ROINode;
}

vtkMRMLAnnotationROINode* vtkSlicerInteractiveSegLogic::GetROINode()
{
	return this->ROI;
}

int vtkSlicerInteractiveSegLogic::checkMarkups(vtkMRMLScalarVolumeNode* input,vtkMRMLMarkupsFiducialNode* markups)
{
	vtkSmartPointer<vtkMatrix4x4> RASToIJKMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
	input->GetRASToIJKMatrix(RASToIJKMatrix);

    int markupNum=markups->GetNumberOfFiducials();
    vector<float> position;
    vector<vector<float>> points;
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
				position.push_back(ijkpos[j]);
			if(i==0)
			{
				points.push_back(position);
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
					points.push_back(position);
				else
				{
					points.insert(points.begin()+j,position);
				}
			}
			position.clear();
        }

		vector<float> star_first=points.front();
		vector<float> star_last=points.back();
		vector<float> box1=points.at(1);
		vector<float> box2=points.at(2);

/*		cout<<"POINTS[0]:"<<star_first[0]<<";"<<star_first[1]<<";"<<star_first[2]<<endl;
		cout<<"POINTS[1]:"<<box1[0]<<";"<<box1[1]<<";"<<box1[2]<<endl;
		cout<<"POINTS[2]:"<<box2[0]<<";"<<box2[1]<<";"<<box2[2]<<endl;
		cout<<"POINTS[3]:"<<star_last[0]<<";"<<star_last[1]<<";"<<star_last[2]<<endl;*/
		
		if(box1.at(2)!=box2.at(2))
			return -2;
		else 
                {
                    //Set tumor first and last center and tight box
                    gData.star_first=MyBasic::Index3D(int(star_first[2]),int(star_first[1]),int(star_first[0]));
                    gData.star_last=MyBasic::Index3D(int(star_last[2]),int(star_last[1]),int(star_last[0]));
                    gData.tightBox=MyBasic::Range3D(int(star_first[2]),int(star_last[2]),
                                                   min(int(box1[1]),int(box2[1])),max(int(box1[1]),int(box2[1])),
                                                   min(int(box1[0]),int(box2[0])),max(int(box1[0]),int(box2[0])));
                    return 1;
                }

			
    }
}
char* vtkSlicerInteractiveSegLogic::apply(vtkMRMLScalarVolumeNode* input,bool flag3D,bool flag2D,
//										vtkMRMLScalarVolumeNode* output,
										vtkMRMLScene* scene)
{
    //loadImage into gData
    vtkImageData* image=input->GetImageData();

    gData.loadImage(image);
    //Set background box
    //Set pixels outside a box which is larger than the tightbox to be background
    MyBasic::Range3D bkgBox;
    MyBasic::Range3D imgBox=gData.image.getRange();
    bkgBox.start = (gData.tightBox.start+imgBox.start)/2;
    bkgBox.end = (gData.tightBox.end+imgBox.end)/2;
    bkgBox.start.sli=0; 
	bkgBox.end.sli=imgBox.end.sli;
	cout<<"imgBox"<<endl;
	imgBox.print();
	cout<<imgBox.start.sli<<";"<<imgBox.start.row<<";"<<imgBox.start.col<<endl<<imgBox.end.sli<<";"<<imgBox.end.row<<";"<<imgBox.end.col<<endl;
	cout<<"bkgBox"<<endl;
	bkgBox.print();
	cout<<bkgBox.start.sli<<";"<<bkgBox.start.row<<";"<<bkgBox.start.col<<endl<<bkgBox.end.sli<<";"<<bkgBox.end.row<<";"<<bkgBox.end.col<<endl;

    Data3D<bool> mat_mask(gData.image.getSize(),true);
    mat_mask.set(bkgBox,false);

    gData.seeds.set(mat_mask,BACKGROUND);
    //set pixels outside tightBox on that slice to be background
//    gData.seeds.setSlice((int)cur_slice,BACKGROUND);
    gData.seeds.set(gData.tightBox,UNKNOWN);

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

	vtkSmartPointer<vtkMRMLScalarVolumeNode> labelVolume = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
	vtkSmartPointer<vtkMRMLScalarVolumeNode> outputLabelVolume = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
	cout<<"Created outputLabelVolume"<<endl;
	vtkSmartPointer<vtkImageData> labelMap = vtkSmartPointer<vtkImageData>::New();
	labelMap=gData.getLabelMap();
	cout<<"labelMap:"<<labelMap<<endl;

	std::string newLabel = std::string(input->GetName())+"-label";
	labelVolume=this->GetVolumesLogic()->CreateAndAddLabelVolume(input,newLabel.c_str());
	labelVolume->SetAndObserveImageData(labelMap);	
	labelVolume->UpdateScene(this->GetMRMLScene());
	

	//Draw ROI
	float box1[3],box4[3];
	box1[0]=gData.tightBox.start.col;
	box1[1]=gData.tightBox.start.row;
	box1[2]=gData.tightBox.start.sli;
	
	box4[0]=gData.tightBox.end.col;
	box4[1]=gData.tightBox.end.row;
	box4[2]=gData.tightBox.end.sli;
	
	vtkSmartPointer<vtkMatrix4x4> IJKToRASMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
	input->GetIJKToRASMatrix(IJKToRASMatrix);
	float center[4]={(box1[0]+box4[0])/2,(box1[1]+box4[1])/2,(box1[2]+box4[2])/2,1};
	float* centerRAS = IJKToRASMatrix->MultiplyPoint(center);
	cout<<"center:"<<center[0]<<";"<<center[1]<<";"<<center[2]<<";"<<endl;
	cout<<"centerRAS:"<<centerRAS[0]<<";"<<centerRAS[1]<<";"<<centerRAS[2]<<";"<<endl;

	vtkSmartPointer<vtkMRMLAnnotationROINode> ROI1 = vtkSmartPointer<vtkMRMLAnnotationROINode>::New();
	this->SetROINode(vtkMRMLAnnotationROINode::SafeDownCast(ROI1));
	this->ROI->SetXYZ(double(centerRAS[0]),double(centerRAS[1]),double(centerRAS[2]));
	cout<<"centerRAS:"<<double(centerRAS[0])<<";"<<double(centerRAS[1])<<";"<<double(centerRAS[2])<<";"<<endl;

	float radius[4]={(box4[0]-box1[0])/2,(box4[1]-box1[1])/2,(box4[2]-box1[2])/2,1};
	float* radiusRAS = IJKToRASMatrix->MultiplyPoint(radius);
	cout<<"radius:"<<radius[0]<<";"<<radius[1]<<";"<<radius[2]<<";"<<endl;
	cout<<"radiusRAS:"<<radiusRAS[0]<<";"<<radiusRAS[1]<<";"<<radiusRAS[2]<<";"<<endl;
	this->ROI->SetRadiusXYZ(double(radiusRAS[0]),double(radiusRAS[1]),double(radiusRAS[2]));
	this->ROI->Initialize(this->GetMRMLScene());

	return labelVolume->GetID();;

}

void vtkSlicerInteractiveSegLogic::reapply(vtkMRMLScalarVolumeNode* newlabels)
{
	vtkImageData* newlabelmap=newlabels->GetImageData();
	vtkSmartPointer<vtkImageData> oldlabelmap=gData.getLabelMap();

	// Find changed labels and add them as seeds
	int* dims=oldlabelmap->GetDimensions();
	for(int s=0;s<dims[2];s++)
	{
		for(int j=0;j<dims[1];j++)
		{
			for(int i=0;i<dims[0];i++)
			{
				int* oldlabel=static_cast<int*>(oldlabelmap->GetScalarPointer(i,j,s));
				int* newlabel=static_cast<int*>(newlabelmap->GetScalarPointer(i,j,s));
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

	reseg(true,true);

	// Display segment result on the 2D views
	vtkSmartPointer<vtkImageData> labelMap = vtkSmartPointer<vtkImageData>::New();
	labelMap=gData.getLabelMap();

//	newlabelmap=labelMap;
//	newlabelmap=NULL;
	newlabels->SetAndObserveImageData(labelMap);	
	newlabels->UpdateScene(this->GetMRMLScene());
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
    mask.getSize().print();
    MyBasic::Range3D& tightBox = gData.tightBox;
	cout<<"tightBox"<<endl;
    tightBox.print();
	cout<<tightBox.start.sli<<";"<<tightBox.start.row<<";"<<tightBox.start.col<<endl<<tightBox.end.sli<<";"<<tightBox.end.row<<";"<<tightBox.end.col<<endl;

    gData.mask.setAll(BACKGROUND);
    gData.mask.set(gData.tightBox,FOREGROUND);

   // MyBasic::Range3D seg_region(0,16,10,46,9,43);
//	this->seg = new AdaptiveSegment3D(gData.image,gData.mask,int((gData.tightBox.start.sli+gData.tightBox.end.sli)/2));//,seg_region);
	this->seg = new AdaptiveSegment3D(gData.image,gData.mask,gData.tightBox.start.sli);//,seg_region);
    MyBasic::Index3D star_middle, star_first, star_last;
    star_middle = (gData.tightBox.start + gData.tightBox.end)/2;
    star_first = gData.star_first;
    star_last = gData.star_last;

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
    cfg.smoothterm = true;
    cfg.dataterm = true;
    cfg.hardConstraint =true;
    cfg.white_to_dark = true;
    cfg.bin_num = 190;
    cfg.max_area = IP::bwarea<LABEL>(mask.getSlice(tightBox.start.sli));
    cfg.min_area = cfg.max_area/3;
    cfg.seed_radius = 2;

//	cout<<"Set Configure"<<endl;
    this->seg->configure(cfg);

    double decrease_area_min[7]={0.5117,0.6762,0.8207,0.8698,0.8453,0.7268,0.5069};
    vector<double> min_ratio(decrease_area_min,decrease_area_min+7);
    vector<double> max_ratio(7,1.3);
    this->seg->setAreaRatio(min_ratio,max_ratio);
//	cout<<"Set Area Ratio"<<endl;

    this->seg->segment();
    gData.label = this->seg->getLabeling();
	cout<<"inti seg label"<<endl;
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
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
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
}

//---------------------------------------------------------------------------
void vtkSlicerInteractiveSegLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
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
