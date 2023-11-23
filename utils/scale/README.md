# Scale (load cell with HX711 Amplifier)

## Setup

![image](https://github.com/kroach7/capstone-esp32-ble/assets/59946691/243dde70-0b5c-424d-a389-211d393faa88)

## Calibration

```
Call set_scale() with no parameter.
Call tare() with no parameter.
Place a known weight on the scale and call get_units(10).
Divide the result in step 3 to your known weight. You should get about the parameter you need to pass to set_scale().
Adjust the parameter in step 4 until you get an accurate reading.
```

## Energy Conservation

- power_down(), power_up()

## Resources

- https://randomnerdtutorials.com/arduino-load-cell-hx711/

- https://registry.platformio.org/libraries/bogde/HX711
