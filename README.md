This contains the library for controlling the drones 

Forked from Maximilian's version 

to install run `pip install "git+https://github.com/Oscar-T24/aerohacks-drone-control"`

Then later in a python script import the main class 

Verfication step : 

Create an empty sketch with 
```python3
from aerohacks_drone import DroneClient, LED
#... your code
```
and run it in your Python installation. If this runs without errors, you're all set !

## Note : 

If the first installation command failed (ex : Git not found) then try the following : 


1. Download the zipped library (download Zip button)
2. Unzip the library to a folder
3. Open a terminal Window (ex : Powershell on windows, Terminal on mac / linux) *inside the unzipped directory*
4. Run `pip install .`

## Testing 

Please have a look at `test-drone.py` which will power the thrusters for 1s then blink LEDs

Checklist : 
1. Make sure to have the battery pack connected for the thrusters to power on. 
2. Connect to the esp32's wifi on your comoputer.
3. Run your python script. 
