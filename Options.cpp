#include "Global.h"
#include "Interface.h"

struct shFlags Flags;

struct shOption {
    const char *mName;
    int *mValue;
    int mDefault;
} Options[] = {
    {"vikeys", &Flags.mVIKeys, 0 },
    {"fadelog", &Flags.mFadeLog, 0 },
    {"show line of sight", &Flags.mShowLOS, 0 },
    {"autopickup", &Flags.mAutopickup, 0 },
    {"Autopickup Types", NULL, 0 },
    {"$ money ", &Flags.mAutopickupTypes[kMoney], 1 },
    {"+ implants", &Flags.mAutopickupTypes[kImplant], 1 },
    {"? floppy disks", &Flags.mAutopickupTypes[kFloppyDisk], 1 },
    {"! canisters", &Flags.mAutopickupTypes[kCanister], 1 },
    {"( tools", &Flags.mAutopickupTypes[kTool], 0 },
    {"] armor", &Flags.mAutopickupTypes[kArmor], 0 },
    {") weapons", &Flags.mAutopickupTypes[kWeapon], 0 },
    {"= ammunition", &Flags.mAutopickupTypes[kProjectile], 1 },
    {"& devices", &Flags.mAutopickupTypes[kDevice], 0 },
    {"/ ray guns", &Flags.mAutopickupTypes[kRayGun], 1 },
    {"* energy cells", &Flags.mAutopickupTypes[kEnergyCell], 1 },
    {NULL, NULL, 0 }
};

void
initializeOptions ()
{
    int i;
    for (i = 0; Options[i].mName; i++) {
        if (Options[i].mValue) {
            *Options[i].mValue = Options[i].mDefault;
        }
    }
}

void
shInterface::editOptions ()
{
    unsigned int i;
    shMenu menu ("Options", shMenu::kMultiPick);
    char letter = 'a';

    /* This is kludgey.  Only works for boolean options right now */

    for (i = 0; Options[i].mName; i++) {
        if (Options[i].mValue) {
            menu.addItem (letter++, Options[i].mName, (void *) i, 1, *Options[i].mValue);
        } else {
            menu.addHeader (Options[i].mName);
        }
    }

    for (i = 0; Options[i].mName; i++) {
        if (Options[i].mValue)
            *Options[i].mValue = 0;
    }
    while (menu.getResult ((const void **) &i)) {
        *Options[i].mValue = 1;
    }
}


