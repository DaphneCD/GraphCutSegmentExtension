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

// InteractiveSeg Logic includes
#include <vtkSlicerInteractiveSegLogic.h>

#include <vtkSlicerVolumesLogic.h>
#include <vtkSlicerAnnotationModuleLogic.h>
#include <vtkSlicerCropVolumeLogic.h>

// InteractiveSeg includes
#include "qSlicerInteractiveSegModule.h"
#include "qSlicerInteractiveSegModuleWidget.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"

//#include "AbstractThreeDViewInstantiator.h"
#include "InteractiveSegInstantiator.h"
#include "vtkMRMLSliceViewDisplayableManagerFactory.h"
#include "vtkMRMLThreeDViewDisplayableManagerFactory.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerInteractiveSegModule, qSlicerInteractiveSegModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerInteractiveSegModulePrivate
{
public:
  qSlicerInteractiveSegModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerInteractiveSegModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerInteractiveSegModulePrivate::qSlicerInteractiveSegModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerInteractiveSegModule methods

//-----------------------------------------------------------------------------
qSlicerInteractiveSegModule::qSlicerInteractiveSegModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerInteractiveSegModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerInteractiveSegModule::~qSlicerInteractiveSegModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerInteractiveSegModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerInteractiveSegModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerInteractiveSegModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerInteractiveSegModule::icon() const
{
  return QIcon(":/Icons/InteractiveSeg.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerInteractiveSegModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerInteractiveSegModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerInteractiveSegModule::setup()
{
  this->Superclass::setup();

  // If the displayable manager is for 3D views:
//  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
 //    "vtkMRMLInteractiveSegDisplayableManager");

   // If the displayable manager is for 2D views:
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->
	 RegisterDisplayableManager("vtkMRMLInteractiveSegDisplayableManager");

  vtkSlicerInteractiveSegLogic* moduleLogic =
     vtkSlicerInteractiveSegLogic::SafeDownCast(this->logic());
   
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
qSlicerAbstractModuleRepresentation* qSlicerInteractiveSegModule
::createWidgetRepresentation()
{
  return new qSlicerInteractiveSegModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerInteractiveSegModule::createLogic()
{
  return vtkSlicerInteractiveSegLogic::New();
}
