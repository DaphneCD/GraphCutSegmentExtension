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
#include "qSlicerInteractiveSegModuleWidget.h"
#include "ui_qSlicerInteractiveSegModuleWidget.h"

#include "qMRMLSceneModel.h"
#include "qMRMLUtils.h"
#include "qSlicerApplication.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"

// Markups includes
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkSlicerMarkupsLogic.h"

// InteractiveSegLogic includes
#include "vtkSlicerInteractiveSegLogic.h"

#include <iostream>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerInteractiveSegModuleWidgetPrivate: public Ui_qSlicerInteractiveSegModuleWidget
{
	Q_DECLARE_PUBLIC(qSlicerInteractiveSegModuleWidget);
protected:
  qSlicerInteractiveSegModuleWidget* const q_ptr;

public:
  qSlicerInteractiveSegModuleWidgetPrivate(qSlicerInteractiveSegModuleWidget& object);
  ~qSlicerInteractiveSegModuleWidgetPrivate();
  vtkSlicerInteractiveSegLogic* logic() const;

};

//-----------------------------------------------------------------------------
// qSlicerInteractiveSegModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerInteractiveSegModuleWidgetPrivate::qSlicerInteractiveSegModuleWidgetPrivate(qSlicerInteractiveSegModuleWidget& object) : q_ptr(&object)
{
}
qSlicerInteractiveSegModuleWidgetPrivate::~qSlicerInteractiveSegModuleWidgetPrivate()
{
}

vtkSlicerInteractiveSegLogic* qSlicerInteractiveSegModuleWidgetPrivate::logic() const
{
    Q_Q(const qSlicerInteractiveSegModuleWidget);
    return vtkSlicerInteractiveSegLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerInteractiveSegModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerInteractiveSegModuleWidget::qSlicerInteractiveSegModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerInteractiveSegModuleWidgetPrivate(*this) )
{
}

qSlicerInteractiveSegModuleWidget::~qSlicerInteractiveSegModuleWidget()
{
}


//-----------------------------------------------------------------------------
void qSlicerInteractiveSegModuleWidget::setup()
{
  Q_D(qSlicerInteractiveSegModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // set up buttons connection
  connect(d->resetPushButton,  SIGNAL(clicked()),
                   this, SLOT(onResetPushButtonClicked()));
  connect(d->applyPushButton,  SIGNAL(clicked()),
                   this, SLOT(onApplyPushButtonClicked()));
  connect(d->reapplyPushButton,  SIGNAL(clicked()),
                   this, SLOT(onReapplyPushButtonClicked()));

  // set up input&output&markups connection
  connect(d->inputVolumeMRMLNodeComboBox,  SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this, SLOT(onInputVolumeMRMLNodeChanged()));
  connect(d->outputVolumeMRMLNodeComboBox,  SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this, SLOT(onOutputVolumeMRMLNodeChanged()));
  connect(d->markupsMRMLNodeComboBox,  SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this, SLOT(onMarkupsMRMLNodeChanged()));
}

//-----------------------------------------------------------------------------
void qSlicerInteractiveSegModuleWidget::setMRMLScene(vtkMRMLScene* scene)
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
void qSlicerInteractiveSegModuleWidget::enter()
{
  // if there are already some
  // volumes or ROIs in the scene, they can be set up for use

  this->onInputVolumeMRMLNodeChanged();
  this->onOutputVolumeMRMLNodeChanged();
  this->onMarkupsMRMLNodeChanged();
  Q_D(qSlicerInteractiveSegModuleWidget);
  d->outputVolumeMRMLNodeComboBox->setEnabled(false);

  this->Superclass::enter();
}

void qSlicerInteractiveSegModuleWidget::updateApplyUpdateButtonState()
{
	
	Q_D(qSlicerInteractiveSegModuleWidget);
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

void qSlicerInteractiveSegModuleWidget::updateReapplyUpdateButtonState()
{
	Q_D(qSlicerInteractiveSegModuleWidget);
	d->reapplyPushButton->setToolTip("Resegmentation with new added labels.");
    d->reapplyPushButton->setEnabled(true);
	d->star2CheckBox->setEnabled(false);
	d->star3CheckBox->setEnabled(false);
	d->inputVolumeMRMLNodeComboBox->setEnabled(false);
	d->markupsMRMLNodeComboBox->setEnabled(false);
    cout<<"Reapply Button Enabled!"<<endl;
}

void qSlicerInteractiveSegModuleWidget::updateResetUpdateButtonState()
{
	Q_D(qSlicerInteractiveSegModuleWidget);
	d->resetPushButton->setToolTip("Reset Segmentation.");
    d->resetPushButton->setEnabled(true);
    cout<<"Reset Button Enabled!"<<endl;
}

void qSlicerInteractiveSegModuleWidget:: onInputVolumeMRMLNodeChanged()
{
    Q_D(qSlicerInteractiveSegModuleWidget);
    Q_ASSERT(d->inputVolumeMRMLNodeComboBox);
    updateApplyUpdateButtonState();
	cout<<"onInputVolumeMRMLNodeChanged"<<endl;
}

void qSlicerInteractiveSegModuleWidget:: onOutputVolumeMRMLNodeChanged()
{
    Q_D(qSlicerInteractiveSegModuleWidget);
    Q_ASSERT(d->outputVolumeMRMLNodeComboBox);
	cout<<"onOutputVolumeMRMLNodeChanged"<<endl;
}

//-----------------------------------------------------------------------------

void qSlicerInteractiveSegModuleWidget::onInputVolumeAdded(vtkMRMLNode *mrmlNode)
{
  Q_D(qSlicerInteractiveSegModuleWidget);
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


void qSlicerInteractiveSegModuleWidget:: onMarkupsMRMLNodeChanged()
{
    Q_D(qSlicerInteractiveSegModuleWidget);
    Q_ASSERT(d->markupsMRMLNodeComboBox);
    updateApplyUpdateButtonState();
}

  
void qSlicerInteractiveSegModuleWidget:: onApplyPushButtonClicked()
{
    Q_D(qSlicerInteractiveSegModuleWidget);
    vtkSlicerInteractiveSegLogic *logic = d->logic();

	if(1==logic->checkMarkups(vtkMRMLScalarVolumeNode::SafeDownCast(d->inputVolumeMRMLNodeComboBox->currentNode()),
		                      vtkMRMLMarkupsFiducialNode::SafeDownCast(d->markupsMRMLNodeComboBox->currentNode())))
    {
		char* labelNodeID= logic->apply(vtkMRMLScalarVolumeNode::SafeDownCast(d->inputVolumeMRMLNodeComboBox->currentNode()),
              d->star3CheckBox->isChecked(),
              d->star2CheckBox->isChecked(),
  //            vtkMRMLScalarVolumeNode::SafeDownCast(d->outputVolumeMRMLNodeComboBox->currentNode()),
			  this->mrmlScene());
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
}

void qSlicerInteractiveSegModuleWidget:: onReapplyPushButtonClicked()
{
	Q_D(qSlicerInteractiveSegModuleWidget);
	vtkSlicerInteractiveSegLogic *logic = d->logic();

	logic->reapply(vtkMRMLScalarVolumeNode::SafeDownCast(d->outputVolumeMRMLNodeComboBox->currentNode()));
	d->resetPushButton->setEnabled(true);
	d->applyPushButton->setEnabled(false);
}

void qSlicerInteractiveSegModuleWidget:: onResetPushButtonClicked()
{
	
	Q_D(qSlicerInteractiveSegModuleWidget);
	vtkSlicerInteractiveSegLogic *logic = d->logic();
	logic->reset(vtkMRMLMarkupsFiducialNode::SafeDownCast(d->markupsMRMLNodeComboBox->currentNode()),
			  this->mrmlScene(),1);
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

void qSlicerInteractiveSegModuleWidget:: onEndCloseEvent()
{
	Q_D(qSlicerInteractiveSegModuleWidget);
	vtkSlicerInteractiveSegLogic *logic = d->logic();
	logic->reset(vtkMRMLMarkupsFiducialNode::SafeDownCast(d->markupsMRMLNodeComboBox->currentNode()),
			  this->mrmlScene(),0);
	d->resetPushButton->setEnabled(false);
	d->applyPushButton->setEnabled(true);
	d->reapplyPushButton->setEnabled(false);
	d->outputVolumeMRMLNodeComboBox->setEnabled(false);
	d->star2CheckBox->setEnabled(true);
	d->star3CheckBox->setEnabled(true);
	d->inputVolumeMRMLNodeComboBox->setEnabled(true);
	d->markupsMRMLNodeComboBox->setEnabled(true);
	cout<<"close scene!"<<endl;
}

  

