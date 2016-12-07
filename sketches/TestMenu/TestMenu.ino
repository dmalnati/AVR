#include "Evm.h"
#include "DisplayAndMenu.h"


class DisplayUser
{
public:
    DisplayUser()
    {
        disp_.AddMenuItem(MenuItemCommand{
            .fnRedraw = NULL,
            .description = (char *)"Command"
        });
    
        disp_.AddMenuItem(MenuItemRange{
            .fnRedraw = NULL,
            .description = (char *)"Range"
        });
    }

    void Init()
    {
        disp_.Init();
    }
    
private:
    DisplayAndMenu<5> disp_;
};

Evm::Instance<10,10,10> evm;
DisplayUser du;


void setup()
{
    du.Init();

    evm.MainLoop();
}


void loop() {}
















