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

// Qt includes
#include <QtPlugin>

// GraphCutInteractiveSegmenter Logic includes
#include <vtkSlicerGraphCutInteractiveSegmenterLogic.h>

#include <vtkSlicerVolumesLogic.h>
#include <vtkSlicerAnnotationModuleLogic.h>
#include <vtkSlicerCropVolumeLogic.h>

// GraphCutInteractiveSegmenter includes
#include "qSlicerGraphCutInteractiveSegmenterModule.h"
#include "qSlicerGraphCutInteractiveSegmenterModuleWidget.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"

//#include "AbstractThreeDViewInstantiator.h"
//#include "GraphCutInteractiveSegmenterInstantiator.h"
//#include "vtkMRMLSliceViewDisplayableManagerFactory.h"
//#include "vtkMRMLThreeDViewDisplayableManagerFactory.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerGraphCutInteractiveSegmenterModule, qSlicerGraphCutInteractiveSegmenterModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerGraphCutInteractiveSegmenterModulePrivate
{
public:
  qSlicerGraphCutInteractiveSegmenterModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerGraphCutInteractiveSegmenterModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerGraphCutInteractiveSegmenterModulePrivate::qSlicerGraphCutInteractiveSegmenterModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerGraphCutInteractiveSegmenterModule methods

//-----------------------------------------------------------------------------
qSlicerGraphCutInteractiveSegmenterModule::qSlicerGraphCutInteractiveSegmenterModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerGraphCutInteractiveSegmenterModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerGraphCutInteractiveSegmenterModule::~qSlicerGraphCutInteractiveSegmenterModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerGraphCutInteractiveSegmenterModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerGraphCutInteractiveSegmenterModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerGraphCutInteractiveSegmenterModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerGraphCutInteractiveSegmenterModule::icon() const
{
  return QIcon(":/Icons/GraphCutInteractiveSegmenter.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerGraphCutInteractiveSegmenterModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerGraphCutInteractiveSegmenterModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerGraphCutInteractiveSegmenterModule::setup()
{
  this->Superclass::setup();

  // If the displayable manager is for 3D views:
//  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
 //    "vtkMRMLGraphCutInteractiveSegmenterDisplayableManager");

   // If the displayable manager is for 2D views:
//  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->
//	 RegisterDisplayableManager("vtkMRMLGraphCutInteractiveSegmenterDisplayableManager");

  vtkSlicerGraphCutInteractiveSegmenterLogic* moduleLogic =
     vtkSlicerGraphCutInteractiveSegmenterLogic::SafeDownCast(this->logic());
   
  qSlicerAbstractCoreModule* volumesModule =
	   qSlicerCoreApplication::application()->moduleManager()->module("Volumes");
   if (volumesModule)
     {
     vtkSlicerVolumesLogic* volumesLogic = 
       vtkSlicerVolumesLogic::SafeDownCast(volumesModule->logic());
     moduleLogic->SetVolumesLogic(volumesLogic);
     }

   qSlicerAbstractCoreModule* cropVolumeModule =
	   qSlicerCoreApplication::application()->moduleManager()->module("CropVolume");
   if (cropVolumeModule)
     {
     vtkSlicerCropVolumeLogic* cropVolumeLogic = 
       vtkSlicerCropVolumeLogic::SafeDownCast(cropVolumeModule->logic());
     moduleLogic->SetCropVolumeLogic(cropVolumeLogic);
     }

  
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerGraphCutInteractiveSegmenterModule
::createWidgetRepresentation()
{
  return new qSlicerGraphCutInteractiveSegmenterModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerGraphCutInteractiveSegmenterModule::createLogic()
{
  return vtkSlicerGraphCutInteractiveSegmenterLogic::New();
}
