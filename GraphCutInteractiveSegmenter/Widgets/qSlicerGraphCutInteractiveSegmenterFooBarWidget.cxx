/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerGraphCutInteractiveSegmenterFooBarWidget.h"
#include "ui_qSlicerGraphCutInteractiveSegmenterFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_GraphCutInteractiveSegmenter
class qSlicerGraphCutInteractiveSegmenterFooBarWidgetPrivate
  : public Ui_qSlicerGraphCutInteractiveSegmenterFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerGraphCutInteractiveSegmenterFooBarWidget);
protected:
  qSlicerGraphCutInteractiveSegmenterFooBarWidget* const q_ptr;

public:
  qSlicerGraphCutInteractiveSegmenterFooBarWidgetPrivate(
    qSlicerGraphCutInteractiveSegmenterFooBarWidget& object);
  virtual void setupUi(qSlicerGraphCutInteractiveSegmenterFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerGraphCutInteractiveSegmenterFooBarWidgetPrivate
::qSlicerGraphCutInteractiveSegmenterFooBarWidgetPrivate(
  qSlicerGraphCutInteractiveSegmenterFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerGraphCutInteractiveSegmenterFooBarWidgetPrivate
::setupUi(qSlicerGraphCutInteractiveSegmenterFooBarWidget* widget)
{
  this->Ui_qSlicerGraphCutInteractiveSegmenterFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerGraphCutInteractiveSegmenterFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerGraphCutInteractiveSegmenterFooBarWidget
::qSlicerGraphCutInteractiveSegmenterFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerGraphCutInteractiveSegmenterFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerGraphCutInteractiveSegmenterFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerGraphCutInteractiveSegmenterFooBarWidget
::~qSlicerGraphCutInteractiveSegmenterFooBarWidget()
{
}
