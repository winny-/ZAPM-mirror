#include "Global.h"
#include "Interface.h"

struct shFlags Flags;

struct shOption {
    char *mName;
    int *mValue;
    int mDefault;
} Options[] = {
    {"vikeys", &Flags.mVIKeys, 0 },
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
    int i;
    shMenu menu ("Options", shMenu::kMultiPick);
    char buf[60];
    char letter = 'a';

    for (i = 0; Options[i].mName; i++) {
        if (Options[i].mValue) {
            snprintf (buf, 60, "%s [%s]", Options[i].mName, 
                      *Options[i].mValue ? "ON" : "OFF");
            menu.addItem (letter++, buf, (void *) i);
        } else {
            menu.addHeader (Options[i].mName);
        }
    }
    while (menu.getResult ((void **) &i)) {
        *Options[i].mValue = !*Options[i].mValue;
    }
}


