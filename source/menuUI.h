#ifndef MENU_UI
#define MENU_UI

#include <maya/MPxCommand.h>

class MenuUI : public MPxCommand
{
public:
  MenuUI();
  ~MenuUI() override;

  MStatus doIt(const MArgList& args) override;
  static void* creator();
};

#endif