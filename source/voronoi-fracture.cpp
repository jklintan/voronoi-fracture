#include "voronoi-fracture.h"

#include <maya/MGlobal.h>

VoronoiFracture::VoronoiFracture() {};

VoronoiFracture::~VoronoiFracture() {};

MStatus VoronoiFracture::doIt(const MArgList& args)
{
    MGlobal::displayInfo("This function will fracture");

    return MStatus(MStatus::MStatusCode::kSuccess);
}

void* VoronoiFracture::creator()
{
    return new VoronoiFracture();
}