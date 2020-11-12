#pragma once

#include <maya/MPxCommand.h>

class MenuUI : public MPxCommand
{
public:
    MenuUI();
    ~MenuUI() override;

    MStatus doIt(const MArgList& args) override;
    static void* creator();
};