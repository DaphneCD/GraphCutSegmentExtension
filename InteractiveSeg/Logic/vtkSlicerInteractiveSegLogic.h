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

// .NAME vtkSlicerInteractiveSegLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerInteractiveSegLogic_h
#define __vtkSlicerInteractiveSegLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerAnnotationModuleLogic.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"

// Markups includes
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkSlicerMarkupsLogic.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLAnnotationROINode.h"

// STD includes
#include <cstdlib>

#include "vtkSlicerInteractiveSegModuleLogicExport.h"
#include "TumorSegm/AdaptiveSegment3D.h"
#include "TumorSegm/AppData.h"
#include "vtkPoints.h"


/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_INTERACTIVESEG_MODULE_LOGIC_EXPORT vtkSlicerInteractiveSegLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerInteractiveSegLogic *New();
  vtkTypeMacro(vtkSlicerInteractiveSegLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
  //CHEN
  int checkMarkups(vtkMRMLScalarVolumeNode* input,vtkMRMLMarkupsFiducialNode* markups);
  char* apply(vtkMRMLScalarVolumeNode* input,bool flag3D,bool flag2D,
//	  vtkMRMLScalarVolumeNode* output,
	  vtkMRMLScene* scene);
//  int showResult(vtkMRMLScalarVolumeNode* input,vtkMRMLModelNode* model,vtkMRMLScene* scene);
  void reset(vtkMRMLMarkupsFiducialNode* markups,vtkMRMLScene* scene,int flag);
  void reapply(vtkMRMLScalarVolumeNode* newlabels);
  
  void initSeg(bool flag3D,bool flag2D);
  void reseg(bool flag3D,bool flag2D);

  void SetVolumesLogic(vtkSlicerVolumesLogic* volumes);
  vtkSlicerVolumesLogic* GetVolumesLogic();

  void SetROINode(vtkMRMLAnnotationROINode* ROINode);
  vtkMRMLAnnotationROINode* GetROINode();
 
protected:
  vtkSlicerInteractiveSegLogic();
  virtual ~vtkSlicerInteractiveSegLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
private:

  vtkSlicerInteractiveSegLogic(const vtkSlicerInteractiveSegLogic&); // Not implemented
  void operator=(const vtkSlicerInteractiveSegLogic&); // Not implemented

  AdaptiveSegment3D* seg;
  AppData gData;
  vtkSlicerVolumesLogic* volumesLogic;
  vtkSlicerAnnotationModuleLogic* annotationLogic;
  vtkMRMLAnnotationROINode* ROI;
  
  
};

#endif
