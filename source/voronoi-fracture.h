#pragma once

#include <maya/MPxCommand.h>

struct Plane;

class VoronoiFracture : public MPxCommand
{
public:
    VoronoiFracture();
    ~VoronoiFracture() override;

    MStatus doIt(const MArgList& args) override;
    static void* creator();

private:
    MStatus clipAndCapMEL(const std::string& object_name, const Plane& clip_plane);
};
