#pragma once

#include <vector>

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MDagModifier.h>
#include <maya/MPoint.h>

struct Plane;

class VoronoiFracture : public MPxCommand
{
public:
    VoronoiFracture();
    ~VoronoiFracture() override;

    MStatus doIt(const MArgList& args) override;
    static void* creator();
    static MSyntax syntaxCreator();

private:
    MStatus internalClipAndCap(MFnMesh& object, const Plane& clip_plane);
    MStatus booleanClipAndCap(MFnMesh& object, const Plane& clip_plane, double half_extent);

    MStatus generateFragmentMeshes(const char* object, size_t num, MDagPathArray& fragment_paths);

    // Generates seed points in world space
    std::vector<MPoint> generateSeedPoints(const MBoundingBox &BB, const MSelectionList& list);

    template<class T, MSyntax::MArgType TYPE>
    struct Flag
    {
        Flag(const char* flag, const char* s_flag, T d) 
            : FLAG(flag), SHORT(s_flag), DEFAULT(d), value(d) { }

        operator T() const { return value; }

        void operator=(const T& v) { value = v; }

        void addToSyntax(MSyntax& s) const { s.addFlag(SHORT, FLAG, TYPE); }

        void setValue(const MArgDatabase& d) 
        { 
            if (d.isFlagSet(FLAG))
                d.getFlagArgument(FLAG, 0, value);
            else
                value = DEFAULT;
        }

    private:
        T value;
        const T DEFAULT;
        const char *FLAG, *SHORT;
    };

    enum class ClipType { INTERNAL, BOOLEAN };

    static const ClipType CLIP_TYPE = ClipType::INTERNAL;

    inline static Flag num_fragments = Flag<unsigned, MSyntax::kUnsigned>("-num_fragments", "-nf", 5u);
    inline static Flag delete_object = Flag<bool, MSyntax::kBoolean>("-delete_object", "-do", true);
    inline static Flag curve_radius  = Flag<double, MSyntax::kDouble>("-curve_radius", "-cr", 0.1);
    inline static Flag disk_axis     = Flag<MString, MSyntax::kString>("-disk_axis", "-da", "");
    inline static Flag steps         = Flag<unsigned, MSyntax::kUnsigned>("-steps", "-s", 0);
    inline static Flag step_noise    = Flag<double, MSyntax::kDouble>("-step_noise", "-sn", 0.05);
    inline static Flag min_distance  = Flag<double, MSyntax::kDouble>("-min_distance", "-md", 1e-2);

    MDagModifier dag_modifier;

    std::unique_ptr<MFnMesh> clipping_mesh;
};
