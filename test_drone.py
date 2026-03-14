from aerohacks_drone import DroneClient, LED
import time

drone = DroneClient()

drone.emergency_stop()
drone.set_p_gain(0.1)
drone.set_i_gain(0.00001)
drone.set_d_gain(2.0)

drone.set_mode(2)
drone.manual_thrusts(10, 10, 10, 10)
drone.set_pitch(0)
drone.set_roll(0)
time.sleep(1)
drone.emergency_stop()
drone.set_led(LED.GREEN, True)
time.sleep(1)
drone.set_led(LED.GREEN, False)
time.sleep(1)
drone.set_led(LED.RED, True)
time.sleep(1)
drone.set_led(LED.RED, False)
time.sleep(1)
drone.set_led(LED.BLUE, True)
time.sleep(1)
drone.set_led(LED.BLUE, False)
