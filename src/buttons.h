/*
    Buttons thread/object


*/
#ifndef BUTTONS_H
#define BUTTONS_H
#include <PinButton.h>

class Buttons
{
  public:
    PinButton *btn1;
    PinButton *btn2;
    PinButton *btn3;

    bool btn1SingleClick;
    bool btn1LongClick;
    bool btn2SingleClick;

    Buttons();
    void Reset();

    static void ButtonsTask(void * param);
  private:
    TaskHandle_t buttonTask;
};


#endif
