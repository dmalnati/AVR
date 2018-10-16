#include "Evm.h"
#include "FunctionChain.h"
#include "SerialInput.h"
#include "LedMatrixTimeMultiplexer.h"


static Evm::Instance<20,20,20> evm;
static SerialAsyncConsoleEnhanced<20> shell;

#if 0
static const uint8_t ROW_COUNT = 3;
static const uint8_t COL_COUNT = 2;
static const uint8_t ROW_PIN_LIST[ROW_COUNT] = { 19, 18, 17 };
static const uint8_t COL_PIN_LIST[COL_COUNT] = { 16, 15 };
#endif

static const uint8_t ROW_COUNT = 14;
static const uint8_t COL_COUNT = 4;
//static const uint8_t ROW_PIN_LIST[ROW_COUNT] = { 12, 13, 14, 15, 16, 17, 7, 8,  23, 24, 25, 26, 32, 1 }; // QFN
//static const uint8_t COL_PIN_LIST[COL_COUNT] = { 2, 9, 10, 11 };  // QFN
static const uint8_t ROW_PIN_LIST[ROW_COUNT] = {   15, 14   , 16, 17, 18, 19, 9, 10, 23, 24, 25, 26,  4, 5 }; // normalized
static const uint8_t COL_PIN_LIST[COL_COUNT] = { 6, 11, 12, 13 };  // normalized




static LedMatrixTimeMultiplexer<ROW_COUNT, COL_COUNT> mux(ROW_PIN_LIST, COL_PIN_LIST);

static TimedEventHandlerDelegate ted;

static FunctionChainAsync<10> fnChain;



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
  shell.RegisterCommand("us", [](char *cmdStr){
      Str str(cmdStr);
  
      if (str.TokenCount(' ') == 2)
      {
          uint32_t us = atol(str.TokenAtIdx(1, ' '));
          
          Log("Starting at us = ", us);
          mux.SetRowIntervalUs(us);
          //mux.Start();
      }
  });


  shell.RegisterCommand("ramp", [](char *cmdStr){
      Str str(cmdStr);
  
      if (str.TokenCount(' ') == 2)
      {
          uint32_t ms = atol(str.TokenAtIdx(1, ' '));

          Log("Ramp at ", ms, " ms");

          
          fnChain.Reset();

          uint32_t rowIntervalUsList[] = { 50000, 40000, 30000, 20000, 10000, 5000, 2500, 1800, 500 };
          for (auto rowIntervalUs : rowIntervalUsList)
          {
              fnChain.Append([=](){
                  mux.SetRowIntervalUs(rowIntervalUs);
              }, ms);
          }

          fnChain.SetCallbackOnComplete([](){
              Log("Ramp done");
          });

          fnChain.Start();
      }
  });




  shell.Start();
  shell.Exec("us 200");
  shell.Exec("allon");
  shell.Exec("start");

  Log("Running");

  evm.MainLoop();
}



void loop(){}




