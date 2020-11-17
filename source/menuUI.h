#pragma once

#include <maya/MPxCommand.h>

class MenuUI : public MPxCommand
{
public:
    MenuUI();
    ~MenuUI() override;

    MStatus doIt(const MArgList& args) override;
    MStatus undoIt(const MArgList& args);
    
    static MStatus remove();
    static void* creator();
};