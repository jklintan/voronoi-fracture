#pragma once

#include <maya/MPxCommand.h>

class VoronoiFracture : public MPxCommand
{
public:
    VoronoiFracture();
    ~VoronoiFracture() override;

    MStatus doIt(const MArgList& args) override;
    static void* creator();
};
