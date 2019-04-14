#include "Evm.h"
#include "Log.h"
#include "SerialInput.h"


static Evm::Instance<10,10,10> evm;

using Menu = SerialAsyncConsoleMenu<8, 3>;
Menu menu;


struct TestStruct
{
    uint32_t u32;
    uint16_t u16;
    uint8_t  u8;

    static const uint8_t STR_LEN = 6;
    char str[STR_LEN + 1] = { 0 };
    
    int32_t i32;
    int16_t i16;
    int8_t  i8;
};

static TestStruct testStruct;

static uint32_t otherU32 = 15;

void ShowTestStruct(TestStruct &ts)
{
    Log("ts.u32: ", ts.u32);
    Log("ts.u16: ", ts.u16);
    Log("ts.u8 : ", ts.u8);
    
    Log("ts.str: ", ts.str);
    
    Log("ts.i32: ", ts.i32);
    Log("ts.i16: ", ts.i16);
    Log("ts.i8 : ", ts.i8);
}




void setup()
{
    LogStart(9600);
    Log("Starting");
    
    menu.RegisterParamU32(P("u32"), &testStruct.u32);
    menu.RegisterParamU16(P("u16"), &testStruct.u16);
    menu.RegisterParamU8(P("u8"), &testStruct.u8);
    
    menu.RegisterParamSTR(P("str"), testStruct.str, TestStruct::STR_LEN);
    
    menu.RegisterParamI32(P("i32"), &testStruct.i32);
    menu.RegisterParamI16(P("i16"), &testStruct.i16);
    menu.RegisterParamI8(P("i8"), &testStruct.i8);
    
    menu.RegisterParamU32(P("otherU32"), &otherU32);
    
    menu.RegisterParamU32(P("wontFit"), &otherU32);
    
    
    menu.RegisterCommand(P("test"), [](){
        Log("Testing");
    });
    
    menu.RegisterCommand(P("eml"), [](){
        Log("ExitMainLoop -- should not get to actual MainLoop");
        evm.EndMainLoop();
    });
    
    uint8_t breakOut = 0;
    menu.RegisterCommand(P("break"), [&breakOut](){
        Log("breaking out to actual MainLoop");
        breakOut = 1;
        evm.EndMainLoop();
    });

    
    menu.RegisterCommand(P("cmdWontFit"), [](){
        Log("You won't see this");
    });
    
    auto fnOnSetCallback = [](){
        LogNL();
        Log("What the menu sees");
        menu.ShowParams();
        LogNL();
        Log("What the caller sees");
        ShowTestStruct(testStruct);
        Log("otherU32: ", otherU32);
        LogNL();
        LogNL();
    };
    
    menu.SetOnSetCallback(fnOnSetCallback);
    
    Log("Commands:");
    menu.ShowCommands();
    
    LogNL();
    
    Log("Current values:");
    fnOnSetCallback();
    
    
    while (!breakOut)
    {
        menu.Start();
        
        evm.HoldStackDangerously();
    }
    
    TimedEventHandlerDelegate ted;
    
    ted.SetCallback([](){
        Log("Made it to MainLoop");
    });
    ted.RegisterForTimedEventInterval(1000, 0);
    
    evm.MainLoop();
}

void loop() {}

















