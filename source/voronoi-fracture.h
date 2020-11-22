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
    MStatus internalClipAndCap(const char* object, const Plane& clip_plane);
    MStatus booleanClipAndCap(MFnMesh& object, const Plane& clip_plane, double half_extent);

    MStatus generateFragmentMeshes(const char* object, size_t num, MFnDagNode& parent);

    template<class T, MSyntax::MArgType TYPE, T DEFAULT>
    struct Flag
    {
        Flag(const char* flag, const char* s_flag) : FLAG(flag), SHORT(s_flag) { }

        operator T() const { return value; }

        void addToSyntax(MSyntax& s) const { s.addFlag(SHORT, FLAG, TYPE); }

        void setValue(const MArgDatabase& d) 
        { 
            if (d.isFlagSet(FLAG))
                d.getFlagArgument(FLAG, 0, value);
            else
                value = DEFAULT;
        }

    private:
        T value = DEFAULT;
        const char *FLAG, *SHORT;
    };

    enum class ClipType { INTERNAL, BOOLEAN };

    inline static Flag num_fragments = Flag<unsigned, MSyntax::kUnsigned, 5u>("-num_fragments", "-nf");
    inline static Flag delete_object = Flag<bool, MSyntax::kBoolean, true>("-delete_object", "-do");

    MDagModifier dag_modifier;
    static const ClipType CLIP_TYPE = ClipType::BOOLEAN;

    std::unique_ptr<MFnMesh> clipping_mesh;
};
