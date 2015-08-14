#include "vtkMRMLInteractiveSegDisplayableManager.h"

#include "vtkObjectFactory.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLDisplayableNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLInteractiveSegDisplayableManager);

//---------------------------------------------------------------------------
vtkMRMLInteractiveSegDisplayableManager::vtkMRMLInteractiveSegDisplayableManager()
{
//	this->AddInteractorStyleObservableEvent(vtkCommand::LeftButtonPressEvent);
//	this->AddInteractorStyleObservableEvent(vtkCommand::LeftButtonReleaseEvent);
}

//---------------------------------------------------------------------------
vtkMRMLInteractiveSegDisplayableManager::~vtkMRMLInteractiveSegDisplayableManager()
{
}

//---------------------------------------------------------------------------
void vtkMRMLInteractiveSegDisplayableManager::PrintSelf(std::ostream &os, vtkIndent indent)
{
  os<<indent<<"Print logic"<<endl;
}
/*
void vtkMRMLInteractiveSegDisplayableManager::SetAndObserveDisplayableNode(vtkMRMLVolumeNode *volumeNode)
{
	if (!volumeNode)
	{
		return;
	}
	else
	{
		vtkMRMLDisplayableNode *displayableNode = volumeNode->GetDisplayNode();
		volumeNode->get
//		vtkSlicerApplicationGUI *applicationGUI = this->GetGUI()->GetApplicationGUI();
	}
}
*/

//---------------------------------------------------------------------------
void vtkMRMLInteractiveSegDisplayableManager::OnMRMLSceneEndClose()
{
}