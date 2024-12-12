ProportionalControlAccelStepper.ino is a simulation of a 4-wire stepper motor using 
a Slide Potentiometer and the [AccelStepper library](https://www.airspayce.com/mikem/arduino/AccelStepper/ProportionalControl_8pde-example.html)

The Wokwi development of the stepper is in [Github issue 191](https://github.com/wokwi/wokwi-features/issues/191) 

This particular simulation https://wokwi.com/projects/327379142347588180 , was 
derived from https://wokwi.com/projects/327324886912467538 after I saw it discussed on 
https://discord.com/channels/787627282663211009/787630013658824707/957984820791738398 

In the diagram.json file the stepper is:

    {
      "type": "wokwi-stepper-motor",
      "id": "sw1",
      "top": -139.16,
      "left": -191.26,
      "attrs": { "display": "steps", "arrow": "white", "gearRatio": "200:1023" }
    },

The documentation is at https://docs.wokwi.com/parts/wokwi-stepper-motor

The source and documentation for the Oscilloscope chip is at 
https://github.com/Dlloydev/Wokwi-Chip-Scope