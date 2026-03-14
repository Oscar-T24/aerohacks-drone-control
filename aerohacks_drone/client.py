import socket
import time
from enum import Enum

"""
Make sure to connect to the Drone's wifi 

For more information : 

https://github.com/McGill-Aerial-Design/aerohacks-drone-control
"""


class LED(Enum):
    RED = "red"
    GREEN = "green"
    BLUE = "blue"

class DroneClient:
    def __init__(self, host="192.168.4.1", port=8080, timeout=2.0):
        self.host = host
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(timeout)
        self.sock.connect((host, port))

    def close(self):
        try:
            self.sock.close()
        except OSError:
            pass

    def msg(self, tx: str) -> str:
        self.sock.sendall((tx + "\n").encode("ascii"))
        rx = bytearray()
        while True:
            b = self.sock.recv(1)
            if not b:
                raise ConnectionError("Drone closed the connection")
            if b == b"\n":
                return rx.decode("ascii")
            rx.extend(b)

    def emergency_stop(self):
        self.msg("mode0")

    def set_mode(self, m: int):
        self.msg(f"mode{m}")

    def get_mode(self) -> str:
        return self.msg("gMode")

    def manual_thrusts(self, a: int, b: int, c: int, d: int):
        self.msg(f"manT\n{a},{b},{c},{d}")

    def increment_thrusts(self, a: int, b: int, c: int, d: int):
        self.msg(f"incT\n{a},{b},{c},{d}")  # only if firmware supports incT

    def get_pitch(self) -> float:
        return float(self.msg("angX")) / 16

    def get_roll(self) -> float:
        return float(self.msg("angY")) / 16

    def get_gyro_pitch(self) -> float:
        return float(self.msg("gyroX"))

    def get_gyro_roll(self) -> float:
        return float(self.msg("gyroY"))

    def set_pitch(self, r: float):
        self.msg(f"gx{r}")

    def set_roll(self, r: float):
        self.msg(f"gy{r}")

    def set_p_gain(self, p: float):
        self.msg(f"gainP{p}")

    def set_i_gain(self, i: float):
        self.msg(f"gainI{i}")

    def set_d_gain(self, d: float):
        self.msg(f"gainD{d}")

    def reset_integral(self):
        self.msg("irst")

    def get_i_values(self):
        x, y = self.msg("geti").split(",")
        return [float(x), float(y)]

    def set_yaw(self, y: float):
        self.msg(f"yaw{y}")


    def set_led(self, led:LED,state: bool):
        self.msg(f"led{led.value}{int(state)}")
        print("sent",f"led{led.value}{int(state)}")

if __name__ == "__main__":

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
    drone.set_led(LED.GREEN,True)
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
