/*
    Buttons thread/class


*/
#include "buttons.h"

Buttons::Buttons() {
  btn1 = new PinButton(37);
  btn2 = new PinButton(38);
  btn3 = new PinButton(39);

  btn1SingleClick = false;

  xTaskCreate(
    Buttons::ButtonsTask,
    "Buttons",
    1000,
    (void *)this,
    1,
    &buttonTask
  );
}

void Buttons::Reset() {
  btn1SingleClick = false;
}


void Buttons::ButtonsTask(void * param) {
  Buttons *btns = (Buttons*)param;

  for (;;) {
    btns->btn1->update();
    btns->btn2->update();
    btns->btn3->update();

    if (btns->btn1->isSingleClick()) {
      btns->btn1SingleClick = true;
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}
