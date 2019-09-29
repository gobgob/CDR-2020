import threading
import cv2
from cv2 import aruco
import numpy as np
import time
import math

ARUCO_DICT = aruco.Dictionary_get(aruco.DICT_4X4_250)
COMPASS_ID = 17
BUFFER_SIZE = 5  # Number of values before giving a result
DEBUG = True


class CompassWatcher():
    def __init__(self):
        # Compass values
        self.values = ""

        # Start `run` in a separate thread
        thread = threading.Thread(target=self.run, args=())
        thread.daemon = True
        thread.start()

    def run(self):
        cap = cv2.VideoCapture(0)

        while(True):
            # Capture frame-by-frame
            ret, frame = cap.read()

            # Frame to Grayscale
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

            # Read markers
            paramaters = aruco.DetectorParameters_create()
            corners, ids, rejectedImgPoints = aruco.detectMarkers(
                gray, ARUCO_DICT, parameters=paramaters)

            # Save the value if the compass is visible
            try:
                if isinstance(ids, np.ndarray):
                    compass = np.where(ids == COMPASS_ID)
                    if len(compass) > 0:
                        c = corners[compass[0][0]][0]
                        c0 = c[0]
                        c1 = c[1]
                        delta = [c1[0] - c0[0], c1[1] - c0[0]]
                        angle_in_rad = math.atan2(delta[0], delta[1])

                        self.values += "N" if angle_in_rad > 0 else "S"

                        # Cut the values to have the last results only
                        self.values = self.values[BUFFER_SIZE * -1:]
                        
                        if (DEBUG):
                            print(self.values)

                        time.sleep(.5)
            except:
                continue

        # When everything done, release the capture
        cap.release()
        cv2.destroyAllWindows()

    def get_value(self):
        if self.values == "S" * BUFFER_SIZE:
            return "South"

        if self.values == "N" * BUFFER_SIZE:
            return "North"

        return "???"
