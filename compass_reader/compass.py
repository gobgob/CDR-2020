import threading
import cv2
from cv2 import aruco
import numpy as np
import time
import math

COMPASS_ID = 17
BUFFER_SIZE = 5  # Number of values before giving a result
WEBCAM_ID = 0
DEBUG = True

class CompassWatcher:
    def __init__(self):
        # Error
        self.error = None

        # Compass values
        self.values = ""

        # Aruco config
        self._aruco_dict = aruco.Dictionary_get(aruco.DICT_4X4_250)
        self._aruco_params = aruco.DetectorParameters_create()

        # Start `run` in a separate thread
        thread = threading.Thread(target=self.run)
        thread.daemon = True
        thread.start()
        self.should_stop = False

    def run(self):
        cap = cv2.VideoCapture(WEBCAM_ID)

        while(self.should_stop == False):
            # Capture frame-by-frame
            ret, frame = cap.read()

            if ret == False:
                if DEBUG:
                    print("No camera output!")
                self.error = "No camera output!"
                break

            # Frame to Grayscale
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

            # Read markers
            corners, ids, rejectedImgPoints = aruco.detectMarkers(
                gray, self._aruco_dict, parameters=self._aruco_params)

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
                        self.values = self.values[-BUFFER_SIZE:]

                        if DEBUG:
                            print(self.values)

                        time.sleep(.5)
            except:
                continue

        # When everything done, release the capture
        cap.release()
        cv2.destroyAllWindows()

    def stop(self):
        self.should_stop = True

    def get_value(self):
        if self.values == "S" * BUFFER_SIZE:
            return "South"

        if self.values == "N" * BUFFER_SIZE:
            return "North"

        return "???"
