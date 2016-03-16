#include <AppRFLinkAdapter.h>


static AppRFLinkAdapterConfig cfg = {
    // just to make it compile
    1, 2, 3, 4, 5, 6
};

static AppRFLinkAdapter app(cfg);


void setup()
{
    app.Run();
}

void loop() {}



