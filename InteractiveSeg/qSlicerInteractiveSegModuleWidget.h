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

#ifndef __qSlicerInteractiveSegModuleWidget_h
#define __qSlicerInteractiveSegModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerInteractiveSegModuleExport.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLModelNode.h"

class qSlicerInteractiveSegModuleWidgetPrivate;
class vtkMRMLNode;

//CHEN
class vtkMRMLMarkupsNode;
class vtkSlicerInteractiveSegLogic;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_INTERACTIVESEG_EXPORT qSlicerInteractiveSegModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerInteractiveSegModuleWidget(QWidget *parent=0);
  virtual ~qSlicerInteractiveSegModuleWidget();

  //CHEN
  /// Get the logic in the proper class
//  vtkSlicerInteractiveSegLogic* logic();

  /// Reset the GUI elements: clear out the table
  void clearGUI();
  void updateApplyButtonState();

public slots:
  //CHEN
  /// Update the selection node from the combo box
  void onInputVolumeMRMLNodeChanged();
  void onOutputVolumeMRMLNodeChanged();
  void onMarkupsMRMLNodeChanged();
  void onInputVolumeAdded(vtkMRMLNode*);

  void updateApplyUpdateButtonState();
  void updateReapplyUpdateButtonState();
  void updateResetUpdateButtonState();

  /// Display property button slots
  void onApplyPushButtonClicked();
  void onReapplyPushButtonClicked();
  void onResetPushButtonClicked();
  void onEndCloseEvent();

  ///3D star shape checked
//  void on3DStarShapeChecked(bool checked);
  ///2D star shape checked
//  void on2DStarShapeChecked(bool checked);


protected:
  QScopedPointer<qSlicerInteractiveSegModuleWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void setMRMLScene(vtkMRMLScene*);
  virtual void enter();

private:
  Q_DECLARE_PRIVATE(qSlicerInteractiveSegModuleWidget);
  Q_DISABLE_COPY(qSlicerInteractiveSegModuleWidget);
};

#endif
