#ifndef __vtkMRMLInteractiveSegDisplayableManager_h
#define __vtkMRMLInteractiveSegDisplayableManager_h

#include "vtkSlicerInteractiveSegModuleMRMLDisplayableManagerExport.h"

// MRML DisplayableManager includes
#include <vtkMRMLAbstractSliceViewDisplayableManager.h>

#include <vtkMRMLVolumeNode.h>


#define VTKIS_VOLUME_PROPS 100

/// \ingroup Slicer_QtModules_VolumeRendering
class VTK_SLICER_INTERACTIVESEG_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLInteractiveSegDisplayableManager
  : public vtkMRMLAbstractSliceViewDisplayableManager
{
public:
	static vtkMRMLInteractiveSegDisplayableManager *New();
    vtkTypeMacro(vtkMRMLInteractiveSegDisplayableManager, vtkMRMLAbstractSliceViewDisplayableManager);
    void PrintSelf(ostream& os, vtkIndent indent);
	
protected:
	vtkMRMLInteractiveSegDisplayableManager();
    ~vtkMRMLInteractiveSegDisplayableManager();
	vtkMRMLInteractiveSegDisplayableManager(const vtkMRMLInteractiveSegDisplayableManager&);
    void operator=(const vtkMRMLInteractiveSegDisplayableManager&);

//	void SetAndObserveDisplayableNode(vtkMRMLVolumeNode *volumeNode);
	
	virtual void OnMRMLSceneEndClose();
};

#endif