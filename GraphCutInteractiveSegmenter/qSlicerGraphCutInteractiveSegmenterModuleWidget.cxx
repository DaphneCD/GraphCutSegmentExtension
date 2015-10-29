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
#include <QDebug>
#include <QMouseEvent>

// SlicerQt includes
#include "qSlicerGraphCutInteractiveSegmenterModuleWidget.h"
#include "ui_qSlicerGraphCutInteractiveSegmenterModuleWidget.h"

#include "qMRMLSceneModel.h"
#include "qMRMLUtils.h"
#include "qSlicerApplication.h"
#include "qSlicerAbstractCoreModule.h"
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLLabelMapVolumeNode.h"

// Markups includes
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkSlicerMarkupsLogic.h"

#include "vtkMRMLCropVolumeParametersNode.h"

#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"

// GraphCutInteractiveSegmenterLogic includes
#include "vtkSlicerGraphCutInteractiveSegmenterLogic.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerGraphCutInteractiveSegmenterModuleWidgetPrivate: public Ui_qSlicerGraphCutInteractiveSegmenterModuleWidget
{
	Q_DECLARE_PUBLIC(qSlicerGraphCutInteractiveSegmenterModuleWidget);
protected:
  qSlicerGraphCutInteractiveSegmenterModuleWidget* const q_ptr;

public:
  qSlicerGraphCutInteractiveSegmenterModuleWidgetPrivate(qSlicerGraphCutInteractiveSegmenterModuleWidget& object);
  ~qSlicerGraphCutInteractiveSegmenterModuleWidgetPrivate();
  vtkSlicerGraphCutInteractiveSegmenterLogic* logic() const;

};

//-----------------------------------------------------------------------------
// qSlicerGraphCutInteractiveSegmenterModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerGraphCutInteractiveSegmenterModuleWidgetPrivate::qSlicerGraphCutInteractiveSegmenterModuleWidgetPrivate(qSlicerGraphCutInteractiveSegmenterModuleWidget& object) : q_ptr(&object)
{
}
qSlicerGraphCutInteractiveSegmenterModuleWidgetPrivate::~qSlicerGraphCutInteractiveSegmenterModuleWidgetPrivate()
{
}

vtkSlicerGraphCutInteractiveSegmenterLogic* qSlicerGraphCutInteractiveSegmenterModuleWidgetPrivate::logic() const
{
    Q_Q(const qSlicerGraphCutInteractiveSegmenterModuleWidget);
    return vtkSlicerGraphCutInteractiveSegmenterLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerGraphCutInteractiveSegmenterModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerGraphCutInteractiveSegmenterModuleWidget::qSlicerGraphCutInteractiveSegmenterModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerGraphCutInteractiveSegmenterModuleWidgetPrivate(*this) )
{
}

qSlicerGraphCutInteractiveSegmenterModuleWidget::~qSlicerGraphCutInteractiveSegmenterModuleWidget()
{
}


//-----------------------------------------------------------------------------
void qSlicerGraphCutInteractiveSegmenterModuleWidget::setup()
{
  Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
  d->setupUi(this);
 
  this->Superclass::setup();

  // set up buttons connection
  connect(d->resetPushButton,  SIGNAL(clicked()),
                   this, SLOT(onResetPushButtonClicked()));
  connect(d->applyPushButton,  SIGNAL(clicked()),
                   this, SLOT(onApplyPushButtonClicked()));
  connect(d->reapplyPushButton,  SIGNAL(clicked()),
                   this, SLOT(onReapplyPushButtonClicked()));
  connect(d->cropPushButton,  SIGNAL(clicked()),
                   this, SLOT(onCropPushButtonClicked()));

  // set up input&output&markups connection
  connect(d->inputVolumeMRMLNodeComboBox,  SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this, SLOT(onInputVolumeMRMLNodeChanged()));
  connect(d->outputVolumeMRMLNodeComboBox,  SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this, SLOT(onOutputVolumeMRMLNodeChanged()));
  connect(d->outputVolumeMRMLNodeComboBox, SIGNAL(nodeAdded(vtkMRMLNode*)),
                   this, SLOT(onOutputVolumeAdded(vtkMRMLNode*)));
  connect(d->markupsMRMLNodeComboBox,  SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this, SLOT(onMarkupsMRMLNodeChanged()));
}

//-----------------------------------------------------------------------------
void qSlicerGraphCutInteractiveSegmenterModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  if(scene == NULL)
    {
    return;
    }

  // observe close event so can re-add a parameters node if necessary
  qvtkReconnect(this->mrmlScene(), vtkMRMLScene::EndCloseEvent,
                this, SLOT(onEndCloseEvent()));

}

//-----------------------------------------------------------------------------
void qSlicerGraphCutInteractiveSegmenterModuleWidget::enter()
{
  // if there are already some
  // volumes or ROIs in the scene, they can be set up for use

  this->onInputVolumeMRMLNodeChanged();
  this->onOutputVolumeMRMLNodeChanged();
  this->onMarkupsMRMLNodeChanged();
  Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
  d->outputVolumeMRMLNodeComboBox->setEnabled(false);

  this->Superclass::enter();
  this->isCropped=false;
}

void qSlicerGraphCutInteractiveSegmenterModuleWidget::updateApplyUpdateButtonState()
{
	
	Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
	if(!d->inputVolumeMRMLNodeComboBox->currentNode())
	{
		d->applyPushButton->setToolTip("Input volume is required to do the segmentation.");
		d->applyPushButton->setEnabled(false);
	}
	else if(!d->markupsMRMLNodeComboBox->currentNode())
	{
		d->applyPushButton->setToolTip("Markups are required to do the segmentation.");
		d->applyPushButton->setEnabled(false);
	}
	/*   else if(!d->outputVolumeMRMLNodeComboBox->currentNode())
	{
	d->applyPushButton->setToolTip("Output volume is required to do the segmentation.");
	d->applyPushButton->setEnabled(false);
	}*/
	else
	{
		d->applyPushButton->setToolTip("Segment with selected markups.");
		if(d->inputVolumeMRMLNodeComboBox->isEnabled()&&
			d->markupsMRMLNodeComboBox->isEnabled())
		{
			d->applyPushButton->setEnabled(true);
			cout<<"Apply Button Enabled!"<<endl;
		}
	}


}

void qSlicerGraphCutInteractiveSegmenterModuleWidget::updateReapplyUpdateButtonState()
{
	Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
	d->reapplyPushButton->setToolTip("Resegmentation with new added labels.");
    d->reapplyPushButton->setEnabled(true);
	d->star2CheckBox->setEnabled(false);
	d->star3CheckBox->setEnabled(false);
	d->inputVolumeMRMLNodeComboBox->setEnabled(false);
	d->markupsMRMLNodeComboBox->setEnabled(false);
    cout<<"Reapply Button Enabled!"<<endl;
}

void qSlicerGraphCutInteractiveSegmenterModuleWidget::updateResetUpdateButtonState()
{
	Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
	d->resetPushButton->setToolTip("Reset Segmentation.");
    d->resetPushButton->setEnabled(true);
    cout<<"Reset Button Enabled!"<<endl;
}

void qSlicerGraphCutInteractiveSegmenterModuleWidget:: onInputVolumeMRMLNodeChanged()
{
    Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
    Q_ASSERT(d->inputVolumeMRMLNodeComboBox);
    updateApplyUpdateButtonState();
	cout<<"onInputVolumeMRMLNodeChanged"<<endl;
}

void qSlicerGraphCutInteractiveSegmenterModuleWidget:: onOutputVolumeMRMLNodeChanged()
{
    Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
    Q_ASSERT(d->outputVolumeMRMLNodeComboBox);
	cout<<"onOutputVolumeMRMLNodeChanged"<<endl;
}

//-----------------------------------------------------------------------------

void qSlicerGraphCutInteractiveSegmenterModuleWidget::onInputVolumeAdded(vtkMRMLNode *mrmlNode)
{
  Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
  if (!mrmlNode)
    {
    return;
    }
  if (d->inputVolumeMRMLNodeComboBox->currentNode() != NULL)
    {
    // there's already a selected node, don't reset it
    return;
    }
//  updateApplyUpdateButtonState();
}


void qSlicerGraphCutInteractiveSegmenterModuleWidget:: onMarkupsMRMLNodeChanged()
{
    Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
    Q_ASSERT(d->markupsMRMLNodeComboBox);
    updateApplyUpdateButtonState();
}

void qSlicerGraphCutInteractiveSegmenterModuleWidget:: onCropPushButtonClicked()
{
	Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
//	vtkSmartPointer<vtkSlicerGraphCutInteractiveSegmenterLogic> logic = d->logic();
	vtkSlicerGraphCutInteractiveSegmenterLogic *logic = d->logic();

//	this->parametersNode = vtkSmartPointer<vtkMRMLCropVolumeParametersNode>::New();
	this->parametersNode = vtkMRMLCropVolumeParametersNode::New();
	this->mrmlScene()->AddNode(parametersNode);

	if(1==logic->checkMarkups(vtkMRMLScalarVolumeNode::SafeDownCast(d->inputVolumeMRMLNodeComboBox->currentNode()),
		                      vtkMRMLMarkupsFiducialNode::SafeDownCast(d->markupsMRMLNodeComboBox->currentNode())))
	{
		cout<<"logic->crop"<<endl;
		this->isCropped=!logic->crop(vtkMRMLScalarVolumeNode::SafeDownCast(d->inputVolumeMRMLNodeComboBox->currentNode()),parametersNode);
		if (this->isCropped)
		{		
			d->inputVolumeMRMLNodeComboBox->setCurrentNodeID(parametersNode->GetOutputVolumeNodeID());
			d->outputVolumeMRMLNodeComboBox->setCurrentNodeID(parametersNode->GetOutputVolumeNodeID());
//			vtkSlicerApplicationLogic *appLogic = this->module()->appLogic();
//			qSlicerAbstractCoreModule* cropVolumeModule =
//	               qSlicerCoreApplication::application()->moduleManager()->module("CropVolume");
			vtkSlicerApplicationLogic *appLogic = qSlicerCoreApplication::application()->moduleManager()->module("Editor")->appLogic();
			vtkMRMLSelectionNode *selectionNode = appLogic->GetSelectionNode();
			selectionNode->SetReferenceActiveVolumeID(parametersNode->GetOutputVolumeNodeID());
			appLogic->PropagateVolumeSelection();
			cout<<"After   CropVolumeLogic()->Apply"<<endl;
		}
	}
	parametersNode->Delete();
}
  
void qSlicerGraphCutInteractiveSegmenterModuleWidget:: onApplyPushButtonClicked()
{
    Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
//    vtkSmartPointer<vtkSlicerGraphCutInteractiveSegmenterLogic> logic = d->logic();
	vtkSlicerGraphCutInteractiveSegmenterLogic *logic = d->logic();

	char* labelNodeID;
	if(this->isCropped)
	{
		if(logic->GetROINode())
			labelNodeID= logic->apply(vtkMRMLScalarVolumeNode::SafeDownCast(this->mrmlScene()->GetNodeByID(this->parametersNode->GetOutputVolumeNodeID())),
			d->star3CheckBox->isChecked(),
			d->star2CheckBox->isChecked());
			//            vtkMRMLScalarVolumeNode::SafeDownCast(d->outputVolumeMRMLNodeComboBox->currentNode()),
			//this->mrmlScene());
	}
	else
	{
		cout<<"not cropped"<<endl;
		if(1==logic->checkMarkups(vtkMRMLScalarVolumeNode::SafeDownCast(d->inputVolumeMRMLNodeComboBox->currentNode()),
			vtkMRMLMarkupsFiducialNode::SafeDownCast(d->markupsMRMLNodeComboBox->currentNode())))
		{
			
			labelNodeID= logic->apply(vtkMRMLScalarVolumeNode::SafeDownCast(d->inputVolumeMRMLNodeComboBox->currentNode()),
				d->star3CheckBox->isChecked(),
				d->star2CheckBox->isChecked());
				//            vtkMRMLScalarVolumeNode::SafeDownCast(d->outputVolumeMRMLNodeComboBox->currentNode()),
				//this->mrmlScene());
		}
	}
	if(labelNodeID!=NULL)
	{
		updateResetUpdateButtonState();
		updateReapplyUpdateButtonState();

		d->outputVolumeMRMLNodeComboBox->setCurrentNodeID(labelNodeID);
		d->outputVolumeMRMLNodeComboBox->setEnabled(true);
		d->applyPushButton->setEnabled(false);
	}
	else
		return;
}

void qSlicerGraphCutInteractiveSegmenterModuleWidget:: onReapplyPushButtonClicked()
{
	Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
	vtkSmartPointer<vtkSlicerGraphCutInteractiveSegmenterLogic> logic = d->logic();

	int time=logic->calcTime();
	char time_str[30];
//	itoa(time,time_str,10);
	sprintf(time_str,"%d",time);
	d->timeLabel->setText(time_str);

	char* labelNodeID = logic->reapply(vtkMRMLLabelMapVolumeNode::SafeDownCast(d->outputVolumeMRMLNodeComboBox->currentNode()), 
		                d->star3CheckBox->isChecked(),
                        d->star2CheckBox->isChecked());
	if(labelNodeID!=NULL)
		d->outputVolumeMRMLNodeComboBox->setCurrentNodeID(labelNodeID);
	d->resetPushButton->setEnabled(true);
	d->applyPushButton->setEnabled(false);
}

void qSlicerGraphCutInteractiveSegmenterModuleWidget:: onResetPushButtonClicked()
{
	
	Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
	vtkSmartPointer<vtkSlicerGraphCutInteractiveSegmenterLogic> logic = d->logic();
	logic->reset(vtkMRMLMarkupsFiducialNode::SafeDownCast(d->markupsMRMLNodeComboBox->currentNode()),1);
	d->resetPushButton->setEnabled(false);
	d->applyPushButton->setEnabled(true);
	d->reapplyPushButton->setEnabled(false);
	d->outputVolumeMRMLNodeComboBox->setEnabled(false);
	d->star2CheckBox->setEnabled(true);
	d->star3CheckBox->setEnabled(true);
	d->inputVolumeMRMLNodeComboBox->setEnabled(true);
	d->markupsMRMLNodeComboBox->setEnabled(true);
	cout<<"reset button clicked!"<<endl;

}

void qSlicerGraphCutInteractiveSegmenterModuleWidget:: onEndCloseEvent()
{
	Q_D(qSlicerGraphCutInteractiveSegmenterModuleWidget);
	vtkSmartPointer<vtkSlicerGraphCutInteractiveSegmenterLogic> logic = d->logic();
	logic->reset(vtkMRMLMarkupsFiducialNode::SafeDownCast(d->markupsMRMLNodeComboBox->currentNode()),0);
	d->resetPushButton->setEnabled(false);
	d->applyPushButton->setEnabled(true);
	d->reapplyPushButton->setEnabled(false);
	d->outputVolumeMRMLNodeComboBox->setEnabled(false);
	d->star2CheckBox->setEnabled(true);
	d->star3CheckBox->setEnabled(true);
	d->inputVolumeMRMLNodeComboBox->setEnabled(true);
	d->markupsMRMLNodeComboBox->setEnabled(true);
	this->isCropped=false;
	cout<<"close scene!"<<endl;
}

  

