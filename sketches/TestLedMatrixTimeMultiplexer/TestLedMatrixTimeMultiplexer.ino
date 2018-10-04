#include "Evm.h"
#include "SerialInput.h"
#include "LedMatrixTimeMultiplexer.h"


static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<20> shell;

static const uint8_t ROW_COUNT = 3;
static const uint8_t COL_COUNT = 2;
static const uint8_t ROW_PIN_LIST[ROW_COUNT] = { 19, 18, 17 };
static const uint8_t COL_PIN_LIST[COL_COUNT] = { 16, 15 };
static LedMatrixTimeMultiplexer<ROW_COUNT, COL_COUNT> mux(ROW_PIN_LIST, COL_PIN_LIST);

static TimedEventHandlerDelegate ted;



  void SetAll(uint8_t onOff)
  {
    for (uint8_t row = 0; row < ROW_COUNT; ++row)
    {
      for (uint8_t col = 0; col < COL_COUNT; ++col)
      {
        mux.SetLedState(row, col, onOff);
      }
    }
  }
  

void setup()
{
  LogStart(9600);
  Log(P("Starting"));
        
            
  // Set up interactive commands
  
  shell.RegisterCommand("set", [](char *cmdStr){
      Str str(cmdStr);
  
      if (str.TokenCount(' ') == 4)
      {
          uint8_t row   = atoi(str.TokenAtIdx(1, ' '));
          uint8_t col   = atoi(str.TokenAtIdx(2, ' '));
          uint8_t onOff = atoi(str.TokenAtIdx(3, ' '));
          
          Log("Setting: ", row, ", ", col, ", ", onOff);
          
          mux.SetLedState(row, col, onOff);
      }
  });

  
  shell.RegisterCommand("allon", [](char *){
      Log("Setting all off");
      SetAll(1);
  });

  shell.RegisterCommand("alloff", [](char *){
      Log("Setting all off");
      SetAll(0);
  });


  shell.RegisterCommand("timeron", [](char *cmdStr){
      Str str(cmdStr);

      uint32_t delayMs = 1000;
  
      if (str.TokenCount(' ') == 2)
      {
          delayMs = atol(str.TokenAtIdx(1, ' '));
      }
      
      Log("Timer on: ", delayMs, " ms");
      
      ted.SetCallback([](){
        static uint8_t idx = 0;

        // calculate current row/col
        uint8_t row = idx / COL_COUNT;
        uint8_t col = idx - (row * COL_COUNT);

        // toggle current pin
        mux.SetLedState(row, col, !mux.GetLedState(row, col));

        // move to next index
        idx = (idx + 1) % (ROW_COUNT * COL_COUNT);
      });

      ted.RegisterForTimedEventInterval(delayMs);
  });

  shell.RegisterCommand("timeroff", [](char *){
      Log("Timer off");
      ted.DeRegisterForTimedEvent();
  });

  


#if 0
  shell.RegisterCommand("p", [](char *cmdStr){
      Str str(cmdStr);
  
      if (str.TokenCount(' ') == 2)
      {
          uint8_t row   = atoi(str.TokenAtIdx(1, ' '));
          
          Log("Painting row: ", row);
          
          mux.RowPaint(row);
      }
  });

  shell.RegisterCommand("c", [](char *cmdStr){
      Str str(cmdStr);
  
      if (str.TokenCount(' ') == 2)
      {
          uint8_t row   = atoi(str.TokenAtIdx(1, ' '));
          
          Log("Clearing row: ", row);
          
          mux.RowClear(row);
      }
  });
#endif

  shell.RegisterCommand("start", [](char *){
      Log("Starting");
      mux.Start();
  });

  shell.RegisterCommand("stop", [](char *){
      Log("Stopping");
      mux.Stop();
  });


  // Thoughts on interval timing
  //
  // 60 frames/sec is a good target
  // 1000 / 60 = 16.66ms
  // So if I have 16.66ms (call it 16) to zip through all the rows, that
  // time budget for each is 16 / ROW_COUNT
  //
  // if ROW_COUNT < 16, then at least 1ms per row
  // if ROW_COUNT == 16, 1ms per row
  // if ROW_COUNT > 16, then 60 frames/sec isn't going to work well but
  //     we'll try and just shoot for 1ms per row
  shell.RegisterCommand("ms", [](char *cmdStr){
      Str str(cmdStr);
  
      if (str.TokenCount(' ') == 2)
      {
          uint32_t ms = atol(str.TokenAtIdx(1, ' '));
          
          Log("Starting at ms = ", ms);
          mux.SetRowIntervalMs(ms);
          mux.Start();
      }
  });

  shell.Start();
  shell.Exec("ms 2");
  shell.Exec("allon");
  shell.Exec("timeron");  

  Log("Running");

  evm.MainLoop();
}



void loop(){}




