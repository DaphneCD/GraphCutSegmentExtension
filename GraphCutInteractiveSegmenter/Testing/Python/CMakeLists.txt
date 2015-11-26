if(Slicer_USE_QtTesting AND Slicer_USE_PYTHONQT)
  slicerMacroBuildScriptedModule(
    NAME GraphCutSegmentSelfTest
    SCRIPTS GraphCutSegmentSelfTest.py
    )
  slicer_add_python_unittest(SCRIPT GraphCutSegmentSelfTest.py)
endif()
