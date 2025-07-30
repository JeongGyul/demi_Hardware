import sys
sys.path.append('/usr/lib/python3/dist-packages')
from picamera2 import Picamera2
from ultralytics import YOLO
import cv2
import serial
import time
import math

ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
time.sleep(2)

model = YOLO("/home/el/Desktop/models/yolov8s_best.pt")

picam2 = Picamera2()
picam2.preview_configuration.main.size = (640, 480)
picam2.preview_configuration.main.format = "RGB888"
picam2.configure("preview")
picam2.start()

MIN_AREA = 4000
object_present = False

while True:
    frame = picam2.capture_array()
    results = model(frame)

    boxes = results[0].boxes
    detected = False

    if boxes:
        frame_h, frame_w, _ = frame.shape
        center_x = frame_w / 2
        center_y = frame_h / 2

        min_distance = float('inf')
        closest_cls_id = None

        for box, cls_id in zip(boxes.xyxy, boxes.cls):
            x1, y1, x2, y2 = box
            area = (x2 - x1) * (y2 - y1)
            if area < MIN_AREA:
                continue

            box_center_x = (x1 + x2) / 2
            box_center_y = (y1 + y2) / 2
            distance = math.sqrt((box_center_x - center_x)**2 + (box_center_y - center_y)**2)

            if distance < min_distance:
                min_distance = distance
                closest_cls_id = int(cls_id)

        if closest_cls_id is not None:
            detected = True
            if not object_present:
                class_name = model.names[closest_cls_id]
                if class_name == "plastic":
                    ser.write(b'P')
                elif class_name == "can":
                    ser.write(b'C')
                elif class_name == "glass":
                    ser.write(b'G')
                object_present = True

    if not detected:
        object_present = False

    annotated_frame = results[0].plot()
    cv2.imshow("YOLOv8 Real-Time Detection", annotated_frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()
ser.close()
