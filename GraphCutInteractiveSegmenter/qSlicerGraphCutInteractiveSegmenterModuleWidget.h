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

#ifndef __qSlicerGraphCutInteractiveSegmenterModuleWidget_h
#define __qSlicerGraphCutInteractiveSegmenterModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerGraphCutInteractiveSegmenterModuleExport.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLCropVolumeParametersNode.h"

class qSlicerGraphCutInteractiveSegmenterModuleWidgetPrivate;
class vtkMRMLNode;

//CHEN
class vtkMRMLMarkupsNode;
class vtkSlicerGraphCutInteractiveSegmenterLogic;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_GRAPHCUTINTERACTIVESEGMENTER_EXPORT qSlicerGraphCutInteractiveSegmenterModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerGraphCutInteractiveSegmenterModuleWidget(QWidget *parent=0);
  virtual ~qSlicerGraphCutInteractiveSegmenterModuleWidget();

  //CHEN
  /// Get the logic in the proper class
//  vtkSlicerGraphCutInteractiveSegmenterLogic* logic();

  /// Reset the GUI elements: clear out the table
  void clearGUI();
  void updateApplyButtonState();
  bool isCropped;
  bool isChecked;
  vtkMRMLCropVolumeParametersNode *parametersNode;

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
  void onCropPushButtonClicked();
  void onApplyPushButtonClicked();
  void onReapplyPushButtonClicked();
  void onResetPushButtonClicked();
  void onEndCloseEvent();

  ///3D star shape checked
//  void on3DStarShapeChecked(bool checked);
  ///2D star shape checked
//  void on2DStarShapeChecked(bool checked);


protected:
  QScopedPointer<qSlicerGraphCutInteractiveSegmenterModuleWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void setMRMLScene(vtkMRMLScene*);
  virtual void enter();

private:
  Q_DECLARE_PRIVATE(qSlicerGraphCutInteractiveSegmenterModuleWidget);
  Q_DISABLE_COPY(qSlicerGraphCutInteractiveSegmenterModuleWidget);
};

#endif
